//File:picbmp.cpp//

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "stmp.rh"
#include "commdlg3.h"

#include "list.hpp"
#include "stamp.hpp"
#include "picbmp.hpp"

#define WIDTHBYTES(bits)      ((((bits) + 31) & ~31) >> 3)

PicBitmap::PicBitmap(int _x=0,int _y=0,int _r=200,char*fn=NULL) : StampNode(){
	settype(7);
	setgroup(0);
	offx=_x; offy=_y;
	rad=_r; w=10; h=10;
	bmp=NULL;
	if(fn) {fname=new char[lstrlen(fn)+1]; strcpy(fname,fn); reload();}
	else fname=NULL;
	resize();
}
BOOL PicBitmap::save(HANDLE hFile)
{
	short t,g,length;
	DWORD dwWritten;
	length=sizeof(offx)+sizeof(offy)+sizeof(rad)+
	        sizeof(length)+lstrlen(fname)+sizeof(t)+sizeof(g);
	WriteFile(hFile,&length,sizeof(length),&dwWritten,NULL);
	WriteFile(hFile,&offx,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&offy,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&rad,sizeof(int),&dwWritten,NULL);
	length=lstrlen(fname);
	WriteFile(hFile,&length,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,fname,length,&dwWritten,NULL);
	t=gettype(); g=getgroup();
	WriteFile(hFile,&t,sizeof(t),&dwWritten,NULL);
	WriteFile(hFile,&g,sizeof(g),&dwWritten,NULL);
	return TRUE;
}
BOOL PicBitmap::load(HANDLE hFile)
{
	DWORD tg;
	short length;
	DWORD dwReaded;
	ReadFile(hFile,&length,sizeof(length),&dwReaded,NULL);
	ReadFile(hFile,&offx,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&offy,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&rad,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&length,sizeof(int),&dwReaded,NULL);
	fname=new char[length+1];
	ReadFile(hFile,fname,length,&dwReaded,NULL); fname[length]=0;
	ReadFile(hFile,&tg,sizeof(DWORD),&dwReaded,NULL);
	settype(LOWORD(tg));
	setgroup(HIWORD(tg));
	reload();
	resize();
	return TRUE;
}
HBITMAP LoadBMPic(char*fn,int *ret)
{
	HBITMAP bmp;
	HANDLE hFile;
	BITMAPFILEHEADER bmpFH;
	HANDLE hBits;
	LPVOID pBits;
	DWORD dwWritten;
	BITMAPINFO bmpI;
	int i; DWORD of=0;
	hFile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
	                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		return NULL; // возвращаем ошибку
	}
	ReadFile(hFile,&bmpFH,sizeof(BITMAPFILEHEADER),&dwWritten,NULL); // Read file header
	if(bmpFH.bfType!=0x4D42) {
		CloseHandle(hFile);
		if(ret) *ret=-1;
		return NULL; // Not a bitmap file
	}
	ReadFile(hFile,&bmpI,sizeof(BITMAPINFO),&dwWritten,NULL); // Read file header
	if(bmpI.bmiHeader.biBitCount!=1) {
		CloseHandle(hFile);
		if(ret) *ret=-2;
		return NULL;
	}
	if(bmpI.bmiHeader.biSizeImage==0)
		bmpI.bmiHeader.biSizeImage=(((bmpI.bmiHeader.biWidth+15)>>4)<<1)*
		                            bmpI.bmiHeader.biHeight;
	SetFilePointer(hFile,bmpFH.bfOffBits, NULL, FILE_BEGIN);
	hBits=GlobalAlloc(GHND,bmpI.bmiHeader.biSizeImage+4);
	if(hBits==NULL) {
		if(ret) *ret=-3;
		CloseHandle(hFile);
		return NULL;
	}
	pBits=(LPVOID)GlobalLock(hBits);
	of=0;
	for(i=0; i<bmpI.bmiHeader.biHeight; i++) {
		ReadFile(hFile,(char*)pBits+of,(((bmpI.bmiHeader.biWidth+31)>>5)<<2),&dwWritten,NULL);
		of+=(((bmpI.bmiHeader.biWidth+15)>>4)<<1);
	}
	bmp=CreateBitmap(bmpI.bmiHeader.biWidth,bmpI.bmiHeader.biHeight,1,1,pBits);
	GlobalUnlock(hBits);
	GlobalFree(hBits);
	if(bmp) {
		HDC hDC=CreateCompatibleDC(NULL);
		HBITMAP old=SelectObject(hDC,bmp);
		PatBlt(hDC,0,0,bmpI.bmiHeader.biWidth,bmpI.bmiHeader.biHeight,DSTINVERT);
		SelectObject(hDC,old);
		DeleteDC(hDC);
	}else if(ret) *ret=-4;
	CloseHandle(hFile);
	if(ret) *ret=0;
	return bmp;
}
int PicBitmap::reload(void)
{
	BITMAP b;
	int ret;
	if(bmp) DeleteObject(bmp);
	w=0; h=0;
	bmp=LoadBMPic(fname,&ret);
	if(bmp==NULL) return ret;
	GetObject(bmp,sizeof(BITMAP),&b);
	w=b.bmWidth; h=b.bmHeight;
	return 0;
}
void PicBitmap::draw(HDC hDC)
{
	int x1,y1;
	if(!bmp) return;
	HBITMAP oldbmp;
	HDC hDC2=CreateCompatibleDC(NULL);
	oldbmp=SelectObject(hDC2,bmp);
	x1=offx-(zw>>1); y1=offy-(zh>>1);
	StretchBlt(hDC,x1,y1,zw,zh,hDC2,0,0,w,h,SRCINVERT);
	SelectObject(hDC2,oldbmp);
	DeleteDC(hDC2);
}
BOOL PicBitmap::hittest(int x,int y)
{
	int x1,y1,x2,y2;
	x1=offx-(zw>>1); x2=x1+zw;
	y1=offy-(zh>>1); y2=y1+zh;
	if(x>=x1&&y>y1&&x<x2&&y<=y2) return TRUE;
	return FALSE;
}
void PicBitmap::drawcontour(HDC hDC)
{
	HPEN oldpen,hPen;
	HBRUSH oldbr=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	hPen=CreatePen(PS_DOT,0,RGB(0,255,255));
	oldpen=SelectObject(hDC,hPen);
	int x1,y1,x2,y2;
	x1=offx-(zw>>1); x2=x1+zw;
	y1=offy-(zh>>1); y2=y1+zh;
	Rectangle(hDC,x1,y1,x2,y2);
	Ellipse(hDC,offx-rad,offy+rad,offx+rad,offy-rad);
	SelectObject(hDC,oldpen);
	SelectObject(hDC,oldbr);
	DeleteObject(hPen);
}
void PicBitmap::calcrect(RECT*r,HDC hDC)
{
	r->left=offx-(zw>>1)-1;
	r->bottom=offy-(zh>>1)-1;
	r->right=offx+(zw>>1)+1;
	r->top=offy+(zh>>1)+1;
}

HWND PicBitmap::starteditor(HWND hMain)
{
	hMainWnd=hMain;
	hWnd=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PICBITMAPEDITOR),
	                       hMainWnd,(DLGPROC)PBitmapDlgProc,(LPARAM)this);
	if(hWnd==NULL) MessageBox(hMain,"Cannot create dialog",
		                        "picbmp.cpp: PicBitmap::starteditor()",MB_OK);
	return hWnd;
}
BOOL PicBitmap::endeditor(void)
{
	if(!hWnd) return TRUE;
	if(isChanged()) update();
	DestroyWindow(hWnd);
	hWnd=NULL;
	return TRUE;
}
void PicBitmap::update(void)
{
	SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
extern HINSTANCE hInst;
void SelectBmpFile(HWND hDlg,PicBitmap* pb)
{
	char szTemp[] = "Bitmap files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
	char szName[256];
	OPENFILENAME ofn;
	HANDLE hFile;
	strcpy(szName,"*.bmp");
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = hDlg; // An invalid hWnd causes non-modality
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
	ofn.lpstrTitle = "Select bitmap";  // Title for dialog
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if(GetOpenFileName( &ofn ) != TRUE)
	{
		if(CommDlgExtendedError()!=0) // 0 value means user selected Cancel
			MessageBox(hDlg,"Cannot get name of file","WARNING",MB_OK|MB_ICONSTOP);
		else return;
		return;
	}
	pb->setfname(szName);
	int ret=pb->reload();
	if(ret) {
		switch(ret) {
		case -1: MessageBox(hDlg,"Cannot load bitmap from file",
			                  "Error",MB_OK|MB_ICONASTERISK);
			break;
		case -2: MessageBox(hDlg,"Bitmpa must be black and white",
			                  "Error",MB_OK|MB_ICONASTERISK);
			break;
		case -3: MessageBox(hDlg,"Cannot allocate more memory",
			                  "Error",MB_OK|MB_ICONASTERISK);
			break;
		}
		pb->setfname(NULL);
	}
	pb->resize();
}
BOOL CALLBACK PBitmapDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PicBitmap* gr=(PicBitmap*)GetWindowLong(hDlg,GWL_USERDATA);
	int ox,oy,t,w,h;
	static BOOL mychange=FALSE;
	BOOL success;
	char txt[32];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,GWL_USERDATA,lParam);
		gr=(PicBitmap*)lParam;
		gr->getoffset(&ox,&oy); gr->getsize(&w);
		mychange=TRUE;
		SetDlgItemText(hDlg,IDC_PATH,gr->getfname() ? gr->getfname() : "no file selected");
		gcvt((float)w/10,3,txt); SetDlgItemText(hDlg,IDC_RADIUS,txt);
		gcvt((float)ox/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERX,txt);
		gcvt((float)oy/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERY,txt);
		mychange=FALSE;
		gr->updated();
		break;
	case WM_SETCURSOR:
		if(gr)
			SendMessage(gr->getmainwnd(),WM_SETSTATUSTEXT,IDS_EDITORTEXT,0);
		return DefWindowProc(hDlg,uMsg,wParam,lParam);
	case WM_MOVEOBJECT:
		mychange=TRUE;
		gr->getoffset(&ox,&oy);
		if(wParam==0) {
			ox+=(int)((short)LOWORD(lParam)); oy+=(int)((short)HIWORD(lParam));
		}
		gcvt((float)ox/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERX,txt);
		gcvt((float)oy/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERY,txt);
		gr->updated();
		mychange=FALSE;
		break;
	case WM_COMMAND: {
		if(HIWORD(wParam)==EN_CHANGE&&mychange==FALSE) {
			switch(LOWORD(wParam)) {
			case IDC_RADIUS:
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getsize(&w);
				GetDlgItemText(hDlg,IDC_RADIUS,txt,32); w=atof(txt)*10;
				gr->setsize(w);
				gr->resize();
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				break;
			case IDC_CENTERX:
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getoffset(&ox,&oy);
				GetDlgItemText(hDlg,IDC_CENTERX,txt,32); ox=atof(txt)*10;
				gr->setoffset(ox,oy);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				break;
			case IDC_CENTERY:
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getoffset(&ox,&oy);
				GetDlgItemText(hDlg,IDC_CENTERY,txt,32); oy=atof(txt)*10;
				gr->setoffset(ox,oy);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				break;
			}
		}
		switch(LOWORD(wParam)) {
		case IDC_CHANGE:
			mychange=TRUE;
			SelectBmpFile(hDlg,gr);
			gr->getoffset(&ox,&oy); gr->getsize(&w);
			SetDlgItemText(hDlg,IDC_PATH,gr->getfname() ? gr->getfname() : "no file selected");
			gcvt((float)w/10,3,txt); SetDlgItemText(hDlg,IDC_RADIUS,txt);
			gcvt((float)ox/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERX,txt);
			gcvt((float)oy/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERY,txt);
			mychange=FALSE;
			gr->changed();
			gr->update();
			break;
		case IDOK:
			GetDlgItemText(hDlg,IDC_RADIUS,txt,32); w=atof(txt)*10;
			gr->setsize(w);
			GetDlgItemText(hDlg,IDC_CENTERX,txt,32); ox=atof(txt)*10;
			GetDlgItemText(hDlg,IDC_CENTERY,txt,32); oy=atof(txt)*10;
			gr->setoffset(ox,oy);
			gr->update();
			break;
		case IDCANCEL:
			gr->close();
			break;
		case IDC_DELETE:
			gr->del();
			break;
		}
	} break;
	default: return FALSE;
	}
	return TRUE;
}

