//File:tree.cpp//

#include <windows.h>

#include "stmp.rh"

#include "list.hpp"
#include "stamp.hpp"
#include "ctree.hpp"
#include "tree.hpp"

VOID CALLBACK DottedLineProc(int x,int y,LPARAM lpData);

int TreeList::size(void)
{
	return num;
}
void TreeList::setoff(short yo){
	short y=0;
	if(!first()) return;
	do {y++; if(y==yo) break; } while(next());
}
short TreeList::getoff(void)
{
	TreeListNode* old=(TreeListNode*)getwin();
	short y=0;
	if(isEmpty()) return -1;
	while(prev()) y++;
	setwin(old);
	return y;
}
void Tree::expand(void)
{
	if(!getwin()) return;
	((TreeNode*)getwin())->expand();
}
void Tree::shrink(void)
{
	if(!getwin()) return;
	((TreeNode*)getwin())->shrink();
}
int Tree::prevgroup(void)
{
	TreeListNode* old=(TreeListNode*)tlist->getwin();
	while(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode() != NULL )
		if(!tlist->prev()) break;
	if(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode()!=NULL)
		tlist->setwin(old);
	else{
		while(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode()==NULL)
			if(!tlist->prev()) break;
		if(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode()==NULL) tlist->setwin(old);
	}
	setwin(((TreeListNode*)tlist->getwin())->getnode());
	if(((TreeNode*)getwin())->getnode()==NULL) {
		reconstruct(); return -1;
	}
	return 0;
}
int Tree::nextgroup(void)
{
	TreeListNode* old=(TreeListNode*)tlist->getwin();
	while(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode() != NULL )
		if(!tlist->next()) break;
	if(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode()!=NULL)
		tlist->setwin(old);
	else{
		while(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode()==NULL)
			if(!tlist->next()) break;
		if(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode()==NULL) tlist->setwin(old);
	}
	setwin(((TreeListNode*)tlist->getwin())->getnode());
	if(((TreeNode*)getwin())->getnode()==NULL) {
		reconstruct(); return -1;
	}
	return 0;
}
void Tree::seekobject(StampNode* sn)
{
	BOOL stop=FALSE;
	TreeNode* tn,*old;
	old=(TreeNode*)getwin();
	if(!toroot()) return;
	do {
		do {
			tn=(TreeNode*)getwin();
			if(tn->getnode()==sn) {
				while(up()) ((TreeNode*)getwin())->expand();
				setwin(tn);
				initlist();
				InvalidateRect(hTreeWnd,NULL,FALSE);
				return;
			}
			if(tn->isOpened()!=-1) {
				if(!down()) {
					if(!next())
						break;
				}
			}else {
				if(!next())
					break;
			}
		} while(1);
		do {
			if(!up()) {stop=TRUE; break;}
		} while(!next());
	} while(!stop);
	setwin(old);
}
BOOL Tree::findgroup(short g)
{
	BOOL stop=FALSE,finded=FALSE;
	TreeNode* tn,*old;
	old=(TreeNode*)getwin();
	if(!toroot()) return NULL;
	do {
		do {
			tn=(TreeNode*)getwin();
			if(tn->getnode()==NULL) {
				if(tn->gettype()==2) {
					if(((TreeGroupNode*)tn)->getval()==g) {
						return TRUE;
					}
				}
			}
			if(tn->isOpened()!=-1) {
				if(!down()) {
					if(!next())
						break;
				}
			}else {
				if(!next())
					break;
			}
		} while(1);
		do {
			if(!up()) {stop=TRUE; break;}
		} while(!next());
	} while(!stop);
	if(!finded) setwin(old);
	return finded;
}
BOOL Tree::findtype(short t)
{
	BOOL stop=FALSE,finded=FALSE;
	TreeNode* tn,*old;
	old=(TreeNode*)getwin();
	if(!toroot()) return NULL;
	do {
		do {
			tn=(TreeNode*)getwin();
			if(tn->getnode()==NULL) {
				if(tn->gettype()==3) {
					if(((TreeTypeNode*)tn)->getval()==t) {
						return TRUE;
					}
				}
			}
			if(tn->isOpened()!=-1) {
				if(!down()) {
					if(!next())
						break;
				}
			}else {
				if(!next())
					break;
			}
		} while(1);
		do {
			if(!up()) {stop=TRUE; break;}
		} while(!next());
	} while(!stop);
	if(!finded) setwin(old);
	return finded;
}
void Tree::insertobject(StampNode* sn)
{
	if(!sn) return;
	if(!findgroup(sn->getgroup())) {
		toroot();
		insertchild(new TreeGroupNode(stmp,sn));
	}
	if(!findtype(sn->gettype())) {
		insertchild(new TreeTypeNode(stmp,sn));
	}
	insertchild(new TreeNode(sn));
	initlist();
	InvalidateRect(hTreeWnd,NULL,FALSE);
//  RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
	RECT rect;
	GetClientRect(hTreeWnd,&rect);
	SetScrollRange(hTreeWnd,SB_VERT,0,tlist->size()*18-rect.bottom,TRUE);
}
void Tree::removeobject(StampNode* sn)
{
	BOOL stop=FALSE;
	TreeNode* tn,*old;
	old=(TreeNode*)getwin();
	if(!toroot()) return;
	do {
		do {
			tn=(TreeNode*)getwin();
			if(tn==NULL) return;
			if((StampNode*)tn->getnode()==sn) {
				remove();
				initlist();
				InvalidateRect(hTreeWnd,NULL,FALSE);
				return;
			}
			if(tn->isOpened()!=-1) {
				if(!down()) {
					if(!next())
						break;
				}
			}else {
				if(!next())
					break;
			}
		} while(1);
		do {
			if(!up()) {stop=TRUE; break;}
		} while(!next());
	} while(!stop);
	setwin(old);
}

void Tree::reconstruct(void)
{
	StampNode* sn;
	toroot();
	remove();
	insert(new HeaderNode(stmp));
	expand();
	if(!stmp->first()) {
		initlist();
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
		return;
	}
	do {
		sn=(StampNode*)stmp->getwin();
		if(!sn) break;
		if(!findgroup(sn->getgroup())) {
			toroot();
			insertchild(new TreeGroupNode(stmp,sn));
		}
		expand();
		if(!findtype(sn->gettype())) {
			insertchild(new TreeTypeNode(stmp,sn));
		}
		expand();
		insertchild(new TreeNode(sn));
	} while(stmp->next());
	initlist();
	RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
}

void Tree::draw(HDC hDC)
{
	char txt[128];
	POINT crd[6]={{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
	RECT rect;
	int level=0,show=-1;
	HDC hDC2;
	short len;
	DTLPDATA dtlData;
	HBITMAP oldbmp,oldbmp2,tbmp;
	HPEN oldpen;
	HBRUSH oldbrush;
	HFONT oldfont;
	BOOL stop=FALSE;
	SIZE size;
	int y,x,xx,tx,ty;
	TreeNode* tn,* curpos;
	curpos=(TreeNode*)getwin();
	if(!toroot()) return;
	y=-yoff+1; x=5; xx=0;
	hDC2=CreateCompatibleDC(hDC);
	oldbmp=SelectObject(hDC2,bmps);
	GetClientRect(hTreeWnd,&rect);
	oldfont=SelectObject(hDC,hFont);
	oldpen=SelectObject(hDC,GetStockObject(BLACK_PEN));
	oldbrush=SelectObject(hDC,GetStockObject(WHITE_BRUSH));
	PatBlt(hDC,0,0,rect.right,rect.bottom,WHITENESS);
	SelectObject(hDC,oldbrush);
	oldbrush=SelectObject(hDC,GetStockObject(GetFocus()==hTreeWnd ? LTGRAY_BRUSH : WHITE_BRUSH));
	SetBkMode(hDC,TRANSPARENT);
	SetTextColor(hDC,0);
	dtlData.hDC=hDC;
	do {
		do {
			tn=(TreeNode*)getwin();
			if(tn==NULL) return;
			if(y>-18||show!=-1)
				if(crd[level].x>=0) {
					dtlData.c=0;
					LineDDA(crd[level].x,crd[level].y,x+5,y+8,(LINEDDAPROC)DottedLineProc,(LPARAM)&dtlData);
					dtlData.c=0;
					LineDDA(x+5,y+8,x+13,y+8,(LINEDDAPROC)DottedLineProc,(LPARAM)&dtlData);
				}
			if(tn->getnode()==NULL) {
				BitBlt(hDC,x,y+3,11,11,hDC2,tn->isOpened() ? 16 : 0,0,SRCCOPY);
				xx=12;
				crd[level].y=y+13;
			}else {xx=12; crd[level].y=y+8;}
			crd[level].x=x+5;
			if(y>-18||show!=-1) {
				switch(tn->gettype()) {
				case 2: tbmp=((TreeGroupNode*)tn)->geticon();
					tx=((TreeGroupNode*)tn)->geticonx(); ty=((TreeGroupNode*)tn)->geticony();
					((TreeGroupNode*)tn)->gettext(txt);
					break;
				case 3: tbmp=((TreeTypeNode*)tn)->geticon();
					tx=((TreeTypeNode*)tn)->geticonx(); ty=((TreeTypeNode*)tn)->geticony();
					((TreeTypeNode*)tn)->gettext(txt);
					break;
				default: tbmp=tn->geticon();
					tx=tn->geticonx(); ty=tn->geticony();
					tn->gettext(txt);
					break;
				}
				if(tbmp) {
					oldbmp2=SelectObject(hDC2,tbmp);
					BitBlt(hDC,x+xx+2,y,16,16,
					       hDC2,tx,ty,SRCCOPY);
					xx+=20;
					SelectObject(hDC2,oldbmp2);
				}
				len=(short)lstrlen(txt);
				if(curpos==tn)
				{
					GetTextExtentPoint32(hDC,txt,len,&size);
					Rectangle(hDC,x+xx-1,y,x+xx+1+size.cx,y+3+size.cy);
				}
				TextOut(hDC,x+xx,y+2,txt,len);
			}
			if(y>rect.bottom&&show!=-1) {stop=TRUE; break;}
			y+=18;
			if(tn->isOpened()==1) {
				if(getnext()) {if(show>level) show=level; }else if(show>level) show=-1;
				if(!down()) {
					if(!next())
						break;
				}else {x+=16; level++; crd[level].x=x+5; crd[level].y=y-4;}
			}else {
				if(!next())
					break;
			}
		} while(1);
		do {
			if(!up()) {stop=TRUE; break;}
			else {x-=16; crd[level].x=-1; if(show==level) show=-1; level--;}
		} while(!next());
	} while(!stop);
	SelectObject(hDC,oldfont);
	SelectObject(hDC,oldpen);
	SelectObject(hDC,oldbrush);
	SelectObject(hDC2,oldbmp);
	DeleteDC(hDC2);
	setwin(curpos);
}
#pragma argsused
void Tree::initlist(void)
{
	RECT rect,rect2;
	char txt[128];
	short len;
	HFONT oldfont;
	HBITMAP tbmp;
	HDC hDC;
	BOOL stop=FALSE;
	SIZE size;
	int y,x,num=0;
	short ypos,yp;

	TreeNode* tn,* curpos;
	tlist->clear();
	curpos=(TreeNode*)getwin();
	if(!toroot()) return;
	y=-yoff+1; x=5; ypos=yp=1;
	hDC=CreateCompatibleDC(NULL);
	GetClientRect(hTreeWnd,&rect);
	oldfont=SelectObject(hDC,hFont);
	do {
		do {
			tn=(TreeNode*)getwin();
			if(!tn) return;
			switch(tn->gettype()) {
			case 2: tbmp=((TreeGroupNode*)tn)->geticon();
				((TreeGroupNode*)tn)->gettext(txt);
				break;
			case 3: tbmp=((TreeTypeNode*)tn)->geticon();
				((TreeTypeNode*)tn)->gettext(txt);
				break;
			default: tbmp=tn->geticon();
				tn->gettext(txt);
				break;
			}
			len=(short)lstrlen(txt);
			GetTextExtentPoint32(hDC,txt,len,&size);
			rect2.left=x+11; rect2.top=y-1; rect2.right=x+13+size.cx; rect2.bottom=y+17;
			if(tbmp) rect2.right+=20;
			tlist->insert(new TreeListNode(tn,&rect2)); num++;
			if(curpos==tn) ypos=yp;
			y+=18; yp++;
			if(tn->isOpened()==1) {
				if(!down()) {
					if(!next())
						break;
				}else x+=16;
			}else {
				if(!next())
					break;
			}
		} while(1);
		do {
			if(!up()) {stop=TRUE; break;}
			else x-=16;
		} while(!next());
	} while(!stop);
	tlist->setoff(ypos);
	tlist->setsize(num);
	SelectObject(hDC,oldfont);
	DeleteDC(hDC);
	setwin(curpos);
	GetClientRect(hTreeWnd,&rect);
	SetScrollRange(hTreeWnd,SB_VERT,0,tlist->size()*18-rect.bottom,TRUE);
}
void Tree::setkillfocus(void)
{
	RECT rect;
	TreeListNode* tln=(TreeListNode*)tlist->getwin();
	if(!tln) return;
	if(wndpos!=-1) {ShowWindow(hTipsWnd,SW_HIDE); wndpos=-1;}
	tln->getrect(&rect);
	InvalidateRect(hTreeWnd,&rect,FALSE);
}
void Tree::keyup(void){
	RECT rect,rect2,wrect;
	TreeListNode* tln=(TreeListNode*)tlist->getwin();
	if(!tln) return;
	if(wndpos!=-1) {ShowWindow(hTipsWnd,SW_HIDE); wndpos=-1;}
	tln->getrect(&rect);
	if(tlist->prev()) {
		tln=(TreeListNode*)tlist->getwin();
		if(!tln) return;
		tln->getrect(&rect2);
		setwin(tln->getnode());
		GetClientRect(hTreeWnd,&wrect);
		if(rect2.top<wrect.top) {
			yoff-=wrect.top-rect2.top;
//      SetScrollRange(hTreeWnd,SB_VERT,0,tlist->size()*18-wrect.bottom,TRUE);
			SetScrollPos(hTreeWnd,SB_VERT,yoff,TRUE);
			initlist();
			RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
		}else{
			RedrawWindow(hTreeWnd,&rect2,NULL,RDW_INVALIDATE);
			RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
		}
	}
}
void Tree::keydown(void){
	RECT rect,rect2,wrect;
	TreeListNode* tln=(TreeListNode*)tlist->getwin();
	if(!tln) return;
	if(wndpos!=-1) {ShowWindow(hTipsWnd,SW_HIDE); wndpos=-1;}
	tln->getrect(&rect);
	if(tlist->next()) {
		tln=(TreeListNode*)tlist->getwin();
		if(!tln) return;
		tln->getrect(&rect2);
		setwin(tln->getnode());
		GetClientRect(hTreeWnd,&wrect);
		if(rect2.bottom>wrect.bottom) {
			yoff+=rect2.bottom-wrect.bottom;
			SetScrollPos(hTreeWnd,SB_VERT,yoff,TRUE);
//      SetScrollRange(hTreeWnd,SB_VERT,0,tlist->size()*18-wrect.bottom,TRUE);
			initlist();
			RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
		}else{
			RedrawWindow(hTreeWnd,&rect2,NULL,RDW_INVALIDATE);
			RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
		}
	}
}
void Tree::keyleft(void){
	RECT rect,rect2;
	TreeListNode* tln=(TreeListNode*)tlist->getwin();
	if(!tln) return;
	if(wndpos!=-1) {ShowWindow(hTipsWnd,SW_HIDE); wndpos=-1;}
	if(((TreeNode*)getwin())->isOpened()==1) {
		shrink();
		initlist();
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
	}else{
		tln->getrect(&rect);
		if(prev()) {
			initlist();
			((TreeListNode*)tlist->getwin())->getrect(&rect2);
			RedrawWindow(hTreeWnd,&rect2,NULL,RDW_INVALIDATE);
			RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
		}else{
			if(up()) {
				tlist->prev();
				((TreeListNode*)tlist->getwin())->getrect(&rect2);
				RedrawWindow(hTreeWnd,&rect2,NULL,RDW_INVALIDATE);
				RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
			}
		}
	}
}
void Tree::keyright(void){
	RECT rect,rect2;
	TreeListNode* tln=(TreeListNode*)tlist->getwin();
	if(!tln) return;
	if(wndpos!=-1) {ShowWindow(hTipsWnd,SW_HIDE); wndpos=-1;}
	if(((TreeNode*)getwin())->isOpened()==0) {
		expand();
		initlist();
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
	}else{
		tln->getrect(&rect);
		if(((TreeNode*)getwin())->isOpened()==1)
			if(down()) {
				tlist->next();
				((TreeListNode*)tlist->getwin())->getrect(&rect2);
				RedrawWindow(hTreeWnd,&rect2,NULL,RDW_INVALIDATE);
				RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
			}else{
				if(next()) {
					tlist->next();
					((TreeListNode*)tlist->getwin())->getrect(&rect2);
					RedrawWindow(hTreeWnd,&rect2,NULL,RDW_INVALIDATE);
					RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
				}
			}
	}
}
void Tree::keymenu(void)
{
	SendMessage(hMainWnd,WM_PANELMENU,
	            (WPARAM)((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->gettype(),
	            (LPARAM)((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode());
/*  switch(((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->gettype())
   {
    case 0:
      MessageBox(hMainWnd,"Popup menu for object","POPUP",MB_OK);
      break;
    case 1:
      MessageBox(hMainWnd,"Popup menu for HEADER","POPUP",MB_OK);
      break;
    case 2:
      MessageBox(hMainWnd,"Popup menu for GROUP","POPUP",MB_OK);
      break;
    case 3:
      MessageBox(hMainWnd,"Popup menu for TYPE","POPUP",MB_OK);
      break;
   }*/
}
void Tree::keyenter(void){
	if(((TreeNode*)getwin())->isOpened()!=-1) {
		if(((TreeNode*)getwin())->isOpened()==0) expand(); else shrink();
		initlist();
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
		return;
	}else{
		SendMessage(hMainWnd,WM_PANELSELECTED,0,(LPARAM)((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode());
	}
}
void Tree::lclick(int x,int y)
{
	TreeListNode* old=(TreeListNode*)tlist->getwin();
	RECT rect,oldrect,wrect;
	POINT pt={x,y};
	if(GetFocus()!=hTreeWnd) SetFocus(hTreeWnd);
	((TreeListNode*)tlist->getwin())->getrect(&oldrect);
	tlist->first();
	GetClientRect(hTreeWnd,&wrect);
	do {
		((TreeListNode*)tlist->getwin())->getrect(&rect);
		if((x>=rect.left-11&&y>=rect.top+3&&x<rect.left&&y<=rect.top+14)&&
		   ((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->isOpened()!=-1) {
			setwin(((TreeListNode*)tlist->getwin())->getnode());
			if(((TreeNode*)getwin())->isOpened()==0) expand(); else shrink();
			initlist();
			RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
			return;
		}else
		if(PtInRect(&rect,pt)) {
			setwin(((TreeListNode*)tlist->getwin())->getnode());
			if(rect.top<wrect.top) {
				yoff-=wrect.top-rect.top;
				ScrollWindow(hTreeWnd,0,-rect.top+wrect.top,NULL,NULL);
				SetScrollPos(hTreeWnd,SB_VERT,yoff,TRUE);
				initlist();
				oldrect.top+=wrect.top-rect.top;
				oldrect.bottom+=wrect.top-rect.top;
				InvalidateRect(hTreeWnd,&oldrect,FALSE);
				((TreeListNode*)tlist->getwin())->getrect(&rect);
				InvalidateRect(hTreeWnd,&rect,FALSE);
				UpdateWindow(hTreeWnd);
			}else
			if(rect.bottom>wrect.bottom) {
				yoff+=rect.bottom-wrect.bottom;
				ScrollWindow(hTreeWnd,0,-rect.bottom+wrect.bottom,NULL,NULL);
				SetScrollPos(hTreeWnd,SB_VERT,yoff,TRUE);
				initlist();
				oldrect.top-=rect.bottom-wrect.bottom;
				oldrect.bottom-=rect.bottom-wrect.bottom;
				InvalidateRect(hTreeWnd,&oldrect,FALSE);
				((TreeListNode*)tlist->getwin())->getrect(&rect);
				InvalidateRect(hTreeWnd,&rect,FALSE);
				UpdateWindow(hTreeWnd);
			}else{
				RedrawWindow(hTreeWnd,&oldrect,NULL,RDW_INVALIDATE);
				RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
			}
			return;
		}
	} while(tlist->next());
	tlist->setwin(old);
}
void Tree::ldblclk(int x,int y)
{
	TreeListNode* old=(TreeListNode*)tlist->getwin();
	RECT rect,oldrect;
	POINT pt={x,y};
	if(GetFocus()!=hTreeWnd) SetFocus(hTreeWnd);
	((TreeListNode*)tlist->getwin())->getrect(&oldrect);
	tlist->first();
	do {
		((TreeListNode*)tlist->getwin())->getrect(&rect);
		if(PtInRect(&rect,pt)) {
			setwin(((TreeListNode*)tlist->getwin())->getnode());
			if(((TreeNode*)getwin())->isOpened()!=-1) {
				if(((TreeNode*)getwin())->isOpened()==0) expand(); else shrink();
				initlist();
				RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
				return;
			}else{
				SendMessage(hMainWnd,WM_PANELSELECTED,0,(LPARAM)((TreeNode*)((TreeListNode*)tlist->getwin())->getnode())->getnode());
				RedrawWindow(hTreeWnd,&rect,NULL,RDW_INVALIDATE);
				RedrawWindow(hTreeWnd,&oldrect,NULL,RDW_INVALIDATE);
			}
		}
	} while(tlist->next());
	tlist->setwin(old);
}
void Tree::rclick(int x,int y)
{
	MessageBeep(MB_OK);
}

inline int min(int a,int b){
	return a<b ? a : b;
}
void Tree::vscroll(int wParam)
{
	int oldyoff=yoff;
	RECT rect;
	if(wndpos!=-1) {ShowWindow(hTipsWnd,SW_HIDE); wndpos=-1;}
	GetClientRect(hTreeWnd,&rect);
	switch(LOWORD(wParam)) {
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		yoff=min((short)HIWORD(wParam),tlist->size()*18-rect.bottom);
		break;
	case SB_PAGEDOWN:
		yoff=min(yoff+(rect.bottom>>1),tlist->size()*18-rect.bottom);
		break;
	case SB_PAGEUP:
		yoff=min(yoff-(rect.bottom>>1),tlist->size()*18-rect.bottom);
		break;
	case SB_LINEUP:
		yoff=min(yoff-18,tlist->size()*18-rect.bottom);
		break;
	case SB_LINEDOWN:
		yoff=min(yoff+18,tlist->size()*18-rect.bottom);
		break;
	}
	if(yoff<0) yoff=0;
	SetScrollPos(hTreeWnd,SB_VERT,yoff,TRUE);
	initlist();
	ScrollWindow(hTreeWnd,0,oldyoff-yoff,NULL,NULL);
	UpdateWindow(hTreeWnd);
//  RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
}

void Tree::mousemove(int x,int y)
{
	RECT rect,rect2;
	char txt[128];
	HBITMAP tbmp;
	int i=0,ii;
	TreeNode* tn;
	POINT pt={x,y};
	TreeListNode* old=((TreeListNode*)tlist->getwin());
	if(!tlist->first()) return;
	GetClientRect(hTreeWnd,&rect);
	do {
		((TreeListNode*)tlist->getwin())->getrect(&rect2);
		if(PtInRect(&rect2,pt)) {
			if(i!=wndpos) {
				ShowWindow(hTipsWnd,SW_HIDE);
				wndpos=-1;
			}
			if(rect2.right>rect.right&&wndpos!=i) {
				GetWindowRect(hTreeWnd,&rect);
				tn=(TreeNode*)((TreeListNode*)tlist->getwin())->getnode();
				switch(tn->gettype()) {
				case 2: tbmp=((TreeGroupNode*)tn)->geticon();
					((TreeGroupNode*)tn)->gettext(txt);
					break;
				case 3: tbmp=((TreeTypeNode*)tn)->geticon();
					((TreeTypeNode*)tn)->gettext(txt);
					break;
				default: tbmp=tn->geticon();
					tn->gettext(txt);
					break;
				}
				if(tbmp) ii=21; else ii=1;
				SetWindowText(hTipsWnd,txt);
				MoveWindow(hTipsWnd,
				           rect2.left+rect.left+ii,
				           rect2.top+rect.top+3,
				           rect2.right-rect2.left-ii+3,
				           rect2.bottom-rect2.top-2,
				           FALSE);
				ShowWindow(hTipsWnd,SW_SHOWNOACTIVATE);
				SendMessage(hTipsWnd,WM_USER,(WPARAM)&rect2,(LPARAM)hTreeWnd);
				wndpos=i;
			}
			break;
		}
		i++;
	} while(tlist->next());
	tlist->setwin(old);
}

typedef struct
{
	HWND parent;
	RECT rect;
	BOOL showed;
} THELPIK,*PTHELPIK;
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
	int len;
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
	RECT rect;
	PAINTSTRUCT ps;
	Tree* tree=(Tree*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_CREATE:
		SetScrollRange(hWnd,SB_VERT,0,0,TRUE);
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		if(tree) tree->setkillfocus();
		break;
	case WM_SYSKEYDOWN:
		if(lParam&0x20000000) {
			if(wParam==VK_RETURN)
				if(tree) tree->keymenu();
		}
		break;
	case WM_SETCURSOR:
		if(tree) SendMessage(tree->GetMainWnd(),WM_SETSTATUSTEXT,IDS_TREETEXT,0);
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	case WM_KEYUP:
		if(wParam==VK_RETURN) if(tree) tree->keyenter();
		break;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_DOWN: tree->keydown(); break;
		case VK_UP: tree->keyup(); break;
		case VK_LEFT: tree->keyleft(); break;
		case VK_RIGHT: tree->keyright(); break;
		}
		break;
	case WM_VSCROLL:
		if(tree) tree->vscroll(wParam);
		break;
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;  //|DLGC_BUTTON;
	case WM_MOUSEMOVE:
		if(tree) tree->mousemove(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_LBUTTONDBLCLK:
		if(tree) tree->ldblclk(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		if(tree) tree->lclick(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_RBUTTONDOWN:
		if(tree) tree->rclick(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rect);
		if(tree==NULL)
			PatBlt(ps.hdc,0,0,rect.right,rect.bottom,WHITENESS);
		else tree->draw(ps.hdc);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		if(tree) tree->clear();
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
