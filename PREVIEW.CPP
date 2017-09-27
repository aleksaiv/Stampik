//File:preview.cpp//

#include <windows.h>
#include <stdio.h>
#include "list.hpp"
#include "stamp.hpp"
#include "preview.hpp"

#include "stmp.rh"

BOOL CALLBACK PreviewDlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

static BOOL drawing=FALSE;
void Preview::update(void){
	redraw();
	HDC hDC=GetDC(hPreviewWnd);
	draw(hDC);
	ReleaseDC(hPreviewWnd,hDC);
}
Preview::Preview(HINSTANCE hI,HWND hMWnd,Stamp* s)
{
	RECT rect;
	stmp=s;
	hMainWnd=hMWnd;
	hInst=hI;
	sel=NULL;
	hWnd=CreateDialogParam(hInst,
	                       MAKEINTRESOURCE(IDD_PREVIEW),
	                       hMainWnd,
	                       (DLGPROC)PreviewDlgProc,(LPARAM)this);
	if(!hWnd) {
		MessageBox(hMainWnd,"Cannot create main preview window",
		           "Error in preview.cpp",MB_OK|MB_ICONSTOP);
		return;
	}
	hPreviewWnd=GetDlgItem(hWnd,IDC_VIEW);
	if(!hPreviewWnd) {
		MessageBox(hMainWnd,"Cannot create preview",
		           "Error in preview.cpp",MB_OK|MB_ICONSTOP);
		return;
	}
	GetWindowRect(hPreviewWnd,&rect);
	VIEW_X=rect.left; VIEW_Y=rect.top;
	GetClientRect(hPreviewWnd,&rect);
	VIEW_WIDTH=rect.right; VIEW_HEIGHT=rect.bottom;
	offx=(STAMP_WIDTH>>1)-(VIEW_WIDTH>>1); offy=(STAMP_HEIGHT>>1)-(VIEW_HEIGHT>>1);
	posx=posy=0;
	HRul=GetDlgItem(hWnd,IDC_HRULER);
	VRul=GetDlgItem(hWnd,IDC_VRULER);
	SendMessage(HRul,WM_USER,1,0l);
	SendMessage(VRul,WM_USER,0,0l);
	SendMessage(HRul,WM_USER+1,offx,0);
	SendMessage(VRul,WM_USER+1,offy-1,0);
//  HDC hDC=CreateCompatibleDC(NULL);
	HDC hDC=GetDC(hPreviewWnd);
	hSurface=CreateCompatibleBitmap(hDC,STAMP_WIDTH,STAMP_HEIGHT);
	HBITMAP oldbmp=SelectObject(hDC,hSurface);
	PatBlt(hDC,0,0,STAMP_WIDTH,STAMP_HEIGHT,WHITENESS);
	SelectObject(hDC,oldbmp);
	ReleaseDC(hPreviewWnd,hDC);
//  DeleteDC(hDC);
}
Preview::~Preview()
{
	DeleteObject(hSurface);
	DestroyWindow(hWnd);
}
void Preview::hidecontour(void)
{
	HDC hDC=GetDC(hPreviewWnd); preparedc(hDC);
	drawcontour(hDC);
	ReleaseDC(hPreviewWnd,hDC);
	cx=cy=0;
}
void Preview::showcontour(void)
{
	HDC hDC=GetDC(hPreviewWnd); preparedc(hDC);
	drawcontour(hDC);
	ReleaseDC(hPreviewWnd,hDC);
	cx=cy=0;
}
void Preview::reldrawcontour(int x,int y)
{
	HDC hDC=GetDC(hPreviewWnd); preparedc(hDC);
	if(sel!=NULL) drawcontour(hDC);
	cx=x; cy=y;
	drawcontour(hDC);
	ReleaseDC(hPreviewWnd,hDC);
}
void Preview::drawcontour(HDC hDC)
{
	if(sel!=NULL) {
		int oldr2=SetROP2(hDC,R2_XORPEN);
		int oldmode=SetBkMode(hDC,TRANSPARENT);
		SetViewportOrgEx(hDC,(STAMP_WIDTH>>1)-offx+cx,(STAMP_HEIGHT>>1)-offy-cy,NULL);
		sel->drawcontour(hDC);
		SetROP2(hDC,oldr2);
		SetBkMode(hDC,oldmode);
	}
}
VOID CALLBACK DottedLineProc(int x,int y,LPARAM lpData);
void Preview::movecross(int x,int y)
{
	HDC hDC=GetDC(hPreviewWnd); preparedc(hDC);
	drawcross(hDC);
	posx=x; posy=y;
	drawcross(hDC);
	ReleaseDC(hPreviewWnd,hDC);
}
void Preview::drawcross(HDC hDC)
{
	HPEN hPen,oldpen;
	hPen=CreatePen(PS_DOT,1,RGB(0,255,255));
	int oldr2=SetROP2(hDC,R2_XORPEN);
	int oldmode=SetBkMode(hDC,TRANSPARENT);
	oldpen=SelectObject(hDC,hPen);
	SetViewportOrgEx(hDC,(STAMP_WIDTH>>1)-offx,(STAMP_HEIGHT>>1)-offy,NULL);
	MoveToEx(hDC,posx,(STAMP_HEIGHT>>1),NULL);
	LineTo(hDC,posx,-(STAMP_HEIGHT>>1));
	MoveToEx(hDC,-(STAMP_WIDTH>>1),posy,NULL);
	LineTo(hDC,(STAMP_WIDTH>>1),posy);
	SelectObject(hDC,oldpen); DeleteObject(hPen);
	SetROP2(hDC,oldr2);
	SetBkMode(hDC,oldmode);
}
void Preview::draw(HDC hDC) // Отображает изображение из памяти на экране
{
	HDC hMemDC;
	HBITMAP oldbmp;
	SetMapMode(hDC,MM_TEXT);
	SetWindowOrgEx(hDC,0,0,NULL);
	SetViewportOrgEx(hDC,0,0,NULL);
	if(drawing) {
		PatBlt(hDC,0,0,VIEW_WIDTH,VIEW_HEIGHT,WHITENESS);
		TextOut(hDC,0,0,"Drawing. please wait...",23);
	}else{
		hMemDC=CreateCompatibleDC(hDC);
		oldbmp=SelectObject(hMemDC,hSurface);
		BitBlt(hDC,0,0,VIEW_WIDTH,VIEW_HEIGHT,hMemDC,offx,offy,SRCCOPY);
		SelectObject(hMemDC,oldbmp);
		DeleteDC(hMemDC);
	}
	preparedc(hDC);
	drawcontour(hDC);
	drawcross(hDC);
}
void Preview::preparedc(HDC hDC)
{
	SetMapMode(hDC,MM_ISOTROPIC);
	SetWindowExtEx(hDC,STAMP_WIDTH,-STAMP_HEIGHT,NULL);
	SetViewportExtEx(hDC,STAMP_WIDTH,STAMP_HEIGHT,NULL);
	SetWindowOrgEx(hDC,0,0,NULL);
	SetViewportOrgEx(hDC,STAMP_WIDTH>>1,STAMP_HEIGHT>>1,NULL);
}
void Preview::redraw(void)  // Рисует штамп в памяти
{
	HBITMAP oldbmp;
	HDC hDC;
	HBRUSH oldbr,br=CreateSolidBrush(RGB(196,0,0));
	hDC=GetDC(hWnd);
	oldbr=SelectObject(hDC,br);
	Ellipse(hDC,0,0,15,15);
	SelectObject(hDC,oldbr);
	ReleaseDC(hWnd,hDC);
	DeleteObject(br);
	drawing=TRUE;
	hDC=CreateCompatibleDC(NULL);
	preparedc(hDC);
	oldbmp=SelectObject(hDC,hSurface);
	stmp->draw(hDC);
	SelectObject(hDC,oldbmp);
	DeleteDC(hDC);
	drawing=FALSE;
	hDC=GetDC(hWnd);
	oldbr=SelectObject(hDC,GetStockObject(LTGRAY_BRUSH));
	PatBlt(hDC,0,0,16,16,PATCOPY);
	SelectObject(hDC,oldbr);
	ReleaseDC(hWnd,hDC);
}
BOOL Preview::hittest(int x,int y)
{
	if(!stmp->first()) return FALSE;
	StampNode* old=sel,*n;
	BOOL ret=FALSE;
	do {
		n=(StampNode*)stmp->getwin();
		if((n)->hittest(offx+x-(STAMP_WIDTH>>1),-offy-y+(STAMP_HEIGHT>>1))) {
			if(old==n) {sel=old; ret=TRUE;}
			else{
				if(SendMessage(hMainWnd,WM_PREVIEWSELECTED,0,(LPARAM)n)!=0)
					sel=old;
				else
					sel=n;
			}
			break;
		}
	} while(stmp->next());
	if(sel!=old) {
		HDC hDC=GetDC(hPreviewWnd); preparedc(hDC);
		int oldr2=SetROP2(hDC,R2_XORPEN);
		int oldmode=SetBkMode(hDC,TRANSPARENT);
		if(old!=NULL) {
			SetViewportOrgEx(hDC,(STAMP_WIDTH>>1)-offx+cx,(STAMP_HEIGHT>>1)-offy-cy,NULL);
			old->drawcontour(hDC);
		}
		cx=cy=0; drawcontour(hDC);
		SetBkMode(hDC,oldmode);
		SetROP2(hDC,oldr2);
		ReleaseDC(hPreviewWnd,hDC);
	}
	return ret;
}

void Preview::SeekObject(StampNode*sn)
{
	HDC hDC=GetDC(hPreviewWnd); preparedc(hDC);
	drawcontour(hDC);
	cx=cy=0;
	sel=sn;
	drawcontour(hDC);
	ReleaseDC(hPreviewWnd,hDC);
}
typedef struct
{
	Preview* preview;
	int mx,my,mx2,my2;
	int capt;
	HCURSOR hCursor;
} PWPDATA, FAR* LPPWPDATA;
#pragma argsused
BOOL CALLBACK PreviewDlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	LPPWPDATA pwpdata;
	switch(uMsg) {
	case WM_INITDIALOG:
		pwpdata=new PWPDATA;
		pwpdata->preview=(Preview*)lParam;
		pwpdata->capt=0;
		pwpdata->mx=pwpdata->my=pwpdata->mx2=pwpdata->my2=0;
		SetWindowLong(hWnd,DWL_USER,(LONG)pwpdata);
		SetWindowLong(GetDlgItem(hWnd,IDC_VIEW),GWL_USERDATA,(LONG)pwpdata);
		break;
	case WM_DESTROY:
		pwpdata=(LPPWPDATA)GetWindowLong(hWnd,DWL_USER);
		delete pwpdata;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
inline int abs(int a){
	return a<0 ? -a : a;
}
LRESULT CALLBACK PreviewWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	int x,y;
	HDC hDC;
	LPPWPDATA pwpdata=(LPPWPDATA)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_RBUTTONDOWN:
		if(pwpdata->capt^1) {
			if(pwpdata->capt==0) {
				pwpdata->hCursor=SetCursor(LoadCursor(NULL,IDC_SIZE));
				SetCapture(hWnd);
			}
			pwpdata->preview->GetOffset(x,y);
			pwpdata->capt|=1;
			pwpdata->mx=x+LOWORD(lParam);
			pwpdata->my=y+HIWORD(lParam);
			SendMessage(pwpdata->preview->GetMainWnd(),WM_SETSTATUSTEXT,IDS_PREVIEWTEXT3,0);
		}
		break;
	case WM_LBUTTONDOWN:
		if(!(pwpdata->capt&2))
			if(!(wParam&MK_CONTROL)) {
				if(pwpdata->preview->hittest(LOWORD(lParam),HIWORD(lParam))==TRUE) {
					if(pwpdata->capt==0) {
						pwpdata->hCursor=SetCursor(LoadCursor(NULL,IDC_SIZE));
						SetCapture(hWnd);
					}
					pwpdata->capt|=2;
					pwpdata->preview->GetOffset(x,y);
					pwpdata->mx2=LOWORD(lParam)+x;
					pwpdata->my2=-HIWORD(lParam)-y;
					SendMessage(pwpdata->preview->GetMainWnd(),WM_SETSTATUSTEXT,IDS_PREVIEWTEXT2,0);
				}
			}else{
				pwpdata->preview->GetOffset(x,y);
				x=x-(STAMP_WIDTH>>1)+(short)LOWORD(lParam);
				y=-y+(STAMP_HEIGHT>>1)-(short)HIWORD(lParam);
				pwpdata->preview->movecross(x,y);
			}
		break;
	case WM_LBUTTONUP:
		if(pwpdata->capt&2) {
			pwpdata->capt^=2;
			if(pwpdata->capt==0) {
				ReleaseCapture();
				SetCursor(pwpdata->hCursor);
			}
			hDC=GetDC(hWnd); pwpdata->preview->preparedc(hDC);
			pwpdata->preview->drawcontour(hDC);
			ReleaseDC(hWnd,hDC);
			pwpdata->preview->moveobject();
			SendMessage(pwpdata->preview->GetMainWnd(),WM_SETSTATUSTEXT,IDS_PREVIEWTEXT1,0);
		}
		break;
	case WM_RBUTTONUP:
		if(pwpdata->capt&1) {
			pwpdata->capt^=1;
			if(pwpdata->capt==0) {
				ReleaseCapture();
				SetCursor(pwpdata->hCursor);
			}
			SendMessage(pwpdata->preview->GetMainWnd(),WM_SETSTATUSTEXT,IDS_PREVIEWTEXT1,0);
		}
		break;
	case WM_MOUSEMOVE:
		if(pwpdata->capt&1) {
			x=pwpdata->mx-(signed short)LOWORD(lParam);
			y=pwpdata->my-(signed short)HIWORD(lParam);
			if(x+pwpdata->preview->GetViewWidth()>=STAMP_WIDTH)
				x=STAMP_WIDTH-pwpdata->preview->GetViewWidth()-1;
			if(y+pwpdata->preview->GetViewHeight()>=STAMP_HEIGHT)
				y=STAMP_HEIGHT-pwpdata->preview->GetViewHeight()-1;
			if(x<0) x=0; if(y<0) y=0;
			pwpdata->preview->SetOffset(x,y);
			hDC=GetDC(hWnd); //pwpdata->preview->preparedc(hDC);
			pwpdata->preview->draw(hDC);
			ReleaseDC(hWnd,hDC);
			SendMessage(pwpdata->preview->GetHRuler(),WM_USER+1,x,0);
			SendMessage(pwpdata->preview->GetVRuler(),WM_USER+1,y-1,0);
		}
		if(pwpdata->capt&2) {
			pwpdata->preview->GetOffset(x,y);
			x=(signed short)LOWORD(lParam)-(pwpdata->mx2-x);
			y=-(signed short)HIWORD(lParam)-(pwpdata->my2+y);
			if(GetKeyState(VK_SHIFT)&0x8000)
				if(abs(x)>abs(y)) y=0; else x=0;
			hDC=GetDC(hWnd); pwpdata->preview->preparedc(hDC);
			pwpdata->preview->drawcontour(hDC);
			pwpdata->preview->movecontour(x,y);
			pwpdata->preview->drawcontour(hDC);
			ReleaseDC(hWnd,hDC);
			pwpdata->preview->dragcontour();
			pwpdata->preview->GetOffset(x,y);
		}else{
			pwpdata->preview->GetOffset(x,y);
		}
		SendMessage(pwpdata->preview->GetHRuler(),WM_USER+2,(signed short)LOWORD(lParam),0);
		SendMessage(pwpdata->preview->GetVRuler(),WM_USER+2,(signed short)HIWORD(lParam)+1,0);
		pwpdata->preview->SetMouseCoord((signed short)LOWORD(lParam)+x-(STAMP_WIDTH>>1),
		                                -(signed short)HIWORD(lParam)-y+(STAMP_HEIGHT>>1));
		break;
	case WM_SETCURSOR:
		SendMessage(pwpdata->preview->GetMainWnd(),WM_SETSTATUSTEXT,IDS_PREVIEWTEXT1,0);
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		pwpdata->preview->draw(ps.hdc);
		EndPaint(hWnd,&ps);
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0l;
}

typedef struct
{
	int type;
	int s;
	signed short p; // start and position
	HFONT hFont;
	HBITMAP hBmp;
} RWPDATA, FAR* LPRWPDATA;
LRESULT CALLBACK RulerWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HBITMAP oldbmp;
	HFONT oldfnt;
	int x,r;
	RECT rect;
	char txt[16]; int ltxt;
	LPRWPDATA rwpdata=(LPRWPDATA)GetWindowLong(hWnd,GWL_USERDATA);

	switch(uMsg) {
	case WM_CREATE:
		rwpdata=new RWPDATA;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)rwpdata);
		rwpdata->hFont=CreateFont(6,0,0,0,0,
		                          FALSE,FALSE,FALSE,
		                          ANSI_CHARSET,
		                          OUT_DEFAULT_PRECIS,
		                          CLIP_DEFAULT_PRECIS,
		                          DEFAULT_QUALITY,
		                          DEFAULT_PITCH|FF_DONTCARE,
		                          "MS Sans Serif");
		break;
	case WM_USER:  // set type
		rwpdata->type=wParam;
		rwpdata->s=rwpdata->p=0;
		hDC=CreateCompatibleDC(NULL);
		GetClientRect(hWnd,&rect);
		if(rwpdata->type) {//Horiz
			rwpdata->hBmp=CreateCompatibleBitmap(hDC,STAMP_WIDTH,rect.bottom);
			oldbmp=SelectObject(hDC,rwpdata->hBmp);
			PatBlt(hDC,0,0,STAMP_WIDTH,rect.bottom,WHITENESS);
		}else{
			rwpdata->hBmp=CreateCompatibleBitmap(hDC,rect.right,STAMP_HEIGHT);
			oldbmp=SelectObject(hDC,rwpdata->hBmp);
			PatBlt(hDC,0,0,rect.right,STAMP_HEIGHT,WHITENESS);
		}
		x=0;
		oldfnt=SelectObject(hDC,rwpdata->hFont);
		SetROP2(hDC,R2_COPYPEN); SetBkMode(hDC,TRANSPARENT);
		if(rwpdata->type==0) {  //Vertical
			SetTextAlign(hDC,TA_LEFT|TA_BASELINE|TA_NOUPDATECP);
			r=(STAMP_HEIGHT>>1);
			MoveToEx(hDC,rect.right,0,NULL);
			LineTo(hDC,rect.right,STAMP_HEIGHT);
			while(1) {
				if(r%100==0) {
					MoveToEx(hDC,rect.right-12,x,NULL);
					ltxt=sprintf(txt,"%d",r/100);
					TextOut(hDC,0,x,txt,ltxt);
				}
				else if(r%50==0) {
					MoveToEx(hDC,rect.right-8,x,NULL);
				}
				else if(r%10==0) MoveToEx(hDC,rect.right-4,x,NULL);
				else MoveToEx(hDC,rect.right-1,x,NULL);
				LineTo(hDC,rect.right,x);
				x+=5; r-=5;
				if(x>STAMP_HEIGHT) break;
			}
		}else{  // Horizontal
			SetTextAlign(hDC,TA_CENTER|TA_BOTTOM|TA_NOUPDATECP);
			r=-(STAMP_WIDTH>>1);
			MoveToEx(hDC,0,rect.bottom,NULL);
			LineTo(hDC,STAMP_WIDTH,rect.bottom);
			while(1) {
				if(r%100==0) {
					MoveToEx(hDC,x,rect.bottom-12,NULL);
					ltxt=sprintf(txt,"%d",r/100);
					TextOut(hDC,x,rect.bottom-10,txt,ltxt);
				}else
				if(r%50==0) {
					MoveToEx(hDC,x,rect.bottom-8,NULL);
				}else
				if(r%10==0)
					MoveToEx(hDC,x,rect.bottom-4,NULL);
				else
					MoveToEx(hDC,x,rect.bottom-1,NULL);
				LineTo(hDC,x,rect.bottom);
				x+=5; r+=5;
				if(x>STAMP_WIDTH) break;
			}
		}
		SelectObject(hDC,oldfnt);
		SelectObject(hDC,oldbmp);
		DeleteDC(hDC);
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_USER+1:  // set start
		if(rwpdata->s!=wParam) {
			rwpdata->s=wParam;
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		break;
	case WM_USER+2:  // set position
		if(rwpdata->p!=(signed short)wParam) {
			hDC=GetDC(hWnd); //rwpdata->preview->preparedc(hDC);
			SetROP2(hDC,R2_MERGEPENNOT);
			GetClientRect(hWnd,&rect);
			if(rwpdata->type==0) {
				MoveToEx(hDC,0,rwpdata->p,NULL);
				LineTo(hDC,rect.right,rwpdata->p);
			}else{
				MoveToEx(hDC,rwpdata->p,0,NULL);  LineTo(hDC,rwpdata->p,rect.bottom);
			}
			rwpdata->p=(signed short)wParam;
			if(rwpdata->type==0) {
				MoveToEx(hDC,0,rwpdata->p,NULL);  LineTo(hDC,rect.right,rwpdata->p);
				r=-rwpdata->p-rwpdata->s+(STAMP_HEIGHT>>1);
			}else{
				MoveToEx(hDC,rwpdata->p,0,NULL);  LineTo(hDC,rwpdata->p,rect.bottom);
				r=rwpdata->p+rwpdata->s-(STAMP_WIDTH>>1);
			}
			oldfnt=SelectObject(hDC,rwpdata->hFont);
			SetROP2(hDC,R2_COPYPEN); SetBkMode(hDC,OPAQUE);
			ltxt=sprintf(txt,"%.1fmm ",(float)(r)/10.0);
			SetTextAlign(hDC,TA_LEFT|TA_TOP); TextOut(hDC,0,0,txt,ltxt);
			SelectObject(hDC,oldfnt);
			ReleaseDC(hWnd,hDC);
		}
		break;
	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		hDC=CreateCompatibleDC(ps.hdc);
		oldbmp=SelectObject(hDC,rwpdata->hBmp);
		GetClientRect(hWnd,&rect);
		if(rwpdata->type) { //horiz
			BitBlt(ps.hdc,0,0,rect.right,rect.bottom,hDC,rwpdata->s,0,SRCCOPY);
			SetROP2(ps.hdc,R2_MERGEPENNOT);
			MoveToEx(ps.hdc,rwpdata->p,0,NULL);  LineTo(ps.hdc,rwpdata->p,rect.bottom);
			r=rwpdata->p+rwpdata->s-(STAMP_WIDTH>>1);
		}else{  //vert
			BitBlt(ps.hdc,0,0,rect.right,rect.bottom,hDC,0,rwpdata->s,SRCCOPY);
			SetROP2(ps.hdc,R2_MERGEPENNOT);
			MoveToEx(ps.hdc,0,rwpdata->p,NULL);
			LineTo(ps.hdc,rect.right,rwpdata->p);
			r=-rwpdata->p-rwpdata->s+(STAMP_HEIGHT>>1);
		}
		SelectObject(hDC,oldbmp);
		DeleteDC(hDC);
		oldfnt=SelectObject(ps.hdc,rwpdata->hFont);
		SetROP2(ps.hdc,R2_COPYPEN); SetBkMode(ps.hdc,OPAQUE);
		ltxt=sprintf(txt,"%.1fmm ",(float)(r)/10.0);
		SetTextAlign(ps.hdc,TA_LEFT|TA_TOP); TextOut(ps.hdc,0,0,txt,ltxt);
		SelectObject(ps.hdc,oldfnt);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		DeleteObject(rwpdata->hBmp);
		DeleteObject(rwpdata->hFont);
		delete rwpdata;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0l;
}
