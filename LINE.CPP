//File:line.cpp//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "stmp.rh"
#include "list.hpp"
#include "stamp.hpp"
#include "line.hpp"

BOOL CALLBACK GLineDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

BOOL GeomLine::save(HANDLE hFile)
{
	short t,g,length;
	DWORD dwWritten;
	length=sizeof(x1)+sizeof(y1)+sizeof(x2)+sizeof(y2)+sizeof(thick)+sizeof(style)+sizeof(t)+sizeof(g);
	WriteFile(hFile,&length,sizeof(length),&dwWritten,NULL);
	WriteFile(hFile,&x1,sizeof(x1),&dwWritten,NULL);
	WriteFile(hFile,&y1,sizeof(y1),&dwWritten,NULL);
	WriteFile(hFile,&x2,sizeof(x2),&dwWritten,NULL);
	WriteFile(hFile,&y2,sizeof(y2),&dwWritten,NULL);
	WriteFile(hFile,&thick,sizeof(thick),&dwWritten,NULL);
	WriteFile(hFile,&style,sizeof(style),&dwWritten,NULL);
	t=gettype(); g=getgroup();
	WriteFile(hFile,&t,sizeof(t),&dwWritten,NULL);
	WriteFile(hFile,&g,sizeof(g),&dwWritten,NULL);
	return TRUE;
}
BOOL GeomLine::load(HANDLE hFile)
{
	DWORD tg;
	short length;
	DWORD dwReaded;
	ReadFile(hFile,&length,sizeof(length),&dwReaded,NULL);
	ReadFile(hFile,&x1,sizeof(x1),&dwReaded,NULL);
	ReadFile(hFile,&y1,sizeof(y1),&dwReaded,NULL);
	ReadFile(hFile,&x2,sizeof(x2),&dwReaded,NULL);
	ReadFile(hFile,&y2,sizeof(y2),&dwReaded,NULL);
	ReadFile(hFile,&thick,sizeof(thick),&dwReaded,NULL);
	ReadFile(hFile,&style,sizeof(style),&dwReaded,NULL);
	ReadFile(hFile,&tg,sizeof(DWORD),&dwReaded,NULL);
	settype(LOWORD(tg));
	setgroup(HIWORD(tg));
	return TRUE;
}

void GeomLine::draw(HDC hDC)
{
	HPEN oldpen,hPen=CreatePen(style,thick,0);
	oldpen=SelectObject(hDC,hPen);
	MoveToEx(hDC,x1,y1,NULL); LineTo(hDC,x2,y2);
	SelectObject(hDC,oldpen);
	DeleteObject(hPen);
}
void GeomLine::drawcontour(HDC hDC)
{
	HPEN oldpen,hPen=CreatePen(style,thick,RGB(255,127,63));
	oldpen=SelectObject(hDC,hPen);
	MoveToEx(hDC,x1,y1,NULL); LineTo(hDC,x2,y2);
	SelectObject(hDC,oldpen);
	DeleteObject(hPen);
}
inline int max(int a,int b){
	return a>b ? a : b;
}
inline int min(int a,int b){
	return a<b ? a : b;
}
BOOL GeomLine::hittest(int x,int y)
{
	RECT r;
	r.left=min(x1,x2); r.right=max(x1,x2);
	r.top=max(y1,y2); r.bottom=min(y1,y2);
	if(x>=r.left&&y>=r.bottom&&x<=r.right&&y<=r.top) return TRUE; else return FALSE;
}

HWND GeomLine::starteditor(HWND hMain)
{
	hMainWnd=hMain;
	hWnd=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_LINEEDITOR),hMainWnd,(DLGPROC)GLineDlgProc,(LPARAM)this);
	if(hWnd==NULL) MessageBox(hMain,"Cannot create dialog","line.cpp: GeomLine::starteditor()",MB_OK);
	return hWnd;
}
BOOL GeomLine::endeditor(void)
{
	if(!hWnd) return TRUE;
	if(isChanged()) update();
	DestroyWindow(hWnd);
	hWnd=NULL;
	return TRUE;
}
void GeomLine::update(void)
{
	SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
BOOL CALLBACK GLineDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	GeomLine* gl=(GeomLine*)GetWindowLong(hDlg,GWL_USERDATA);
	int x1,x2,y1,y2;
	static BOOL mychange=FALSE;
	char txt[32];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,GWL_USERDATA,lParam);
		gl=(GeomLine*)lParam;
		gl->getcoord(x1,y1,x2,y2);
		mychange=TRUE;
		gcvt((float)x1/10,3,txt); SetDlgItemText(hDlg,IDC_CRDX1,txt);
		gcvt((float)y1/10,3,txt); SetDlgItemText(hDlg,IDC_CRDY1,txt);
		gcvt((float)x2/10,3,txt); SetDlgItemText(hDlg,IDC_CRDX2,txt);
		gcvt((float)y2/10,3,txt); SetDlgItemText(hDlg,IDC_CRDY2,txt);
		x1=gl->getthick(); x2=gl->getstyle();
		gcvt((float)x1/10,3,txt); SetDlgItemText(hDlg,IDC_THICKNESS,txt);
		gcvt((float)x2,3,txt); SetDlgItemText(hDlg,IDC_STYLE,txt);
		mychange=FALSE;
		gl->updated();
		break;
	case WM_SETCURSOR:
		if(gl)
			SendMessage(gl->getmainwnd(),WM_SETSTATUSTEXT,IDS_EDITORTEXT,0);
		return DefWindowProc(hDlg,uMsg,wParam,lParam);
	case WM_MOVEOBJECT:
		mychange=TRUE;
		gl->getcoord(x1,y1,x2,y2);
		if(wParam==0) {
			x1+=(int)((short)LOWORD(lParam)); x2+=(int)((short)LOWORD(lParam));
			y1+=(int)((short)HIWORD(lParam)); y2+=(int)((short)HIWORD(lParam));
		}
		gcvt((float)x1/10,3,txt); SetDlgItemText(hDlg,IDC_CRDX1,txt);
		gcvt((float)y1/10,3,txt); SetDlgItemText(hDlg,IDC_CRDY1,txt);
		gcvt((float)x2/10,3,txt); SetDlgItemText(hDlg,IDC_CRDX2,txt);
		gcvt((float)y2/10,3,txt); SetDlgItemText(hDlg,IDC_CRDY2,txt);
		gl->updated();
		mychange=FALSE;
		break;
	case WM_COMMAND:
		if(HIWORD(wParam)==EN_CHANGE&&mychange==FALSE)
			switch(LOWORD(wParam)) {
			case IDC_CRDX1:
				SendMessage(gl->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gl->getcoord(x1,y1,x2,y2);
				GetDlgItemText(hDlg,IDC_CRDX1,txt,32); x1=atof(txt)*10;
				gl->setcoord(x1,y1,x2,y2);
				SendMessage(gl->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gl->changed();
				break;
			case IDC_CRDX2:
				SendMessage(gl->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gl->getcoord(x1,y1,x2,y2);
				GetDlgItemText(hDlg,IDC_CRDX2,txt,32); x2=atof(txt)*10;
				gl->setcoord(x1,y1,x2,y2);
				SendMessage(gl->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gl->changed();
				break;
			case IDC_CRDY1:
				SendMessage(gl->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gl->getcoord(x1,y1,x2,y2);
				GetDlgItemText(hDlg,IDC_CRDY1,txt,32); y1=atof(txt)*10;
				gl->setcoord(x1,y1,x2,y2);
				SendMessage(gl->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gl->changed();
				break;
			case IDC_CRDY2:
				SendMessage(gl->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gl->getcoord(x1,y1,x2,y2);
				GetDlgItemText(hDlg,IDC_CRDY2,txt,32); y2=atof(txt)*10;
				gl->setcoord(x1,y1,x2,y2);
				SendMessage(gl->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gl->changed();
				break;
			case IDC_THICKNESS:
				SendMessage(gl->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				GetDlgItemText(hDlg,IDC_THICKNESS,txt,32); x1=atof(txt)*10;
				gl->setthick(x1);
				SendMessage(gl->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gl->changed();
				break;
			}
		else switch(LOWORD(wParam)) {
			case IDOK:
				GetDlgItemText(hDlg,IDC_CRDX1,txt,32); x1=atof(txt)*10;
				GetDlgItemText(hDlg,IDC_CRDY1,txt,32); y1=atof(txt)*10;
				GetDlgItemText(hDlg,IDC_CRDX2,txt,32); x2=atof(txt)*10;
				GetDlgItemText(hDlg,IDC_CRDY2,txt,32); y2=atof(txt)*10;
				gl->setcoord(x1,y1,x2,y2);
				GetDlgItemText(hDlg,IDC_THICKNESS,txt,32);
				gl->setthick((int)(atof(txt)*10.0));
				GetDlgItemText(hDlg,IDC_STYLE,txt,32);
				gl->setstyle((int)atof(txt));
				gl->update();
				break;
			case IDCANCEL:
				gl->close();
				break;
			case IDC_DELETE:
				gl->del();
				break;
			}
		break;
	default: return FALSE;
	}
	return TRUE;
}
