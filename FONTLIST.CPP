//File:fontlist.cpp//

#include <windows.h>
int CALLBACK EFFProc(ENUMLOGFONT FAR* lpelf,NEWTEXTMETRIC FAR* lpntm,int ftype,LPARAM lParam)
{
	if(ftype==TRUETYPE_FONTTYPE) {
		SendMessage((HWND)lParam,CB_ADDSTRING,0,(LPARAM)(lpelf->elfLogFont.lfFaceName));
	}
	return 0x123;
}
int CALLBACK EFFProc2(ENUMLOGFONT FAR* lpelf,NEWTEXTMETRIC FAR* lpntm,int ftype,LPARAM lParam)
{
	if(ftype==TRUETYPE_FONTTYPE) {
		SendMessage((HWND)lParam,LB_ADDSTRING,0,(LPARAM)(lpelf->elfLogFont.lfFaceName));
	}
	return 0x123;
}
void FillFontCombobox(char*defname,HWND hWnd)
{
	HDC hDC=CreateCompatibleDC(NULL);
	SendMessage(hWnd,CB_RESETCONTENT,0,0);
	EnumFontFamilies(hDC,NULL,(FONTENUMPROC)EFFProc,(LPARAM)hWnd);
	SendMessage(hWnd,CB_SELECTSTRING,0,(LPARAM)defname);
	DeleteDC(hDC);
}
void FillFontListbox(char*defname,HWND hWnd)
{
	HDC hDC=CreateCompatibleDC(NULL);
	SendMessage(hWnd,LB_RESETCONTENT,0,0);
	EnumFontFamilies(hDC,NULL,(FONTENUMPROC)EFFProc2,(LPARAM)hWnd);
	SendMessage(hWnd,LB_SELECTSTRING,0,(LPARAM)defname);
	DeleteDC(hDC);
}
#define DEFAULTSTRING "123ABCXYZabcxyzÀÁÂÝÞßàáâãýþÿ"
char fvtext[256];
char fvfname[64];
int fvstyle=0;
HWND fvWnd;
HINSTANCE hMainInst;
int fvX,fvY,fvWidth,fvHeight,fvFontHeight;

void calcsize(void)
{
	SIZE size;
	HFONT oldfont,hFont=CreateFont(fvFontHeight,0,0,0,fvstyle&1 ? 700 : 400,
	                               fvstyle&2 ? TRUE : FALSE,
	                               fvstyle&4 ? TRUE : FALSE,FALSE,
	                               ANSI_CHARSET,
	                               OUT_CHARACTER_PRECIS,
	                               CLIP_CHARACTER_PRECIS,
	                               PROOF_QUALITY,
	                               DEFAULT_PITCH|FF_DONTCARE,
	                               fvfname);
	HDC hDC=CreateCompatibleDC(NULL);
	oldfont=SelectObject(hDC,hFont);
	GetTextExtentPoint32(hDC,fvtext,lstrlen(fvtext),&size);
	fvWidth=size.cx+12; fvHeight=size.cy+12;
	SelectObject(hDC,oldfont);
	DeleteObject(hFont);
	DeleteDC(hDC);
	if(fvWidth>480) fvWidth=480;
	MoveWindow(fvWnd,fvX,fvY,fvWidth,fvHeight,TRUE);
}

HWND CreateFontViewer(HINSTANCE hI,HWND hMain)
{
	hMainInst=hI;
	fvX=fvY=0;
	fvWidth=320; fvHeight=100; fvFontHeight=32;
	fvWnd=CreateWindow("stampfontviewer",DEFAULTSTRING,WS_POPUP,
	                   fvX,fvY,fvWidth,fvHeight,
	                   hMain,NULL,hI,NULL);
	if(!fvWnd) return NULL;
	calcsize();
	return fvWnd;
}
void CloseFontViewer(void)
{
	if(fvWnd) DestroyWindow(fvWnd);
}
void SetFontViewerSize(int h)
{
	fvFontHeight=h;
}
void SetFontViewer(char*txt,char*fname,int st)
{
	if(txt) strcpy(fvtext,txt); else strcpy(fvtext,DEFAULTSTRING);
	if(fname) strcpy(fvfname,fname);
	fvstyle=st;
	calcsize();
}
void MoveFontViewer(int x,int y)
{
	fvX=x; fvY=y;
	MoveWindow(fvWnd,fvX,fvY,fvWidth,fvHeight,TRUE);
}
void ShowFontViewer(void)
{
	ShowWindow(fvWnd,SW_SHOWNOACTIVATE);
}
void HideFontViewer(void)
{
	ShowWindow(fvWnd,SW_HIDE);
}
LRESULT CALLBACK FontViewerProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rect;
	HFONT hFont,oldfont;
	switch(uMsg) {
	case WM_PAINT:
		hFont=CreateFont(fvFontHeight,0,0,0,fvstyle&1 ? 700 : 400,
		                 fvstyle&2 ? TRUE : FALSE,
		                 fvstyle&4 ? TRUE : FALSE,FALSE,
		                 ANSI_CHARSET,
		                 OUT_CHARACTER_PRECIS,
		                 CLIP_CHARACTER_PRECIS,
		                 PROOF_QUALITY,
		                 DEFAULT_PITCH|FF_DONTCARE,
		                 fvfname);
		BeginPaint(hWnd,&ps);
		oldfont=SelectObject(ps.hdc,hFont);
		GetClientRect(hWnd,&rect);
		Rectangle(ps.hdc,0,0,rect.right,rect.bottom);
		rect.left+=5; rect.top+=5; rect.right-=5; rect.bottom-=5;
		ExtTextOut(ps.hdc,6,6,ETO_CLIPPED|ETO_OPAQUE,&rect,
		           fvtext,lstrlen(fvtext),NULL);
		SelectObject(ps.hdc,oldfont);
		EndPaint(hWnd,&ps);
		DeleteObject(hFont);
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
