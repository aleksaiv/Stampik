//File: cmytree.cpp//

#include <string.h>
#include <windows.h>

#include "types.hpp"
#include "clist.hpp"
#include "ctree.hpp"
#include "cmytree.hpp"
#include "cstamp.hpp"

#include "stamp.rh"

extern HINSTANCE hInst;

CMyTree::~CMyTree()
{
	closeall();
	while(!isEmpty())
	{
		remove();
	}
}
void CMyTree::create(CStamp *s){
	ListNode *w;
	char buf[128];
	s->first();
	do {
		w=s->GetWin();
		w->GetTypeString(buf);
		if(!seekNode(buf))
			insert(new CMyTreeNode(buf,NULL));
		w->GetNameString(buf);
		insertchild(new CMyTreeNode(buf,w));
		s->next();
	} while(!s->isHead());
	closeall();
}
bool CMyTree::seekNode(char*t){
	BOOL stop=FALSE;
//    closeall();
	CTreeNode *oldnode;
	savewindow();
	first();
	oldnode=getnode();
	if(!isEmpty())
		do {
			do {
				if(strcmp(((CMyTreeNode*)getnode())->gettext(),t)==0)
					return true;
				if(!open())
					if(!next())
						break;
			} while(1);
			first();
			if(getnode()==oldnode) break;
			do {
				if(!close()) {stop=TRUE; break;}
			} while(!next());
		} while(!stop);
	restorewindow();
	return false;
}
void CMyTree::ins(ListNode*n)
{
	char buf[128];
	closeall();
	n->GetGroupString(buf);
	if(!seekNode(buf)) {
		insert(new CMyTreeNode(buf,NULL,n));
		expand();
	}
	n->GetTypeString(buf);
	if(!open()) insertchild(new CMyTreeNode(buf,NULL,n));
	else
	if(!seekNode(buf)) {
		insert(new CMyTreeNode(buf,NULL,n));
	}
	expand();
	n->GetNameString(buf);
	insertchild(new CMyTreeNode(buf,n));
}
void CMyTree::del(ListNode*n)
{
	char buf[128];
	if(isEmpty()) return;
	closeall();
	n->GetGroupString(buf);
	if(!seekNode(buf)) return;
	open();
	n->GetTypeString(buf);
	if(!seekNode(buf)) return;
	open();
	do {
		if(((CMyTreeNode*)getnode())->getlnode()==n)
			remove();
	} while(next());
}
typedef struct
{
	CMyTree *tree;
	CTreeNode *list[100];
	RECT listcrd[100];
	char listlevel[100];
	int listsize,listpos;
	int yoff;
	BOOL focus;
	int wndpos;
	BOOL wndcreated;
	HWND hWnd;
} TREEDATA;
void InitTreeList(HWND hWnd,TREEDATA *td)
{
	BOOL stop=FALSE;
	CTreeNode *n;
	SIZE size;
	HDC hDC;
	LOGFONT lf;
	HFONT hFont,oldfnt;
	int p=0,x,y,level;

	if(td->tree->isEmpty()) {
		td->listsize=0; td->listpos=0;
		return;
	}

	td->tree->savewindow();
	n=td->tree->getnode();
	td->tree->closeall();
	hDC=GetDC(hWnd);
	x=5; y=-td->yoff; level=0;
	lf.lfHeight=8; lf.lfWidth=0;
	lf.lfEscapement=0; lf.lfOrientation=0;
	lf.lfWeight=0; lf.lfItalic=FALSE;
	lf.lfUnderline=FALSE; lf.lfStrikeOut=FALSE;
	lf.lfCharSet=ANSI_CHARSET;
	lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
	lf.lfQuality=DEFAULT_QUALITY;
	lf.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
	CopyMemory(lf.lfFaceName,"MS Sans Serif",14);
	hFont=CreateFontIndirect(&lf);
	oldfnt=SelectObject(hDC,hFont);
	do {
		do {
			td->list[p]=td->tree->getnode();
			if(td->list[p]==n) td->listpos=p;
			td->listcrd[p].left=x;
			td->listcrd[p].top=y;
			GetTextExtentPoint32(hDC,((CMyTreeNode*)td->list[p])->gettext(),
			                     strlen( ((CMyTreeNode*)td->list[p])->gettext()),
			                     &size);
			td->listcrd[p].right=x+32+size.cx;
			td->listcrd[p].bottom=y+18;
			td->listlevel[p]=level;
			y+=18;
			p++;
			if(td->tree->opened()) {
				if(!td->tree->open()) {
					if(!td->tree->next())
						break;
				}else {level++; x+=16;}
			}else {
				if(!td->tree->next())
					break;
			}
		} while(1);
		td->tree->first();
		do {
			if(!td->tree->close()) {stop=TRUE; break;}else {x-=16; level--;}
		} while(!td->tree->next());
	} while(!stop);
	td->tree->restorewindow();
	td->listsize=p;
	SelectObject(hDC,oldfnt);
	DeleteObject(hFont);
	ReleaseDC(hWnd,hDC);
}
typedef struct
{
	HWND parent;
	RECT rect;
	BOOL showed;
} THELPIK, *PTHELPIK;
LRESULT CALLBACK TreeHelpikWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	LOGFONT lf;
	HBRUSH oldbrush;
	HFONT hFont,oldfnt;
	RECT rect;
	POINT pt;
	char buf[256];
	char len;
	PTHELPIK pth;
	switch(uMsg) {
	case WM_CREATE:
		pth=new THELPIK;
		pth->parent=NULL;
		pth->showed=FALSE;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)pth);
		break;
	case WM_USER:
		pth=(PTHELPIK)GetWindowLong(hWnd,GWL_USERDATA);
		CopyMemory(&(pth->rect),(LPVOID)wParam,sizeof(RECT));
		pth->parent=(HWND)lParam;
		pth->showed=TRUE;
		SetCapture(hWnd);
		break;
	case WM_SHOWWINDOW:
		if((BOOL)wParam==FALSE) {
			pth=(PTHELPIK)GetWindowLong(hWnd,GWL_USERDATA);
			if(pth->showed) ReleaseCapture(),pth->showed=FALSE;
		}
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	case WM_MOUSEMOVE:
		pt.x=LOWORD(lParam); pt.y=HIWORD(lParam);
		pth=(PTHELPIK)GetWindowLong(hWnd,GWL_USERDATA);
		MapWindowPoints(hWnd,pth->parent,&pt,1);
		if(!PtInRect(&(pth->rect),pt)&&pth->showed) {
			ShowWindow(hWnd,SW_HIDE);
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN: {
		POINT p={LOWORD(lParam),HIWORD(lParam)};
		pth=(PTHELPIK)GetWindowLong(hWnd,GWL_USERDATA);
		MapWindowPoints(hWnd,pth->parent,&p,1);
		SendMessage(pth->parent,uMsg,wParam,
		            (LPARAM)( (DWORD) p.x + ( (DWORD) p.y<<16l ) )
		            );
	} break;
	case WM_PAINT:
		hDC=BeginPaint(hWnd,&ps);
		oldbrush=SelectObject(hDC,GetStockObject(WHITE_BRUSH));
		GetClientRect(hWnd,&rect);
		lf.lfHeight=8; lf.lfWidth=0;
		lf.lfEscapement=0; lf.lfOrientation=0;
		lf.lfWeight=0; lf.lfItalic=FALSE;
		lf.lfUnderline=FALSE; lf.lfStrikeOut=FALSE;
		lf.lfCharSet=ANSI_CHARSET;
		lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
		lf.lfQuality=DEFAULT_QUALITY;
		lf.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		CopyMemory(lf.lfFaceName,"MS Sans Serif",14);
		hFont=CreateFontIndirect(&lf);
		oldfnt=SelectObject(hDC,hFont);
		Rectangle(hDC,0,0,rect.right,rect.bottom);
		SetBkMode(hDC,TRANSPARENT);
		len=GetWindowTextLength(hWnd);
		GetWindowText(hWnd,buf,256);
		TextOut(hDC,2,2,buf,len);
		SelectObject(hDC,oldfnt);
		SelectObject(hDC,oldbrush);
		DeleteObject(hFont);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		pth=(PTHELPIK)GetWindowLong(hWnd,GWL_USERDATA);
		delete pth;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
LRESULT CALLBACK TreeWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HDC hDC,hDC2;
	PAINTSTRUCT ps;
	RECT rect,rect2;
	int x,y,xx;
	POINT crd[4]={{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
	LOGFONT lf;
	HFONT hFont,oldfnt;
	CMyTree *tree;
	CTreeNode *curpos;
	TREEDATA *td;
	BOOL stop;
	HBITMAP bmps,oldbmp;
	SIZE size;
	char level=0,len;
	HPEN oldpen;
	HBRUSH oldbrush;
	HMENU hMenu;
	char buf[80];
	if(uMsg==WM_CREATE) {
		tree=new CMyTree;
		td=new TREEDATA;
		td->tree=tree; td->yoff=0;
		td->listsize=0;
		td->wndcreated=FALSE;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)td);
		td->hWnd=CreateWindow("TreeHelpik","",
		                      WS_POPUP,
		                      0,0,10,10,
		                      hWnd,NULL,hInst,NULL
		                      );

//    SendMessage(td->hWnd,WM_USER,&rect,(LPARAM)hWnd);
		return 0;
	}
	td=(TREEDATA*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_SETFOCUS:
		td->focus=TRUE;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_KILLFOCUS:
		td->focus=FALSE;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case TM_INSERT:
		td->tree->ins((ListNode*)lParam);
		InitTreeList(hWnd,td);
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		GetClientRect(hWnd,&rect);
		SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect.bottom,TRUE);
		break;
	case TM_REMOVE:
		td->tree->del((ListNode*)lParam);
		InitTreeList(hWnd,td);
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		GetClientRect(hWnd,&rect);
		SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect.bottom,TRUE);
		break;
	case TM_CLEAR:
		delete td->tree;
		td->tree=new CMyTree;
		InitTreeList(hWnd,td);
		break;
	case WM_VSCROLL:
		GetClientRect(hWnd,&rect);
		switch(LOWORD(wParam)) {
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			td->yoff=HIWORD(wParam);
			break;
		case SB_PAGEDOWN:
			td->yoff=min(td->yoff+(rect.bottom>>1),td->listsize*18-rect.bottom);
			break;
		case SB_PAGEUP:
			td->yoff=min(td->yoff-(rect.bottom>>1),td->listsize*18-rect.bottom);
			break;
		case SB_LINEUP:
			td->yoff=min(td->yoff-18,td->listsize*18-rect.bottom);
			break;
		case SB_LINEDOWN:
			td->yoff=min(td->yoff+18,td->listsize*18-rect.bottom);
			break;
		}
		if(td->yoff<0) td->yoff=0;
		InitTreeList(hWnd,td);
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
		break;
	case WM_MOUSEMOVE: {
		POINT pt;
		pt.x=LOWORD(lParam);
		pt.y=HIWORD(lParam);
		hDC=GetDC(hWnd);
		GetClientRect(hWnd,&rect);
		for(int i=0; i<td->listsize; i++) {
			if(PtInRect(&(td->listcrd[i]),pt)) {
				if(i!=td->wndpos&&td->wndcreated) {
					ShowWindow(td->hWnd,SW_HIDE);
					td->wndcreated=FALSE;
					td->wndpos=-1;
				}
				if(td->listcrd[i].right>rect.right&&td->wndpos!=i) {
					GetWindowRect(hWnd,&rect);
					SetWindowText(td->hWnd,((CMyTreeNode*)td->list[i])->gettext());
					MoveWindow(td->hWnd,
					           td->listcrd[i].left+rect.left+12,
					           td->listcrd[i].top+rect.top+2,
					           td->listcrd[i].right-td->listcrd[i].left-12,
					           td->listcrd[i].bottom-td->listcrd[i].top-1,
					           FALSE);
					ShowWindow(td->hWnd,SW_SHOWNOACTIVATE);
					SendMessage(td->hWnd,WM_USER,(WPARAM)&(td->listcrd[i]),(LPARAM)hWnd);
					td->wndcreated=TRUE;
					td->wndpos=i;
				}
				break;
			}
		}
		ReleaseDC(hWnd,hDC);
	}
		                 break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDM_SHOWHIDE:
			SendMessage(GetParent(hWnd),TM_SHOWNODE,
			            MAKELONG(((CMyTreeNode*)td->list[td->listpos])->GetType(),
			                     td->listlevel[td->listpos]==0 ?
			                     ((CMyTreeNode*)td->list[td->listpos])->GetGroup() : -1),0);
			break;
		case IDM_OPEN:
			if(td->tree->opened()) td->tree->shrink();
			else td->tree->expand();
			InitTreeList(hWnd,td); RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			GetClientRect(hWnd,&rect);
			if(td->listcrd[td->listpos].bottom>rect.bottom) {
				td->yoff+=td->listcrd[td->listpos].bottom-rect.bottom;
				SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
				InitTreeList(hWnd,td);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}else if(td->listcrd[td->listpos].top<rect.top) {
				td->yoff-=rect.top-td->listcrd[td->listpos].top;
				SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
				InitTreeList(hWnd,td);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect.bottom,TRUE);
			break;
		case IDM_SELECT:
			SendMessage(GetParent(hWnd),TM_SELECTNODE,0,
			            (LPARAM)((CMyTreeNode*)td->list[td->listpos])->getlnode());
			break;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN: {
		POINT pt;
		pt.x=LOWORD(lParam);
		pt.y=HIWORD(lParam);
		if(!td->focus) SetFocus(hWnd);
		hDC=GetDC(hWnd);
		GetClientRect(hWnd,&rect);
		for(int i=0; i<td->listsize; i++) {
			if(PtInRect(&(td->listcrd[i]),pt)) {
				if(td->listpos!=i) {
					td->tree->setnode(td->list[i]);
					rect.left=min(td->listcrd[i].left,
					              td->listcrd[td->listpos].left);
					rect.right=max(td->listcrd[i].right,
					               td->listcrd[td->listpos].right)+1;
					rect.top=min(td->listcrd[td->listpos].top,
					             td->listcrd[i].top);
					rect.bottom=max(td->listcrd[td->listpos].bottom,
					                td->listcrd[i].bottom)+1;
					td->listpos=i;
				}else rect.left=-2;
				if(uMsg==WM_RBUTTONDOWN) {
					hMenu=CreatePopupMenu();
					if(((CMyTreeNode*)td->list[i])->getlnode()==NULL) {
						AppendMenu(hMenu,MF_STRING,IDM_OPEN,"&Expand/Collapse");
						AppendMenu(hMenu,MF_SEPARATOR,0,0);
						AppendMenu(hMenu,MF_STRING,IDM_SHOWHIDE,"&Show/hide objects");
						AppendMenu(hMenu,MF_STRING,IDM_INSERTOBJECT,"Add new object\tF7");
					}else{
						AppendMenu(hMenu,MF_STRING,IDM_SELECT,"&Select\tEnter");
						AppendMenu(hMenu,MF_SEPARATOR,0,0);
						AppendMenu(hMenu,MF_STRING,IDM_DUPLICATE,"&Duplicate\tShift+F7");
						AppendMenu(hMenu,MF_STRING,IDM_DELETEOBJECT,"&Remove\tF8");
					}
					ClientToScreen(hWnd,&pt);
					TrackPopupMenu(hMenu,TPM_RIGHTBUTTON,pt.x,pt.y,0,hWnd,NULL);
					DestroyMenu(hMenu);
				}else
				if((pt.x<=td->listcrd[i].left+11||uMsg==WM_LBUTTONDBLCLK)&&
				   ((CMyTreeNode*)td->list[i])->getlnode()==NULL) {
					if(td->tree->opened())
						td->tree->shrink();
					else td->tree->expand();
					InitTreeList(hWnd,td);
					rect.left=-1;
				}else if(uMsg==WM_LBUTTONDBLCLK&&td->listlevel[i]==2)
					SendMessage(GetParent(hWnd),TM_SELECTNODE,0,
					            (LPARAM)((CMyTreeNode*)td->list[i])->getlnode());
				GetClientRect(hWnd,&rect2);
				if(td->listcrd[td->listpos].bottom>rect2.bottom) {
					td->yoff+=td->listcrd[td->listpos].bottom-rect2.bottom;
					SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
					InitTreeList(hWnd,td);
					rect.left=-1;
					SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect2.bottom,TRUE);
				}else if(td->listcrd[td->listpos].top<rect2.top) {
					td->yoff-=rect2.top-td->listcrd[td->listpos].top;
					SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
					InitTreeList(hWnd,td);
					rect.left=-1;
					SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect2.bottom,TRUE);
				}
				if(rect.left!=-1&&rect.left!=-2) RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE);
				else if(rect.left==-1) RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
				break;
			}
		}
		ReleaseDC(hWnd,hDC);
	}
		                   break;
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS|DLGC_BUTTON;
	case WM_KEYUP:
		if(wParam==VK_RETURN) {
			if(((CMyTreeNode*)td->list[td->listpos])->getlnode()==NULL) {
				td->tree->setnode(td->list[td->listpos]);
				if(td->tree->opened())
					td->tree->shrink();
				else td->tree->expand();
				InitTreeList(hWnd,td);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}else if(td->listlevel[td->listpos]==2)
				SendMessage(GetParent(hWnd),TM_SELECTNODE,0,
				            (LPARAM)((CMyTreeNode*)td->list[td->listpos])->getlnode());
			GetClientRect(hWnd,&rect);
			if(td->listcrd[td->listpos].bottom>rect.bottom) {
				td->yoff+=td->listcrd[td->listpos].bottom-rect.bottom;
				SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
				InitTreeList(hWnd,td);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}else if(td->listcrd[td->listpos].top<rect.top) {
				td->yoff-=rect.top-td->listcrd[td->listpos].top;
				SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
				InitTreeList(hWnd,td);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect.bottom,TRUE);
		}
		break;
	case WM_KEYDOWN:
		if(td->wndcreated) {
			ShowWindow(td->hWnd,SW_HIDE);
			td->wndcreated=FALSE; td->wndpos=-1;
		}
		switch((int)wParam) {
		case VK_DELETE:
			if(((CMyTreeNode*)td->tree->getnode())->getlnode()!=NULL)
				SendMessage(GetParent(hWnd),TM_DELETENODE,
				            (WPARAM)MAKELONG(((CMyTreeNode*)td->tree->getnode())->GetType(),
				                             ((CMyTreeNode*)td->tree->getnode())->GetGroup()),
				            (LPARAM)(((CMyTreeNode*)td->tree->getnode())->getlnode())
				            );
			break;
		case VK_LEFT:
			if(td->tree->opened())
				td->tree->shrink();
			else
			if(td->tree->isFirst()) {
				if(!td->tree->close()) break;
			}else
				td->tree->first();
			InitTreeList(hWnd,td);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case VK_RIGHT:
			if(td->tree->opened()) {if(!td->tree->open()) break; }
			else td->tree->expand();
			InitTreeList(hWnd,td);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case VK_DOWN:
			if(td->listpos<td->listsize-1) {
				td->listpos++;
				td->tree->setnode(td->list[td->listpos]);
				rect.left=min(td->listcrd[td->listpos].left,
				              td->listcrd[td->listpos-1].left);
				rect.right=max(td->listcrd[td->listpos].right,
				               td->listcrd[td->listpos-1].right)+1;
				rect.top=td->listcrd[td->listpos-1].top;
				rect.bottom=td->listcrd[td->listpos].bottom+1;
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE);
			}
			break;
		case VK_UP:
			if(td->listpos>0) {
				td->listpos--;
				td->tree->setnode(td->list[td->listpos]);
				rect.left=min(td->listcrd[td->listpos].left,
				              td->listcrd[td->listpos+1].left);
				rect.right=max(td->listcrd[td->listpos].right,
				               td->listcrd[td->listpos+1].right)+1;
				rect.top=td->listcrd[td->listpos].top;
				rect.bottom=td->listcrd[td->listpos+1].bottom+1;
				RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE);
			}
			break;
		}
		GetClientRect(hWnd,&rect);
		if(td->listcrd[td->listpos].bottom>rect.bottom) {
			td->yoff+=td->listcrd[td->listpos].bottom-rect.bottom;
			SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
			InitTreeList(hWnd,td);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}else if(td->listcrd[td->listpos].top<rect.top) {
			td->yoff-=rect.top-td->listcrd[td->listpos].top;
			SetScrollPos(hWnd,SB_VERT,td->yoff,TRUE);
			InitTreeList(hWnd,td);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		SetScrollRange(hWnd,SB_VERT,0,td->listsize*18-rect.bottom,TRUE);
		break;
	case WM_PAINT:
		hDC=BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rect);
		FillRect(hDC,&rect,GetStockObject(WHITE_BRUSH));
		tree=td->tree;
		if(tree->isEmpty()) {
			EndPaint(hWnd,&ps);
			break;
		}
		tree->savewindow();
		curpos=tree->getnode();
		tree->closeall();
		xx=0; x=5;
		lf.lfHeight=8; lf.lfWidth=0;
		lf.lfEscapement=0; lf.lfOrientation=0;
		lf.lfWeight=0; lf.lfItalic=FALSE;
		lf.lfUnderline=FALSE; lf.lfStrikeOut=FALSE;
		lf.lfCharSet=ANSI_CHARSET;
		lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
		lf.lfQuality=DEFAULT_QUALITY;
		lf.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		CopyMemory(lf.lfFaceName,"MS Sans Serif",14);
		hFont=CreateFontIndirect(&lf);
		oldfnt=SelectObject(hDC,hFont);
		stop=FALSE;
		bmps=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TREEBITMAPS));
		hDC2=CreateCompatibleDC(hDC);
		oldbmp=SelectObject(hDC2,bmps);
		GetClientRect(hWnd,&rect);
		y=-td->yoff;
		oldpen=SelectObject(hDC,GetStockObject(BLACK_PEN));
		oldbrush=SelectObject(hDC,GetStockObject(td->focus ? LTGRAY_BRUSH : WHITE_BRUSH));
		SetBkMode(hDC,TRANSPARENT);
		SetTextColor(hDC,0);
		do {
			do {
				if(y>-18)
					if(crd[level].x>=0) {
						MoveToEx(hDC,crd[level].x,crd[level].y,NULL);
						LineTo(hDC,x+5,y+8);
						LineTo(hDC,x+13,y+8);
					}
				if(((CMyTreeNode*)tree->getnode())->getlnode()==NULL) {
					BitBlt(hDC,x,y+3,11,11,hDC2,tree->opened() ? 16 : 0,0,SRCCOPY);
					xx=12;
					crd[level].y=y+13;
				}else {xx=12; crd[level].y=y+8;}
				crd[level].x=x+5;
				if(y>-18) {
					switch(level) {
					case 0:
						switch(((CMyTreeNode*)tree->getnode())->GetGroup()) {
						case 0: BitBlt(hDC,x+xx+2,y,16,16,hDC2,32,0,SRCCOPY); break;
						case 1: BitBlt(hDC,x+xx+2,y,16,16,hDC2,48,0,SRCCOPY); break;
						case 2: BitBlt(hDC,x+xx+2,y,16,16,hDC2,64,0,SRCCOPY); break;
						}
						xx+=20;
						break;
					case 1:
						switch(((CMyTreeNode*)tree->getnode())->GetType()) {
						case 0: BitBlt(hDC,x+xx+2,y,16,16,hDC2,112,0,SRCCOPY); break;
						case 1: BitBlt(hDC,x+xx+2,y,16,16,hDC2,80,0,SRCCOPY); break;
						case 2: BitBlt(hDC,x+xx+2,y,16,16,hDC2,96,0,SRCCOPY); break;
						case 3: BitBlt(hDC,x+xx+2,y,16,16,hDC2,128,0,SRCCOPY); break;
						case 4: BitBlt(hDC,x+xx+2,y,16,16,hDC2,144,0,SRCCOPY); break;
						case 5: BitBlt(hDC,x+xx+2,y,16,16,hDC2,160,0,SRCCOPY); break;
						case 6: BitBlt(hDC,x+xx+2,y,16,16,hDC2,176,0,SRCCOPY); break;
						case 7: BitBlt(hDC,x+xx+2,y,16,16,hDC2,64,0,SRCCOPY); break;
						}
						xx+=20;
						break;
					}
					len=strlen( ((CMyTreeNode*)tree->getnode())->gettext() );
					if(curpos==tree->getnode())
					{
						GetTextExtentPoint32(hDC,((CMyTreeNode*)tree->getnode())->gettext(),len,&size);
						Rectangle(hDC,x+xx-1,y,x+xx+1+size.cx,y+2+size.cy);
					}
					TextOut(hDC,x+xx,y+2,((CMyTreeNode*)tree->getnode())->gettext(),len);
				}
				if(y>rect.bottom) {stop=TRUE; break;}
				y+=18;
				if(tree->opened()) {
					if(!tree->open()) {
						if(!tree->next())
							break;
					}else {x+=16; level++; crd[level].x=x+5; crd[level].y=y-4;}
				}else {
					if(!tree->next())
						break;
				}
			} while(1);
			tree->first();
			do {
				if(!tree->close()) {stop=TRUE; break;}
				else {x-=16; crd[level].x=-1; level--;}
			} while(!tree->next());
		} while(!stop);
		SelectObject(hDC,oldpen);
		SelectObject(hDC,oldbrush);
		SelectObject(hDC2,oldbmp);
		DeleteObject(bmps);
		DeleteDC(hDC);
		SelectObject(hDC,oldfnt);
		DeleteObject(hFont);
		tree->restorewindow();
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		DestroyWindow(td->hWnd);
		delete td->tree;
		delete td;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
