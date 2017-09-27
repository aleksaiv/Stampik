//File: circle.cpp//

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "stmp.rh"

#include "list.hpp"
#include "stamp.hpp"
#include "circle.hpp"
GeomCircle::GeomCircle(int _x=0,int _y=0,int _r=500,int _t=7) : StampNode(){
	settype(4);
	setgroup(2);
	offx=_x; offy=_y;
	rad=_r; thick=_t;
}
BOOL GeomCircle::save(HANDLE hFile)
{
	short t,g,length;
	DWORD dwWritten;
	length=sizeof(offx)+sizeof(offy)+sizeof(rad)+sizeof(thick)+sizeof(t)+sizeof(g);
	WriteFile(hFile,&length,sizeof(length),&dwWritten,NULL);
	WriteFile(hFile,&offx,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&offy,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&rad,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&thick,sizeof(int),&dwWritten,NULL);
	t=gettype(); g=getgroup();
	WriteFile(hFile,&t,sizeof(t),&dwWritten,NULL);
	WriteFile(hFile,&g,sizeof(g),&dwWritten,NULL);
	return TRUE;
}
BOOL GeomCircle::load(HANDLE hFile)
{
	DWORD tg;
	short length;
	DWORD dwReaded;
	ReadFile(hFile,&length,sizeof(length),&dwReaded,NULL);
	ReadFile(hFile,&offx,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&offy,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&rad,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&thick,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&tg,sizeof(DWORD),&dwReaded,NULL);
	settype(LOWORD(tg));
	setgroup(HIWORD(tg));
	return TRUE;
}
void GeomCircle::draw(HDC hDC)
{
	HPEN pen,oldpen;
	HBRUSH oldbr;
	int x1,y1,x2,y2;
	pen=CreatePen(PS_SOLID,thick,0);
	oldpen=SelectObject(hDC,pen);
	oldbr=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	x1=offx-rad; x2=x1+(rad<<1); y1=offy-rad; y2=y1+(rad<<1);
	Ellipse(hDC,x1,y1,x2,y2);
	SelectObject(hDC,oldpen);
	SelectObject(hDC,oldbr);
	DeleteObject(pen);
}
BOOL GeomCircle::hittest(int x,int y)
{
	int x1,y1,x2,y2;
	x1=offx-rad; x2=x1+(rad<<1);
	y1=offy-rad; y2=y1+(rad<<1);
	if(x>=x1-(thick>>1) && y>y1-((thick-1)>>1) &&
	   x<x2+((thick+1)>>1)-1 && y<=y2+(thick>>1) &&
	   !(x>=x1+((thick+1)>>1) && y>y1+(thick>>1)+1 &&
	     x<x2-(thick>>1)-1 && y<y2-((thick-1)>>1))
	   )
		return TRUE;
	return FALSE;
}
void GeomCircle::drawcontour(HDC hDC)
{
	HBRUSH oldbr=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	HPEN oldpen,hPen;
	hPen=CreatePen(PS_DOT,0,RGB(0,255,255));
	oldpen=SelectObject(hDC,hPen);
	int x1,y1,x2,y2;
	x1=offx-rad; x2=x1+(rad<<1);
	y1=offy-rad; y2=y1+(rad<<1);
	Ellipse(hDC,x1-(thick>>1)-1,y1-((thick-1)>>1)-1,x2+((thick+1)>>1),y2+(thick>>1)+1);
	Ellipse(hDC,x1+((thick+1)>>1),y1+(thick>>1)+1,x2-(thick>>1)-1,y2-((thick-1)>>1)-1);
	SelectObject(hDC,oldpen);
	SelectObject(hDC,oldbr);
	DeleteObject(hPen);
}
void GeomCircle::calcrect(RECT*r,HDC hDC)
{
	r->left=offx-rad-((thick+1)>>1)-1;
	r->bottom=offy-rad-((thick+1)>>1)-1;
	r->right=offx+rad+((thick+1)>>1)+1;
	r->top=offy+rad+((thick+1)>>1)+1;
}

HWND GeomCircle::starteditor(HWND hMain)
{
	hMainWnd=hMain;
	hWnd=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_CIRCLEEDITOR),hMainWnd,
	                       (DLGPROC)GCircleDlgProc,(LPARAM)this);
	if(hWnd==NULL) MessageBox(hMain,"Cannot create dialog",
		                        "circle.cpp: GeomCircle::starteditor()",MB_OK);
	return hWnd;
}
BOOL GeomCircle::endeditor(void)
{
	if(!hWnd) return TRUE;
	if(isChanged()) update();
	DestroyWindow(hWnd);
	hWnd=NULL;
	return TRUE;
}
void GeomCircle::update(void)
{
	SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
BOOL CALLBACK GCircleDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	GeomCircle* gr=(GeomCircle*)GetWindowLong(hDlg,GWL_USERDATA);
	int ox,oy,t,rad;
	static BOOL mychange=FALSE;
	BOOL success;
	char txt[32];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,GWL_USERDATA,lParam);
		gr=(GeomCircle*)lParam;
		gr->getoffset(&ox,&oy); gr->getsize(&rad,&t);
		mychange=TRUE;
		gcvt((float)rad/10,3,txt); SetDlgItemText(hDlg,IDC_RADIUS,txt);
		gcvt((float)rad/5,3,txt); SetDlgItemText(hDlg,IDC_DIAMETER,txt);
		gcvt((float)t/10,3,txt); SetDlgItemText(hDlg,IDC_THICKNESS,txt);
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
				mychange=TRUE;
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getsize(&rad,&t);
				GetDlgItemText(hDlg,IDC_RADIUS,txt,32); rad=atof(txt)*10;
				gcvt((float)rad/5,3,txt); SetDlgItemText(hDlg,IDC_DIAMETER,txt);
				gr->setsize(rad,t);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				mychange=FALSE;
				break;
			case IDC_DIAMETER:
				mychange=TRUE;
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getsize(&rad,&t);
				GetDlgItemText(hDlg,IDC_DIAMETER,txt,32); rad=atof(txt)*5;
				gcvt((float)rad/10,3,txt); SetDlgItemText(hDlg,IDC_RADIUS,txt);
				gr->setsize(rad,t);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				mychange=FALSE;
				break;
			case IDC_THICKNESS:
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getsize(&rad,&t);
				GetDlgItemText(hDlg,IDC_THICKNESS,txt,32); t=atof(txt)*10;
				gr->setsize(rad,t);
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
		case IDOK:
			GetDlgItemText(hDlg,IDC_RADIUS,txt,32); rad=atof(txt)*10;
			GetDlgItemText(hDlg,IDC_THICKNESS,txt,32); t=atof(txt)*10;
			gr->setsize(rad,t);
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

