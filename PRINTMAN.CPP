//File:printman.cpp//

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "commdlg3.h"

#include "list.hpp"
#include "stamp.hpp"
#include "rect.hpp"
#include "ltext.hpp"
#include "ctext.hpp"
#include "picbmp.hpp"
#include "circle.hpp"
#include "line.hpp"
#include "triangle.hpp"
#include "printman.rh"
#include "printman.hpp"


BOOL CALLBACK PrintManagerProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK PMDlgSmallPreviewWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK PrintManagerProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

HWND WndMain;
HINSTANCE hInst;
HBITMAP PreviewBmp;
PrintManager* pm;
BOOL stopprint=FALSE;
BOOL InitApp(HINSTANCE hInstance){
	WNDCLASS wc;
	hInst=hInstance;
	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc=PMDlgSmallPreviewWnd;
	wc.cbClsExtra=wc.cbWndExtra=0;
	wc.hInstance=hInst;
	wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName="StampDlgPreviewWnd";
	if(!RegisterClass(&wc)) {
		MessageBox(NULL,"Cannot register class 'PrintManDlgPreview'",
		           "Error in printman.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.lpfnWndProc=PrintManagerPreviewProc;
	wc.lpszMenuName=NULL;
	wc.hbrBackground=GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName="PrintManPMPreview";
	if(!RegisterClass(&wc)) {
		MessageBox(NULL,"Cannot register class 'PrintManPMPreview'",
		           "Error in printman.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	pm=new PrintManager();
	WndMain=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PRINTMANAGER),NULL,
	                          (DLGPROC)PrintManagerProc,(LPARAM)pm);
	SetWindowLong(GetDlgItem(WndMain,IDC_PMPREVIEW),GWL_USERDATA,(LONG)pm);
	pm->setwnd(WndMain);
	if(!WndMain) {
		MessageBox(NULL,"Cannot create main window",
		           "Error in printman.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	RedrawWindow(GetDlgItem(WndMain,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
	return TRUE;
}

#pragma argsused
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpszCmdLine,int nCmdShow)
{
	MSG msg;
	if(hPrevInstance) return 0;
	if(!InitApp(hInstance)) return 0;
	while(GetMessage(&msg,NULL,0,0)) {
		if(IsDialogMessage(WndMain,&msg)==FALSE) {
			TranslateMessage(&msg);    // Translates virtual key codes.
			DispatchMessage(&msg);     // Dispatches message to window.
		}
	}
	UnregisterClass("PrintManPMPreview",hInstance);
	UnregisterClass("StampDlgPreviewWnd",hInstance);
	return msg.wParam;
}

void PrintManager::setupprinter(void)
{
	PRINTDLG pd;
	LPDEVMODE pDevMode;
	LPDEVNAMES pDevNames;
	HDC hDC;
	pd.lStructSize=sizeof(PRINTDLG);
	pd.hwndOwner=hWnd;
	pd.hDevMode=hDevMode;
	pd.hDevNames=hDevNames;
	pd.Flags=PD_NOPAGENUMS|PD_NOSELECTION|PD_PRINTSETUP;
	pd.nFromPage=0; pd.nToPage=0;
	pd.nMinPage=0; pd.nMaxPage=0;
	pd.nCopies=0;
	pd.hInstance=hInst;
	pd.lCustData=0;
	pd.lpfnPrintHook=NULL;
	pd.lpfnSetupHook=NULL;
	pd.lpPrintTemplateName=NULL;
	pd.lpSetupTemplateName=NULL;
	pd.hPrintTemplate=NULL;
	pd.hSetupTemplate=NULL;
	if(PrintDlg(&pd)==TRUE) {
		hDevMode=pd.hDevMode; hDevNames=pd.hDevNames;
		if(hDevMode!=NULL) {
			pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
			pDevNames=(LPDEVNAMES)GlobalLock(hDevNames);
			hDC=CreateDC((LPCTSTR)(pDevNames+pDevNames->wDriverOffset),
			             (LPCTSTR)(pDevMode->dmDeviceName),NULL,pDevMode);
			pagewidth=GetDeviceCaps(hDC,HORZSIZE)*10;
			pageheight=GetDeviceCaps(hDC,VERTSIZE)*10;
			DeleteDC(hDC);
			GlobalUnlock(hDevMode);
			GlobalUnlock(hDevNames);
		}
	}else
	if(hDevMode!=NULL||hDevNames!=NULL)
		switch(CommDlgExtendedError()) {
		case PDERR_PRINTERNOTFOUND:
		case PDERR_NODEVICES:
			pd.hDevNames=NULL;
			pd.hDevMode=NULL;
			if(hDevMode) GlobalFree(hDevMode);
			if(hDevMode) GlobalFree(hDevMode);
			pd.Flags=PD_NOPAGENUMS|PD_NOSELECTION|PD_PRINTSETUP;
			if(PrintDlg(&pd)!=TRUE) {
				hDevMode=pd.hDevMode; hDevNames=pd.hDevNames;
				if(hDevMode!=NULL) {
					pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
					pDevNames=(LPDEVNAMES)GlobalLock(hDevNames);
					hDC=CreateDC((LPCTSTR)(pDevNames+pDevNames->wDriverOffset),
					             (LPCTSTR)(pDevMode->dmDeviceName),NULL,pDevMode);
					pagewidth=GetDeviceCaps(hDC,HORZSIZE)*10;
					pageheight=GetDeviceCaps(hDC,VERTSIZE)*10;
					DeleteDC(hDC);
					GlobalUnlock(hDevMode);
					GlobalUnlock(hDevNames);
				}
			} break;
		}
}
void PrintManager::getprintername(char*s)
{
	LPDEVMODE pDevMode;
	if(hDevMode!=NULL) {
		pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
		strncpy(s,(char*)(pDevMode->dmDeviceName),32);
		GlobalUnlock(hDevMode);
	}else strcpy(s,"unknown");
}
void PMList::insertdummy(ListNode*ln)
{
	int id;
	id=(int)SendMessage(hWnd,LB_ADDSTRING,0,(LPARAM)((PMListNode*)ln)->getname());
	((PMListNode*)ln)->setid(id);
	PMListNode* old=(PMListNode*)getwin();
	if(first()) {
		do {
			if(((PMListNode*)getwin())->getid()>=id&&getwin()!=ln) {
				((PMListNode*)getwin())->setid(((PMListNode*)getwin())->getid()+1);
			}
		} while(next());
		setwin(old);
	}
}
void PMList::removedummy(ListNode* ln)
{
	int id;
	PMListNode* old=(PMListNode*)getwin();
	if(old) {
		id=((PMListNode*)ln)->getid();
		SendMessage(hWnd,LB_DELETESTRING,id,0);
	}
	if(first()) {
		do {
			if(((PMListNode*)getwin())->getid()>id)
				((PMListNode*)getwin())->setid(((PMListNode*)getwin())->getid()-1);
		} while(next());
		setwin(old);
	}
}
void PrintManager::getdefaultprinter(void)
{
	HDC hDC;
	LPDEVMODE pDevMode;
	LPDEVNAMES pDevNames;
	if(hDevMode) GlobalFree(hDevMode);
	if(hDevNames) GlobalFree(hDevNames);
	PRINTDLG pd;
	pd.lStructSize=sizeof(PRINTDLG);
	pd.hwndOwner=NULL;
	pd.hDevMode=0;
	pd.hDevNames=0;
	pd.Flags=PD_NOPAGENUMS|PD_NOSELECTION|PD_PRINTSETUP|PD_RETURNDEFAULT;
	pd.nFromPage=0; pd.nToPage=0;
	pd.nMinPage=0; pd.nMaxPage=0;
	pd.nCopies=0;
	pd.hInstance=hInst;
	pd.lCustData=0;
	pd.lpfnPrintHook=NULL;
	pd.lpfnSetupHook=NULL;
	pd.lpPrintTemplateName=NULL;
	pd.lpSetupTemplateName=NULL;
	pd.hPrintTemplate=NULL;
	pd.hSetupTemplate=NULL;
	if(PrintDlg(&pd)==TRUE) {
		hDevMode=pd.hDevMode; hDevNames=pd.hDevNames;
		if(hDevMode!=NULL) {
			pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
			pDevNames=(LPDEVNAMES)GlobalLock(hDevNames);
			hDC=CreateDC((LPCTSTR)(pDevNames+pDevNames->wDriverOffset),
			             (LPCTSTR)(pDevMode->dmDeviceName),NULL,pDevMode);
			pagewidth=GetDeviceCaps(hDC,HORZSIZE)*10;
			pageheight=GetDeviceCaps(hDC,VERTSIZE)*10;
			DeleteDC(hDC);
			GlobalUnlock(hDevMode);
			GlobalUnlock(hDevNames);
		}else{
			pagewidth=2100; pageheight=2970;
		}
	}
}
PrintManager::PrintManager()
{
	HDC hDC;
	capt=FALSE;
	fname=NULL;
	pmlist=new PMList(NULL);
	hDevMode=hDevNames=NULL;
	LPDEVMODE pDevMode;
	LPDEVNAMES pDevNames;
	PRINTDLG pd;
	pd.lStructSize=sizeof(PRINTDLG);
	pd.hwndOwner=NULL;
	pd.hDevMode=0;
	pd.hDevNames=0;
	pd.Flags=PD_NOPAGENUMS|PD_NOSELECTION|PD_PRINTSETUP|PD_RETURNDEFAULT;
	pd.nFromPage=0; pd.nToPage=0;
	pd.nMinPage=0; pd.nMaxPage=0;
	pd.nCopies=0;
	pd.hInstance=hInst;
	pd.lCustData=0;
	pd.lpfnPrintHook=NULL;
	pd.lpfnSetupHook=NULL;
	pd.lpPrintTemplateName=NULL;
	pd.lpSetupTemplateName=NULL;
	pd.hPrintTemplate=NULL;
	pd.hSetupTemplate=NULL;
	if(PrintDlg(&pd)==TRUE) {
		hDevMode=pd.hDevMode; hDevNames=pd.hDevNames;
		if(hDevMode!=NULL) {
			pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
			pDevNames=(LPDEVNAMES)GlobalLock(hDevNames);
			hDC=CreateDC((LPCTSTR)(pDevNames+pDevNames->wDriverOffset),
			             (LPCTSTR)(pDevMode->dmDeviceName),NULL,pDevMode);
			pagewidth=GetDeviceCaps(hDC,HORZSIZE)*10;
			pageheight=GetDeviceCaps(hDC,VERTSIZE)*10;
			DeleteDC(hDC);
			GlobalUnlock(hDevMode);
			GlobalUnlock(hDevNames);
		}else{
			pagewidth=2100; pageheight=2970;
		}
	}
}

BOOL CALLBACK PrintManagerProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PrintManager* pm=(PrintManager*)GetWindowLong(hDlg,DWL_USER);
	char pname[64];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,DWL_USER,lParam);
		pm=(PrintManager*)lParam;
		pm->getprintername(pname);
		SetDlgItemText(hDlg,IDC_PRINTERNAME,pname);
		break;
	case WM_COMMAND:
		if(HIWORD(wParam)==LBN_SELCHANGE&&LOWORD(wParam)==IDC_FILES)
		{
			pm->listsel(SendDlgItemMessage(hDlg,IDC_FILES,LB_GETCURSEL,0,0));
		}else
			switch(LOWORD(wParam)) {
			case IDC_ADD:
				pm->append(); pm->Changed();
				break;
			case IDC_REMOVE:
				pm->remove(); pm->Changed();
				break;

			case IDC_NEW:
				pm->clear();
				RedrawWindow(GetDlgItem(hDlg,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
				break;
			case IDC_SAVE:
				pm->save();
				break;
			case IDC_SAVEAS:
				pm->saveas();
				break;
			case IDC_OPEN:
				pm->open();
				RedrawWindow(GetDlgItem(hDlg,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
				break;

			case IDC_PRINT:
				if(MessageBox(hDlg,"You really want to begin to print?",
				              "Print now",MB_YESNO)==IDYES) {
					if(pm->intersect())
						if(MessageBox(hDlg,"Want to continue to print?",
						              "Some stamps are crossed!",MB_YESNO)!=IDYES) return 0;
					pm->print();
				}
				break;
			case IDC_SETUP:
				pm->setupprinter(); pm->Changed();
				pm->getprintername(pname);
				SetDlgItemText(hDlg,IDC_PRINTERNAME,pname);
				RedrawWindow(GetDlgItem(hDlg,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
				break;
			case IDCANCEL:
				if(pm->clear()==TRUE)
					DestroyWindow(hDlg);
				break;
			}
		break;
	case WM_CLOSE:
		if(pm->clear()==TRUE)
			DestroyWindow(hDlg);
		break;
	case WM_DESTROY:
		if(PreviewBmp) DeleteObject(PreviewBmp);
		delete pm;
		PostQuitMessage(0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
BOOL PrintManager::intersect(void)
{
	PMListNode* old=(PMListNode*)pmlist->getwin();
	PMListNode* n,* n2;
	HDC hDC;
	RECT rect1,rect2,dstrect; POINT p;
	if(!pmlist->first()) return FALSE;
	do {
		n2=(PMListNode*)pmlist->getwin();
		n2->getrect(&rect1); n2->getoffset(&p);
		rect1.left+=p.x; rect1.right+=p.x;
		rect1.top=p.y-rect1.top; rect1.bottom=p.y-rect1.bottom;
		if(pmlist->next())
			do {
				n=(PMListNode*)pmlist->getwin();
				if(n!=n2) {
					n->getrect(&rect2); n->getoffset(&p);
					rect2.left+=p.x; rect2.right+=p.x;
					rect2.top=p.y-rect2.top; rect2.bottom=p.y-rect2.bottom;
					if(IntersectRect(&dstrect,&rect1,&rect2)) {
						hDC=GetDC(GetDlgItem(hWnd,IDC_PMPREVIEW));
						for(int i=0; i<2; i++) {
							MessageBeep(MB_OK);
							PatBlt(hDC,dstrect.left/zoom,dstrect.top/zoom,
							       (dstrect.right-dstrect.left+1)/zoom+1,
							       (dstrect.bottom-dstrect.top+1)/zoom+1,
							       DSTINVERT);
							Sleep(200);
						}
						ReleaseDC(hWnd,hDC);
						pmlist->setwin(old);
						return TRUE;
					}
				}
			} while(pmlist->next());
		pmlist->setwin(n2);
	} while(pmlist->next());
	pmlist->setwin(old);
	return FALSE;
}
LRESULT CALLBACK PrintManagerPreviewProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PrintManager* pm=(PrintManager*)GetWindowLong(hWnd,GWL_USERDATA);
	PMList* pmlist=NULL;
	HDC hDC;
	PAINTSTRUCT ps;
	short key;
	if(pm!=NULL) pmlist=pm->pmlist; else pmlist=NULL;
	switch(uMsg) {
	case WM_MOUSEMOVE: {
		POINT mp={(int)((short)LOWORD(lParam))*pm->zoom,(int)((short)HIWORD(lParam))*pm->zoom};
		POINT p;
		RECT rect,rect1,rect2,dstrect;
		if(pmlist==NULL) return 0;
		if(pm->capt) {
			if(pmlist==NULL) return 0;
			PMListNode* n=(PMListNode*)pmlist->getwin();
			PMListNode*old;
			n->getrect(&rect);
			hDC=GetDC(hWnd);
			PatBlt(hDC,(pm->mx+pm->oldx+rect.left)/pm->zoom,
			       (pm->my+pm->oldy-rect.top)/pm->zoom,
			       (rect.right-rect.left+1)/pm->zoom+1,
			       (rect.top-rect.bottom+1)/pm->zoom+1,
			       DSTINVERT);
			old=n;
			pm->pmlist->first();
			pm->oldx=mp.x; pm->oldy=mp.y; pm->moved=1;
			rect1.left=pm->mx+pm->oldx+rect.left; rect1.top=pm->my+pm->oldy-rect.top;
			rect1.right=pm->mx+pm->oldx+rect.right; rect1.bottom=pm->my+pm->oldy-rect.bottom;
			key=GetKeyState(VK_SHIFT)&0x8000;
			do {
				n=(PMListNode*)pmlist->getwin();
				if(n!=old) {
					n->getrect(&rect2); n->getoffset(&p);
					rect2.left+=p.x; rect2.right+=p.x; rect2.top=p.y-rect2.top; rect2.bottom=p.y-rect2.bottom;
					if(!key) {
						if(rect1.top<=rect2.bottom&&rect1.bottom>=rect2.top) {
							if(abs(rect1.left-rect2.right)<5*pm->zoom) pm->oldx=rect2.right-pm->mx-rect.left;
							if(abs(rect1.right-rect2.left)<5*pm->zoom) pm->oldx=rect2.left-pm->mx-rect.right;
							rect1.left=pm->mx+pm->oldx+rect.left; rect1.right=pm->mx+pm->oldx+rect.right;
						}
						if(rect1.left<=rect2.right&&rect1.right>=rect2.left) {
							if(abs(rect1.top-rect2.bottom)<5*pm->zoom) pm->oldy=rect2.bottom-pm->my+rect.top+1;
							if(abs(rect1.bottom-rect2.top)<5*pm->zoom) pm->oldy=rect2.top-pm->my+rect.bottom-1;
							rect1.top=pm->my+pm->oldy-rect.top;
							rect1.bottom=pm->my+pm->oldy-rect.bottom;
						}
					}
					if(IntersectRect(&dstrect,&rect1,&rect2)) {MessageBeep(MB_OK); break;}
				}
			} while(pmlist->next());
			pmlist->setwin(old);
			if(pm->mx+pm->oldx+rect.left<=0) pm->oldx=1-pm->mx-rect.left;
			if(pm->mx+pm->oldx+rect.right>=pm->pagewidth-1)
				pm->oldx=pm->pagewidth-pm->mx-rect.right-2;
			if(pm->my+pm->oldy-rect.top<=0) pm->oldy=1-pm->my+rect.top;
			if(pm->my+pm->oldy-rect.bottom>=pm->pageheight-1)
				pm->oldy=pm->pageheight-pm->my+rect.bottom-2;
			PatBlt(hDC,(pm->mx+pm->oldx+rect.left)/pm->zoom,
			       (pm->my+pm->oldy-rect.top)/pm->zoom,
			       (rect.right-rect.left+1)/pm->zoom+1,
			       (rect.top-rect.bottom+1)/pm->zoom+1,
			       DSTINVERT);
			ReleaseDC(hWnd,hDC);
		}
	} break;
	case WM_LBUTTONUP: {
		POINT p;
		RECT rect;
		if(pmlist==NULL) return 0;
		if(pm->capt) {
			ReleaseCapture();
			pm->capt=FALSE;
			PMListNode* n=(PMListNode*)pmlist->getwin();
			n->getrect(&rect); n->getoffset(&p);
			hDC=GetDC(hWnd);
			PatBlt(hDC,(pm->mx+pm->oldx+rect.left)/pm->zoom,
			       (pm->my+pm->oldy-rect.top)/pm->zoom,
			       (rect.right-rect.left+1)/pm->zoom+1,
			       (rect.top-rect.bottom+1)/pm->zoom+1,
			       DSTINVERT);
			if(pm->moved==1) {
				pm->moved=0; pm->Changed();
				n->setoffset(pm->mx+pm->oldx,pm->my+pm->oldy);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			ReleaseDC(hWnd,hDC);
		}
	} break;
	case WM_LBUTTONDOWN: {
		POINT mp={(int)((short)LOWORD(lParam))*pm->zoom,(int)((short)HIWORD(lParam))*pm->zoom};
		POINT p;
		RECT rect,srect;
		if(pmlist==NULL) return 0;
		PMListNode* old=(PMListNode*)pmlist->getwin();
		PMListNode* n;
		if(!pmlist->first()) return 0;
		do {
			n=(PMListNode*)pmlist->getwin();
			n->getrect(&rect); n->getoffset(&p);
			srect.left=(p.x+rect.left); srect.right=(p.x+rect.right);
			srect.top=(p.y-rect.top); srect.bottom=(p.y-rect.bottom);
			if(mp.x>=srect.left&&mp.y>=srect.top&&mp.x<=srect.right&&mp.y<=srect.bottom) {
				hDC=GetDC(hWnd);
				if(n!=old) {
					if(PreviewBmp) DeleteObject(PreviewBmp);
					PreviewBmp=ReadStampNamePreview(n->getfname(),NULL);
					SendDlgItemMessage(pm->getwnd(),IDC_STAMPPREVIEW,WM_USER+1,0,(LPARAM)PreviewBmp);
					SendDlgItemMessage(pm->getwnd(),IDC_STAMPPREVIEW,WM_USER,0,0);
					SendDlgItemMessage(pm->getwnd(),IDC_FILES,LB_SETCURSEL,n->getid(),0);
					SetDlgItemText(pm->getwnd(),IDC_STAMPNAME,n->getfname());
					pm->drawpage(hDC);
				}
				pm->capt=TRUE; SetCapture(hWnd); pm->moved=0;
				pm->mx=p.x-mp.x;
				pm->oldx=mp.x; pm->oldy=mp.y;
				pm->my=p.y-mp.y;
				PatBlt(hDC,(pm->mx+pm->oldx+rect.left)/pm->zoom,
				       (pm->my+pm->oldy-rect.top)/pm->zoom,
				       (rect.right-rect.left+1)/pm->zoom+1,
				       (rect.top-rect.bottom+1)/pm->zoom+1,
				       DSTINVERT);
				ReleaseDC(hWnd,hDC);
				return 0;
			}
		} while(pmlist->next());
		pmlist->setwin(old);
	} break;
	case WM_PAINT:
		hDC=BeginPaint(hWnd,&ps);
		if(pm) pm->drawpage(hDC);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
void PrintManager::drawpage(HDC hDC)
{
	RECT rect;
	HBRUSH oldbr;
	GetClientRect(GetDlgItem(hWnd,IDC_PMPREVIEW),&rect);
	zoom=(int)((float)pagewidth/(rect.right-7)+.5);
	if(pagewidth<pageheight) {
		zoom=(int)((float)pageheight/(rect.bottom-7)+.5);
	}
	pwidth=pagewidth/zoom; pheight=pageheight/zoom;
	Rectangle(hDC,0,0,pwidth,pheight);
	oldbr=SelectObject(hDC,GetStockObject(LTGRAY_BRUSH));
	if(pwidth<rect.right)
		PatBlt(hDC,pwidth,0,rect.right-pwidth+1,rect.bottom,PATCOPY);
	if(pheight<rect.bottom)
		PatBlt(hDC,0,pheight,rect.right,rect.bottom-pheight+1,PATCOPY);
	SelectObject(hDC,GetStockObject(DKGRAY_BRUSH));
	PatBlt(hDC,pwidth,5,5,pheight,PATCOPY);
	PatBlt(hDC,5,pheight,pwidth,5,PATCOPY);
	SelectObject(hDC,oldbr);
	PMListNode*old=(PMListNode*)pmlist->getwin();
	PMListNode*n;
	POINT p;
	if(!old) return;
	pmlist->first();
	do {
		n=(PMListNode*)pmlist->getwin();
		n->getrect(&rect); n->getoffset(&p);
		if(n==old)
			oldbr=SelectObject(hDC,GetStockObject(LTGRAY_BRUSH));
		Rectangle(hDC,(float)(p.x+rect.left)/zoom,(float)(p.y-rect.top)/zoom,
		          (float)(p.x+rect.right)/zoom+1,(float)(p.y-rect.bottom)/zoom+1);
		if(n==old)
			SelectObject(hDC,oldbr);
	} while(pmlist->next());
	pmlist->setwin(old);
}

typedef struct
{
	HBITMAP hBmp;
	int offx,offy;
	int sizex,sizey;
	int mx,my;
	BOOL captured;
} DSPWDATA;

LRESULT CALLBACK PMDlgSmallPreviewWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HBITMAP hBmp,oldbmp;
	BITMAP bmp;
	HDC hMemDC,hDC;
	RECT rect;
	PAINTSTRUCT ps;
	DSPWDATA* dspwd;
	dspwd=(DSPWDATA*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_CREATE:
		dspwd=new DSPWDATA;
		dspwd->captured=FALSE;
		dspwd->offx=dspwd->offy=0;
		dspwd->sizex=dspwd->sizey=0;
		dspwd->mx=dspwd->my=0;
		dspwd->hBmp=NULL;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)dspwd);
		break;
	case WM_USER+1:
		dspwd->hBmp=(HBITMAP)lParam;
		GetObject(dspwd->hBmp,sizeof(BITMAP),&bmp);
		GetClientRect(hWnd,&rect);
		dspwd->offx=(bmp.bmWidth>>1)-(rect.right>>1);
		dspwd->offy=(bmp.bmHeight>>1)-(rect.bottom>>1);
		dspwd->sizex=bmp.bmWidth; dspwd->sizey=bmp.bmHeight;
		dspwd->captured=FALSE;
		break;
	case WM_MOUSEMOVE:
		if(dspwd->captured==FALSE) break;
		dspwd->offx=dspwd->mx-(int)LOWORD(lParam);
		dspwd->offy=dspwd->my-(int)HIWORD(lParam);
		GetClientRect(hWnd,&rect);
		if(dspwd->offx<0) dspwd->offx=0;
		if(dspwd->offy<0) dspwd->offy=0;
		if(dspwd->offx+rect.right>=dspwd->sizex)
			dspwd->offx=dspwd->sizex-rect.right-1;
		if(dspwd->offy+rect.bottom>=dspwd->sizey)
			dspwd->offy=dspwd->sizey-rect.bottom-1;
	case WM_USER:
		hBmp=dspwd->hBmp;
		hDC=GetDC(hWnd);
		GetClientRect(hWnd,&rect);
		if(hBmp!=NULL) {
			GetObject(hBmp,sizeof(BITMAP),&bmp);
			hMemDC=CreateCompatibleDC(hDC);
			oldbmp=SelectObject(hMemDC,hBmp);
			BitBlt(hDC,0,0,rect.right,rect.bottom,hMemDC,dspwd->offx,dspwd->offy,SRCCOPY);
			SelectObject(hMemDC,oldbmp);
			DeleteDC(hMemDC);
		}else{
			PatBlt(hDC,0,0,rect.right,rect.bottom,WHITENESS);
			MoveToEx(hDC,0,0,NULL); LineTo(hDC,rect.right,rect.bottom);
			MoveToEx(hDC,rect.right,0,NULL); LineTo(hDC,0,rect.bottom);
		}
		ReleaseDC(hWnd,hDC);
		break;
	case WM_LBUTTONDOWN:
		if(dspwd->hBmp!=NULL) {
			dspwd->captured=TRUE;
			SetCapture(hWnd);
			dspwd->mx=LOWORD(lParam)+dspwd->offx;
			dspwd->my=HIWORD(lParam)+dspwd->offy;
		}
		break;
	case WM_LBUTTONUP:
		if(dspwd->captured) {
			dspwd->captured=FALSE;
			ReleaseCapture();
		}
		break;
	case WM_PAINT:
		hBmp=dspwd->hBmp;
		BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rect);
		if(hBmp!=NULL) {
			GetObject(hBmp,sizeof(BITMAP),&bmp);
			hMemDC=CreateCompatibleDC(ps.hdc);
			oldbmp=SelectObject(hMemDC,hBmp);
			BitBlt(ps.hdc,0,0,rect.right,rect.bottom,hMemDC,dspwd->offx,dspwd->offy,SRCCOPY);
			SelectObject(hMemDC,oldbmp);
			DeleteDC(hMemDC);
		}else{
			PatBlt(ps.hdc,0,0,rect.right,rect.bottom,WHITENESS);
			MoveToEx(ps.hdc,0,0,NULL); LineTo(ps.hdc,rect.right,rect.bottom);
			MoveToEx(ps.hdc,rect.right,0,NULL); LineTo(ps.hdc,0,rect.bottom);
		}
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		delete dspwd;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}

BOOL PrintManager::clear(void)
{
	if(_changed) {
		switch(MessageBox(hWnd,"Save changes now?","Project has been changed",MB_YESNOCANCEL|MB_ICONQUESTION))
		{
		case IDOK:
			if(save()==FALSE) return FALSE;
			break;
		case IDNO:
			break;
		case IDCANCEL:
			return FALSE;
		}
		_changed=FALSE;
	}
	delete pmlist;
	SendMessage(GetDlgItem(hWnd,IDC_FILES),LB_RESETCONTENT,0,0);
	pmlist=new PMList(GetDlgItem(hWnd,IDC_FILES));
	SetWindowText(hWnd,"Print manager 1.01 - unnamed project");
	return TRUE;
}
BOOL PrintManager::open(void)
{
	char szTemp[] = "Print manager project files (*.spj)\0*.spj\0All Files (*.*)\0*.*\0";
	char szName[128];
	OPENFILENAME ofn;
	OFNCUSTDATA myCustData;
	if(_changed) {
		switch(MessageBox(hWnd,"Save changes now?","Project has been changed",MB_YESNOCANCEL|MB_ICONQUESTION))
		{
		case IDOK:
			if(save()==FALSE) return FALSE;
			break;
		case IDNO:
			break;
		case IDCANCEL: return FALSE;
		}
		_changed=FALSE;
	}
	strcpy(szName,"*.spj");
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = WndMain; // An invalid hWnd causes non-modality
	ofn.hInstance = hInst;
	ofn.lpstrFilter = (LPSTR)szTemp;  // See previous note concerning string
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = (LPSTR)szName;  // Stores the result in this variable
	ofn.nMaxFile = sizeof( szName );
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Open project";  // Title for dialog
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	myCustData.preview=FALSE;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if(GetOpenFileName( &ofn ) != TRUE)
	{
		if(CommDlgExtendedError()!=0) // 0 value means user selected Cancel
			MessageBox(WndMain,"Cannot get name of file","WARNING",MB_OK|MB_ICONSTOP);
		else return FALSE;
		return FALSE;
	}
	if(!clear()) return FALSE;
	if(fname) delete fname;
	fname=new char[lstrlen(szName)+1];
	strcpy(fname,szName);
	wsprintf(szName,"Print manager 1.01 - %s",fname);
	SetWindowText(hWnd,szName);
	return openfile(fname);
}
BOOL PrintManager::save(void)
{
	if(fname==NULL) return saveas();
	savefile(fname);
	_changed=FALSE;
	return TRUE;
}
BOOL PrintManager::saveas(void)
{
	char szTemp[] = "Print manager project files (*.spj)\0*.spj\0All Files (*.*)\0*.*\0";
	char szName[128];
	OPENFILENAME ofn;
	if(fname) strcpy(szName,fname); else strcpy(szName,"");
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = hWnd; // An invalid hWnd causes non-modality
	ofn.hInstance = hInst;
	ofn.lpstrFilter = (LPSTR)szTemp;  // See previous note concerning string
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = (LPSTR)szName;  // Stores the result in this variable
	ofn.nMaxFile = sizeof( szName );
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Save as...";  // Title for dialog
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = "spj";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if(GetSaveFileName( &ofn ) != TRUE)
	{
		if(CommDlgExtendedError()!=0) // 0 value means user selected Cancel
			MessageBox(WndMain,"Cannot get name of file","WARNING",MB_OK|MB_ICONSTOP);
		else return FALSE;
		return FALSE;
	}
	if(fname) delete fname;
	fname=new char[lstrlen(szName)+1];
	strcpy(fname,szName);
	wsprintf(szName,"Print manager 1.01 - %s",fname);
	SetWindowText(hWnd,szName);
	return save();
}
typedef struct
{
	DWORD id;
	WORD version;
	int numobj;
	char device;
} PMANHEADER;
BOOL PrintManager::savefile(char*fn)
{
	HANDLE hFile;
	PMListNode *n;
	DWORD dwWritten;
	LPDEVMODE pDevMode;
	WORD len;
	RECT rect; POINT p;
	int numobj=0;
	PMANHEADER header;
	hFile=CreateFile(fn,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,
	                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile==INVALID_HANDLE_VALUE) return FALSE; // возвращаем ошибку
	if(!pmlist->first())
		numobj=0;
	else{
		do {numobj++;} while(pmlist->next());
		pmlist->first();
	}
	header.id=0x4A505453; // STPJ
	header.version=0x101;
	header.numobj=numobj;
	if(hDevMode!=0) header.device=1; else header.device=0;
	WriteFile(hFile,&header,sizeof(header),&dwWritten,NULL);
	if(hDevMode!=0) {
		pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
		WriteFile(hFile,pDevMode,sizeof(DEVMODE),&dwWritten,NULL);
		GlobalUnlock(hDevMode);
	}
	if(numobj>0)
		do {
			n=(PMListNode*)pmlist->getwin();
			len=(WORD)(lstrlen(n->getfname())+1);
			WriteFile(hFile,&len,sizeof(WORD),&dwWritten,NULL);
			WriteFile(hFile,n->getfname(),len,&dwWritten,NULL);
			len=(WORD)(lstrlen(n->getname())+1);
			WriteFile(hFile,&len,sizeof(WORD),&dwWritten,NULL);
			WriteFile(hFile,n->getname(),len,&dwWritten,NULL);
			n->getrect(&rect);
			WriteFile(hFile,&rect,sizeof(RECT),&dwWritten,NULL);
			n->getoffset(&p);
			WriteFile(hFile,&p,sizeof(POINT),&dwWritten,NULL);
		} while(pmlist->next());
	CloseHandle(hFile);
	_changed=FALSE;
	return TRUE;
}
BOOL PrintManager::openfile(char*fn)
{
	char* fname,* name;
	RECT rect; POINT p;
	HANDLE hFile;
	HDC hDC;
	LPDEVMODE pDevMode;
	LPDEVNAMES pDevNames;
	WORD len;
	DWORD dwWritten;
	PMANHEADER header;
	hFile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
	                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return FALSE; // возвращаем ошибку
	}
	ReadFile(hFile,&header,sizeof(header),&dwWritten,NULL); // Read file header
	if(header.id!=0x4A505453) {
		CloseHandle(hFile);
		return FALSE; // Not a stamp file
	}
	if(header.version<0x101) {
		CloseHandle(hFile);
		return FALSE;
	}
	if(header.device==1) {
		if(hDevMode!=NULL) GlobalFree(hDevMode);
		hDevMode=GlobalAlloc(GHND,sizeof(DEVMODE));
		pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
		ReadFile(hFile,pDevMode,sizeof(DEVMODE),&dwWritten,NULL);
		GlobalUnlock(hDevMode);
	}
	pmlist->clear();
	for(int i=0; i<header.numobj; i++) {
		ReadFile(hFile,&len,sizeof(WORD),&dwWritten,NULL);
		fname=new char[len];
		ReadFile(hFile,fname,len,&dwWritten,NULL);
		ReadFile(hFile,&len,sizeof(WORD),&dwWritten,NULL);
		name=new char[len];
		ReadFile(hFile,name,len,&dwWritten,NULL);
		ReadFile(hFile,&rect,sizeof(RECT),&dwWritten,NULL);
		ReadFile(hFile,&p,sizeof(POINT),&dwWritten,NULL);
		pmlist->insert(new PMListNode(fname,name,&rect,&p));
		delete fname; delete name;
	}
	CloseHandle(hFile);
	_changed=FALSE;
	if(hDevMode!=NULL) {
		pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
		pDevNames=(LPDEVNAMES)GlobalLock(hDevNames);
		hDC=CreateDC((LPCTSTR)(pDevNames+pDevNames->wDriverOffset),
		             (LPCTSTR)(pDevMode->dmDeviceName),NULL,pDevMode);
		if(hDC==NULL) {
			MessageBox(WndMain,(LPCTSTR)(pDevMode->dmDeviceName),"Cannot find printer",MB_OK|MB_ICONINFORMATION);
			GlobalUnlock(hDevMode);
			GlobalFree(hDevMode); hDevMode=NULL;
			if(hDevNames) {
				GlobalUnlock(hDevNames);
				GlobalFree(hDevNames);
			}
			getdefaultprinter();
		}else{
			pagewidth=GetDeviceCaps(hDC,HORZSIZE)*10;
			pageheight=GetDeviceCaps(hDC,VERTSIZE)*10;
			DeleteDC(hDC);
			GlobalUnlock(hDevMode);
			GlobalUnlock(hDevNames);
		}
	}
	name=new char[64];
	pm->getprintername(name);
	SetDlgItemText(pm->getwnd(),IDC_PRINTERNAME,name);
	delete name;
	return TRUE;
}
void PrintManager::remove(void)
{
	int p=SendDlgItemMessage(hWnd,IDC_FILES,LB_GETCURSEL,0,0);
	if(!pmlist->first()) return;
	ListNode*old=pmlist->getwin();
	do {
		if(((PMListNode*)pmlist->getwin())->getid()==p) {
			if(MessageBox(hWnd,((PMListNode*)pmlist->getwin())->getname(),"Remove item",MB_YESNO)==IDYES) {
				pmlist->remove();
				_changed=TRUE;
			}
			RedrawWindow(GetDlgItem(hWnd,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
			return;
		}
	} while(pmlist->next());
	pmlist->setwin(old);
}
StampNode* FAR GRectCreator(void)
{
	return new GeomRect;
}
StampNode* FAR GLineCreator(void)
{
	return new GeomLine;
}
StampNode* FAR GCircleCreator(void)
{
	return new GeomCircle;
}
StampNode* FAR PBitmapCreator(void)
{
	return new PicBitmap;
}
StampNode* FAR GTriangleCreator(void)
{
	return new GeomTriangle;
}
void PrintManager::append(void)
{
	Stamp* stmp=new Stamp("unnamed");
	RECT rect;
	POINT pt={pagewidth>>1,pageheight>>1};
	stmp->RegisterClass(-1,5,"Rectangles",NULL,0,0,GRectCreator);
	stmp->RegisterClass(-1,3,"Lines",NULL,0,0,GLineCreator);
	stmp->RegisterClass(-1,4,"Circles",NULL,0,0,GCircleCreator);
	stmp->RegisterClass(-1,7,"Pictures(BITMAP)",NULL,0,0,PBitmapCreator);
	stmp->RegisterClass(-1,6,"Triangles",NULL,0,0,GTriangleCreator);
	if(stmp->open()==0) {
		stmp->calcrect(&rect);
		rect.top+=15; rect.left-=15; rect.right+=15; rect.bottom-=15;
		pmlist->insert(new PMListNode(stmp->getfname(),stmp->getname(),&rect,&pt));
		RedrawWindow(GetDlgItem(hWnd,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
	}
	delete stmp;
}

// Structure used for all OpenFileName notifications
typedef struct tagNMHDR
{
	HWND hwndFrom;
	UINT idFrom;
	UINT code;            // NM_ code
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;
typedef struct _OFNOTIFY
{
	NMHDR hdr;
	LPOPENFILENAME lpOFN;
	LPSTR pszFile;                        // May be NULL
} OFNOTIFY, FAR *LPOFNOTIFY;
BOOL NEAR PASCAL TestNotify(HWND hDlg, LPOFNOTIFY pofn)
{
	LPOPENFILENAME ofn;
	POFNCUSTDATA ofnCustData;
	switch (pofn->hdr.code) {
	case CDN_SELCHANGE: {
		char szFile[MAX_PATH];
		char name[250];
		if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
		                                 szFile, sizeof(szFile)) <= sizeof(szFile)) {
			ofn=(LPOPENFILENAME)GetWindowLong(hDlg,DWL_USER);
			ofnCustData=(POFNCUSTDATA)ofn->lCustData;
			if(ofnCustData->__hBitmapPreview)
				DeleteObject(ofnCustData->__hBitmapPreview);
			ofnCustData->__hBitmapPreview=ReadStampNamePreview(szFile,name);
			SetDlgItemText(hDlg, IDC_STAMPNAME,name);
			if(ofnCustData->__hBitmapPreview) {
				SendDlgItemMessage(hDlg,IDC_PREVIEW,WM_USER+1,0,
				                   (LPARAM)ofnCustData->__hBitmapPreview);
				SendDlgItemMessage(hDlg, IDC_PREVIEW,WM_USER,0,0);
			}
		}
	}
		                  break;
	}
	return(TRUE);
}
BOOL CALLBACK OpenFileNameHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POFNCUSTDATA ofnCustData;
	LPOPENFILENAME ofn;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		ofn=(LPOPENFILENAME)lParam;
		ofnCustData=(POFNCUSTDATA)ofn->lCustData;
		SetWindowLong(hDlg,DWL_USER,lParam);
		if(ofnCustData->preview==FALSE)
			ShowWindow(GetDlgItem(hDlg,IDC_SAVEPREVIEW),SW_HIDE);
		break;
	case WM_NOTIFY:
		TestNotify(hDlg,(LPOFNOTIFY)lParam);
		return FALSE;
	case WM_DESTROY:
		ofn=(LPOPENFILENAME)GetWindowLong(hDlg,DWL_USER);
		ofnCustData=(POFNCUSTDATA)ofn->lCustData;
		if(ofnCustData->preview) {
			if(IsDlgButtonChecked(hDlg,IDC_SAVEPREVIEW))
				ofnCustData->previewstat=TRUE;
			else
				ofnCustData->previewstat=FALSE;
		}
		if(ofnCustData->__hBitmapPreview)
			DeleteObject(ofnCustData->__hBitmapPreview);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
VOID CALLBACK DottedLineProc(int x,int y,LPARAM lpData)
{
	LPDTLPDATA dtldata=(LPDTLPDATA)lpData;
	dtldata->c++; dtldata->c%=2;
	if(dtldata->c)
		SetPixel(dtldata->hDC,x,y,0);
}

void PrintManager::listsel(int id)
{
	if(!pmlist->first()) return;
	ListNode* old=pmlist->getwin();
	do {
		if(((PMListNode*)pmlist->getwin())->getid()==id) {
			if(PreviewBmp) DeleteObject(PreviewBmp);
			PreviewBmp=ReadStampNamePreview(((PMListNode*)pmlist->getwin())->getfname(),NULL);
			SendDlgItemMessage(hWnd,IDC_STAMPPREVIEW,WM_USER+1,0,
			                   (LPARAM)PreviewBmp);
			SendDlgItemMessage(hWnd,IDC_STAMPPREVIEW,WM_USER,0,0);
			SetDlgItemText(hWnd,IDC_STAMPNAME,((PMListNode*)pmlist->getwin())->getfname());
			RedrawWindow(GetDlgItem(hWnd,IDC_PMPREVIEW),NULL,NULL,RDW_INVALIDATE);
			return;
		}
	} while(pmlist->next());
	pmlist->setwin(old);
}
BOOL CALLBACK printdlgproc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
void PrintManager::print(void)
{
	if(!pmlist->first()) return;
	RECT rect;
	POINT pt;
	LPDEVMODE pDevMode;
	LPDEVNAMES pDevNames;
	HDC hDC;
	DOCINFO di;
	PMListNode* old=(PMListNode*)pmlist->getwin();
	PMListNode*n;
	Stamp* stmp=new Stamp("new");

	stmp->RegisterClass(-1,5,"Rectangles",NULL,0,0,GRectCreator);
	stmp->RegisterClass(-1,3,"Lines",NULL,0,0,GLineCreator);
	stmp->RegisterClass(-1,4,"Circles",NULL,0,0,GCircleCreator);
	stmp->RegisterClass(-1,7,"Pictures(BITMAP)",NULL,0,0,PBitmapCreator);
	stmp->RegisterClass(-1,6,"Triangles",NULL,0,0,GTriangleCreator);

	pDevMode=(LPDEVMODE)GlobalLock(hDevMode);
	pDevNames=(LPDEVNAMES)GlobalLock(hDevNames);
	hDC=CreateDC((LPCTSTR)(pDevNames+pDevNames->wDriverOffset),
	             (LPCTSTR)(pDevMode->dmDeviceName),NULL,pDevMode);
	GlobalUnlock(hDevMode);
	GlobalUnlock(hDevNames);
	di.lpszDocName="Print manager project";
	di.cbSize=lstrlen(di.lpszDocName);
	di.lpszOutput=NULL;
	StartDoc(hDC,&di);
	StartPage(hDC);
	SetMapMode(hDC,MM_LOMETRIC);
	HWND hDlg;
	stopprint=FALSE;
	hDlg=CreateDialog(hInst,MAKEINTRESOURCE(IDD_PRINT),hWnd,(DLGPROC)printdlgproc);
	SetViewportOrgEx(hDC,0,0,NULL);
	stmp->close();
	do {
		n=(PMListNode*)pmlist->getwin();
		n->getrect(&rect);
		n->getoffset(&pt);
		if(stopprint==TRUE) {AbortDoc(hDC); break;}
		if(stmp->open_file(n->getfname())==0) {
			if(stopprint==TRUE) {AbortDoc(hDC); break;}
			SetDlgItemText(hDlg,IDC_PRINTFNAME,n->getfname());
			SetDlgItemText(hDlg,IDC_PRINTNAME,n->getname());
			RedrawWindow(GetDlgItem(hDlg,IDC_PRINTFNAME),NULL,NULL,RDW_INVALIDATE);
			RedrawWindow(GetDlgItem(hDlg,IDC_PRINTNAME),NULL,NULL,RDW_INVALIDATE);
			SetWindowOrgEx(hDC,-pt.x,pt.y,NULL);
			stmp->print_stamp(hDC);
			stmp->close();
		}
	} while(pmlist->next());
	pmlist->setwin(old);
	if(stopprint==FALSE) {
		EndPage(hDC);
		EndDoc(hDC);
	}
	DeleteDC(hDC);
	delete stmp;
	DestroyWindow(hDlg);
}
BOOL CALLBACK printdlgproc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG:
		stopprint=FALSE;
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDCANCEL) {
			stopprint=TRUE;
			SetWindowText(hWnd,"Aborting...");
		}
		break;
	case WM_CLOSE:
		stopprint=TRUE;
		SetWindowText(hWnd,"Aborting...");
		break;
	default: return FALSE;
	}
	return TRUE;
}
