//File:triangle.cpp//

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "stmp.rh"

#include "list.hpp"
#include "stamp.hpp"
#include "triangle.hpp"
GeomTriangle::GeomTriangle(int _x=0,int _y=0,int _s=400,int _t=7) : StampNode(){
	settype(6);
	setgroup(2);
	offx=_x; offy=_y;
	side=_s;
	thick=_t;
}
BOOL GeomTriangle::save(HANDLE hFile)
{
	short t,g,length;
	DWORD dwWritten;
	length=sizeof(offx)+sizeof(offy)+sizeof(side)+sizeof(thick)+sizeof(t)+sizeof(g);
	WriteFile(hFile,&length,sizeof(length),&dwWritten,NULL);
	WriteFile(hFile,&offx,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&offy,sizeof(int),&dwWritten,NULL);
	WriteFile(hFile,&side,sizeof(side),&dwWritten,NULL);
	WriteFile(hFile,&thick,sizeof(int),&dwWritten,NULL);
	t=gettype(); g=getgroup();
	WriteFile(hFile,&t,sizeof(t),&dwWritten,NULL);
	WriteFile(hFile,&g,sizeof(g),&dwWritten,NULL);
	return TRUE;
}
BOOL GeomTriangle::load(HANDLE hFile)
{
	DWORD tg;
	short length;
	DWORD dwReaded;
	ReadFile(hFile,&length,sizeof(length),&dwReaded,NULL);
	ReadFile(hFile,&offx,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&offy,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&side,sizeof(side),&dwReaded,NULL);
	ReadFile(hFile,&thick,sizeof(int),&dwReaded,NULL);
	ReadFile(hFile,&tg,sizeof(DWORD),&dwReaded,NULL);
	settype(LOWORD(tg));
	setgroup(HIWORD(tg));
	return TRUE;
}
void GeomTriangle::draw(HDC hDC)
{
	POINT pnt[8];
	HBRUSH hbr,oldbrush;
	HPEN hpen,oldpen;
	double t=(double)thick/2.0;
	double radius=(double)side/3.464101615;
	pnt[0].x=offx; pnt[0].y=(radius-t)*2+offy;
	pnt[1].x=offx-(radius-t)*1.732051;
	pnt[2].x=offx+(radius-t)*1.732051;
	pnt[1].y=offy-(radius-t);
	pnt[2].y=pnt[1].y;
	pnt[3].x=pnt[0].x; pnt[3].y=pnt[0].y;

	pnt[4].x=offx; pnt[4].y=(radius+thick/2.0)*2+offy;
	pnt[5].x=offx-(radius+t)*1.732051;
	pnt[6].x=offx+(radius+t)*1.732051;
	pnt[5].y=offy-(radius+t);
	pnt[6].y=pnt[5].y;
	pnt[7].x=pnt[4].x; pnt[7].y=pnt[4].y;
	hbr=CreateSolidBrush(0);
	oldbrush=SelectObject(hDC,hbr);
	hpen=CreatePen(PS_NULL,0,0);
	oldpen=SelectObject(hDC,hpen);
	Polygon(hDC,pnt,8);
	SelectObject(hDC,oldbrush);
	SelectObject(hDC,oldpen);
	DeleteObject(hbr);
	DeleteObject(hpen);
}
BOOL GeomTriangle::hittest(int x,int y)
{
	RECT r;
	calcrect(&r,NULL);
	if(x>=r.left&&y<=r.top&&x<=r.right&&y>=r.bottom)
		return TRUE;
	return FALSE;
}
void GeomTriangle::drawcontour(HDC hDC)
{
	POINT pnt[8];
	HBRUSH oldbrush;
	HPEN hpen,oldpen;
	double t=(double)thick/2.0;
	double radius=(double)side/3.464101615;//4.242640287;
	pnt[0].x=offx; pnt[0].y=(radius-t)*2+offy;
	pnt[1].x=offx-(radius-t)*1.732051;
	pnt[1].y=offy-(radius-t);
	pnt[2].x=offx+(radius-t)*1.732051;
	pnt[2].y=pnt[1].y;
	pnt[3].x=pnt[0].x; pnt[3].y=pnt[0].y;

	pnt[4].x=offx; pnt[4].y=(radius+thick/2.0)*2+offy;
	pnt[5].x=offx-(radius+t)*1.732051;
	pnt[6].x=offx+(radius+t)*1.732051;
	pnt[5].y=offy-(radius+t);
	pnt[6].y=pnt[5].y;
	pnt[7].x=pnt[4].x; pnt[7].y=pnt[4].y;
	oldbrush=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	hpen=CreatePen(PS_DOT,0,RGB(64,128,255));
	oldpen=SelectObject(hDC,hpen);
	Polygon(hDC,pnt,8);
	SelectObject(hDC,oldbrush);
	SelectObject(hDC,oldpen);
	DeleteObject(hpen);
}
void GeomTriangle::calcrect(RECT*r,HDC hDC)
{
	double t=(double)thick/2.0;
	r->left=offx-(double)side/2.0-t;
	r->right=offx+(double)side/2.0+t;
	r->top=offy+(int)((double)side*0.577350269)+t;
	r->bottom=offy-t-(int)((double)side*0.288675134);
}

HWND GeomTriangle::starteditor(HWND hMain)
{
	hMainWnd=hMain;
	hWnd=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_TRIANGLEEDITOR),hMainWnd,
	                       (DLGPROC)GTriangleDlgProc,(LPARAM)this);
	if(hWnd==NULL) MessageBox(hMain,"Cannot create dialog",
		                        "triangle.cpp: GeomTriangle::starteditor()",MB_OK);
	return hWnd;
}
BOOL GeomTriangle::endeditor(void)
{
	if(!hWnd) return TRUE;
	if(isChanged()) update();
	DestroyWindow(hWnd);
	hWnd=NULL;
	return TRUE;
}
void GeomTriangle::update(void)
{
	SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
BOOL CALLBACK GTriangleDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	GeomTriangle* gr=(GeomTriangle*)GetWindowLong(hDlg,GWL_USERDATA);
	int ox,oy,t,w,h;
	static BOOL mychange=FALSE;
	BOOL success;
	char txt[32];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,GWL_USERDATA,lParam);
		gr=(GeomTriangle*)lParam;
		gr->getoffset(&ox,&oy); gr->getsize(&w,&t);
		mychange=TRUE;
		gcvt((float)w/10,3,txt); SetDlgItemText(hDlg,IDC_SIDELENGTH,txt);
		gcvt((float)gr->getiradius()/10,3,txt); SetDlgItemText(hDlg,IDC_IRADIUS,txt);
		gcvt((float)gr->getoradius()/10,3,txt); SetDlgItemText(hDlg,IDC_ORADIUS,txt);
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
			case IDC_SIDELENGTH:
				mychange=TRUE;
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getsize(&w,&t);
				GetDlgItemText(hDlg,IDC_SIDELENGTH,txt,32); w=atof(txt)*10;
				gr->setsize(w,t);
				gcvt((float)gr->getoradius()/10,3,txt); SetDlgItemText(hDlg,IDC_ORADIUS,txt);
				gcvt((float)gr->getiradius()/10,3,txt); SetDlgItemText(hDlg,IDC_IRADIUS,txt);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				mychange=FALSE;
				break;
			case IDC_IRADIUS:
				mychange=TRUE;
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				GetDlgItemText(hDlg,IDC_IRADIUS,txt,32); w=atof(txt)*10;
				gr->setiradius(w);
				gr->getsize(&w,&t);
				gcvt((float)w/10,3,txt); SetDlgItemText(hDlg,IDC_SIDELENGTH,txt);
				gcvt((float)gr->getoradius()/10,3,txt); SetDlgItemText(hDlg,IDC_ORADIUS,txt);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				mychange=FALSE;
				break;
			case IDC_ORADIUS:
				mychange=TRUE;
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				GetDlgItemText(hDlg,IDC_ORADIUS,txt,32); w=atof(txt)*10;
				gr->setoradius(w);
				gr->getsize(&w,&t);
				gcvt((float)w/10,3,txt); SetDlgItemText(hDlg,IDC_SIDELENGTH,txt);
				gcvt((float)gr->getiradius()/10,3,txt); SetDlgItemText(hDlg,IDC_IRADIUS,txt);
				SendMessage(gr->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				gr->changed();
				mychange=FALSE;
				break;
			case IDC_THICKNESS:
				SendMessage(gr->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
				gr->getsize(&w,&t);
				GetDlgItemText(hDlg,IDC_THICKNESS,txt,32); t=atof(txt)*10;
				gr->setsize(w,t);
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
			GetDlgItemText(hDlg,IDC_SIDELENGTH,txt,32); w=atof(txt)*10;
			GetDlgItemText(hDlg,IDC_THICKNESS,txt,32); t=atof(txt)*10;
			gr->setsize(w,t);
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

