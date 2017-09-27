//File: ctext.cpp//

#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include "stmp.rh"
#include "list.hpp"
#include "stamp.hpp"
#include "ctext.hpp"
#include "fontlist.hpp"

BOOL CALLBACK CTextDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
CircleText::CircleText() : text(NULL) {
	settype(1);
	setgroup(1);
	strcpy(fname,"Arial Cyr");
	align=1; h=32; width=360;
	org=FALSE; radius=250;
	space=0; offx=offy=0;
	bold=FALSE; italic=FALSE; underline=FALSE;
	angle=90; thick=0; bthick=FALSE;
	forprint=0;
	calccontour(NULL);
}
CircleText::~CircleText()
{
	if(text) delete text;
}
void CircleText::settext(char* t)
{
	if(text) delete text;
	if(!t) return;
	text=new char[lstrlen(t)+1];
	if(text) strcpy(text,t);
}
BOOL CircleText::save(HANDLE hFile)
{
	short length=0,t,g;
	DWORD dwWritten;
	length=sizeof(t)+sizeof(g)+sizeof(fname)+sizeof(offx)+sizeof(offy)+sizeof(h)+
	        sizeof(org)+sizeof(radius)+sizeof(angle)+sizeof(align)+sizeof(bold)+
	        sizeof(italic)+sizeof(underline)+sizeof(width)+sizeof(space)+
	        sizeof(bthick)+sizeof(thick)+sizeof(length)+lstrlen(text);
	WriteFile(hFile,&length,sizeof(length),&dwWritten,NULL);
	if(text) length=(short)lstrlen(text);
	WriteFile(hFile,&length,sizeof(short),&dwWritten,NULL);
	if(text) WriteFile(hFile,text,length,&dwWritten,NULL);
	t=gettype(); g=getgroup();
	WriteFile(hFile,&t,sizeof(t),&dwWritten,NULL);
	WriteFile(hFile,&g,sizeof(g),&dwWritten,NULL);
	WriteFile(hFile,fname,sizeof(fname),&dwWritten,NULL);
	WriteFile(hFile,&offx,sizeof(offx),&dwWritten,NULL);
	WriteFile(hFile,&offy,sizeof(offy),&dwWritten,NULL);
	WriteFile(hFile,&org,sizeof(org),&dwWritten,NULL);
	WriteFile(hFile,&radius,sizeof(radius),&dwWritten,NULL);
	WriteFile(hFile,&h,sizeof(h),&dwWritten,NULL);
	WriteFile(hFile,&angle,sizeof(angle),&dwWritten,NULL);
	WriteFile(hFile,&align,sizeof(align),&dwWritten,NULL);
	WriteFile(hFile,&bold,sizeof(bold),&dwWritten,NULL);
	WriteFile(hFile,&italic,sizeof(italic),&dwWritten,NULL);
	WriteFile(hFile,&underline,sizeof(underline),&dwWritten,NULL);
	WriteFile(hFile,&width,sizeof(width),&dwWritten,NULL);
	WriteFile(hFile,&space,sizeof(space),&dwWritten,NULL);
	WriteFile(hFile,&bthick,sizeof(bthick),&dwWritten,NULL);
	WriteFile(hFile,&thick,sizeof(thick),&dwWritten,NULL);
	return TRUE;
}
BOOL CircleText::load(HANDLE hFile)
{
	short length=0;
	DWORD tg;
	DWORD dwReaded;
	if(text) delete text;
	ReadFile(hFile,&length,sizeof(length),&dwReaded,NULL);
	ReadFile(hFile,&length,sizeof(short),&dwReaded,NULL);
	if(length>0) {
		text=new char[length+1];
		ReadFile(hFile,text,length,&dwReaded,NULL); text[length]=0;
	}
	ReadFile(hFile,&tg,sizeof(DWORD),&dwReaded,NULL);
	settype(LOWORD(tg));
	setgroup(HIWORD(tg));
	ReadFile(hFile,fname,sizeof(fname),&dwReaded,NULL);
	ReadFile(hFile,&offx,sizeof(offx),&dwReaded,NULL);
	ReadFile(hFile,&offy,sizeof(offy),&dwReaded,NULL);
	ReadFile(hFile,&org,sizeof(org),&dwReaded,NULL);
	ReadFile(hFile,&radius,sizeof(radius),&dwReaded,NULL);
	ReadFile(hFile,&h,sizeof(h),&dwReaded,NULL);
	ReadFile(hFile,&angle,sizeof(angle),&dwReaded,NULL);
	ReadFile(hFile,&align,sizeof(align),&dwReaded,NULL);
	ReadFile(hFile,&bold,sizeof(bold),&dwReaded,NULL);
	ReadFile(hFile,&italic,sizeof(italic),&dwReaded,NULL);
	ReadFile(hFile,&underline,sizeof(underline),&dwReaded,NULL);
	ReadFile(hFile,&width,sizeof(width),&dwReaded,NULL);
	ReadFile(hFile,&space,sizeof(space),&dwReaded,NULL);
	ReadFile(hFile,&bthick,sizeof(bthick),&dwReaded,NULL);
	ReadFile(hFile,&thick,sizeof(thick),&dwReaded,NULL);
	return TRUE;
}
inline int max(int a,int b){
	return a>b ? a : b;
}
inline int min(int a,int b){
	return a<b ? a : b;
}
inline double abs(double a){
	return a<0 ? -a : a;
}
void CircleText::print(HDC hDC)
{
	forprint=1;
	draw(hDC);
	forprint=0;
}
void CircleText::draw(HDC hDC)
{
	if(!text) return;
	OUTLINETEXTMETRIC otm;
	TEXTMETRIC tm;
	SIZE size;
	int i,numchar,bc;
	double ang,bbb,sa,ea;
	int rad;
	int *dx=new int[lstrlen(text)+1];
	int *ss=new int[lstrlen(text)+1];
	char tmp[128];
	HFONT hFont,oldfont;
//  if(forprint)angle=-angle;
	LOGFONT lfnt;
	lfnt.lfHeight=h;
	lfnt.lfWidth=bthick ? thick : 0;
	lfnt.lfWeight=bold ? 700 : 400;
	lfnt.lfItalic=italic;
	lfnt.lfUnderline=underline;
	lfnt.lfStrikeOut=FALSE;
	lfnt.lfEscapement=0;
	lfnt.lfOrientation=0;
	lfnt.lfCharSet=ANSI_CHARSET;
	lfnt.lfOutPrecision=OUT_CHARACTER_PRECIS;
	lfnt.lfClipPrecision=CLIP_CHARACTER_PRECIS;
	lfnt.lfQuality=PROOF_QUALITY;
	lfnt.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
	lstrcpy(lfnt.lfFaceName,fname);
	hFont=CreateFontIndirect(&lfnt);
	oldfont=SelectObject(hDC,hFont);
	SetTextCharacterExtra(hDC,space);
	SetTextJustification(hDC,0,0);
	SetTextAlign(hDC,TA_BASELINE|TA_CENTER);
	GetOutlineTextMetrics(hDC,sizeof(OUTLINETEXTMETRIC),&otm);
	if(org) rad=radius+otm.otmMacAscent; else rad=radius;
	if(rad==0) rad=1;
	for(i=0; i<lstrlen(text); i++) {
		GetTextExtentPoint32(hDC,text+i,1,&size); ss[i]=size.cx;
	}
	if(align==3) {
		for(i=0,bc=0; i<lstrlen(text); i++)
			if(text[i]==otm.otmTextMetrics.tmBreakChar) bc++;
		GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
		if(org) {
			ea=angle+(width>>1); sa=angle-(width>>1);
		}else{
			ea=angle-(width>>1); sa=angle+(width>>1);
		}
		contb=width; conta=angle-(width>>1);
		i=(abs((double)sa-(double)ea)*3.1415926535/180.0)*(double)rad;
		if(bc>0) numchar=(double)(i-size.cx)/(double)bc; else numchar=0;
		size.cx=i; bc=0;
		for(i=0; i<lstrlen(text); i++) {
			dx[i]=bc+(ss[i]>>1);
			bc+=ss[i]+space;
			if(text[i]==' ') bc+=numchar;
			if(bc>size.cx) break;
		}
		numchar=i;
	}else{
		GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
		contb=i=((double)size.cx/(double)rad)*180/3.1415926535;
		if(org) {
			if(align==0) {sa=angle-i; conta=angle-i;}
			if(align==1) {sa=angle-(i>>1); conta=angle-(i>>1);}
			if(align==2) {sa=angle; conta=angle;}
		}else{
			if(align==0) {sa=angle; conta=angle-i;}
			if(align==1) {sa=angle+(i>>1); conta=angle-(i>>1);}
			if(align==2) {sa=angle+i; conta=angle;}
		}
		GetTextExtentExPoint(hDC,text,lstrlen(text),size.cx,&numchar,dx,&size);
		for(i=numchar-1; i>0; i--) dx[i]=dx[i-1]+(ss[i]>>1); dx[0]=ss[0]>>1;
	}
	SetTextCharacterExtra(hDC,0); SetTextJustification(hDC,0,0);
	DeleteObject(hFont);
	sa=sa*3.14159265/180.0;//ea=ea*3.14159265/180.0;
	if(org) { /*sa-=6.283185306;*/ conta-=360;}
	for(i=0; i<numchar; i++) {
		if(org==FALSE) {
			ang=(double)sa-((double)dx[i])/(double)rad;
			bbb=ang*1800/3.141592653585-900;
		}else{
			ang=(double)sa+((double)dx[i])/(double)rad;
			bbb=ang*1800/3.141592653585+900;
		}
		if(forprint) bbb=-bbb;
		lfnt.lfEscapement=(long)(-bbb);
		hFont=CreateFontIndirect(&lfnt);
		SelectObject(hDC,hFont);
		TextOut(hDC,offx+(double)rad*cos(ang),offy+(double)rad*sin(ang),text+i,1);
		DeleteObject(hFont);
	}
	SelectObject(hDC,oldfont);
//  if(forprint)angle=-angle;
}

BOOL CircleText::hittest(int x,int y)
{
	if(x>=offx-radius&&y>=offy-radius&&x<=offx+radius&&y<=offy+radius)
		return FALSE;
	if(x>=offx-radius-h&&y>=offy-radius-h&&x<=offx+radius+h&&y<=offy+radius+h)
		return TRUE;
	else return FALSE;
}
void CircleText::drawcontour(HDC hDC)
{
	HBRUSH oldbr=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	HPEN oldpen,hPen;
	int x1,y1,x2,y2,hh;
	hPen=CreatePen(PS_DOT,0,RGB(0,255,255));
	oldpen=SelectObject(hDC,hPen);
	if(h<0) hh=-h; else hh=h;
	x1=offx+(double)radius*cos((conta)*3.14159653585/180);
	y1=offy+(double)radius*sin((conta)*3.14159653585/180);
	x2=offx+(double)radius*cos((conta+contb)*3.14159653585/180);
	y2=offy+(double)radius*sin((conta+contb)*3.14159653585/180);
	Arc(hDC,offx-radius,offy+radius,offx+radius,offy-radius,x1,y1,x2,y2);
	x1=offx+(double)(radius+hh)*cos((conta)*3.14159653585/180);
	y1=offy+(double)(radius+hh)*sin((conta)*3.14159653585/180);
	x2=offx+(double)(radius+hh)*cos((conta+contb)*3.14159653585/180);
	y2=offy+(double)(radius+hh)*sin((conta+contb)*3.14159653585/180);
	Arc(hDC,offx-radius-hh,offy+radius+hh,offx+radius+hh,offy-radius-hh,x1,y1,x2,y2);
	MoveToEx(hDC,x1,y1,NULL);
	LineTo(hDC,offx,offy); LineTo(hDC,x2,y2);
	SelectObject(hDC,oldpen);
	SelectObject(hDC,oldbr);
	DeleteObject(hPen);
}
void CircleText::calcrect(RECT*r,HDC hDC)
{
	calccontour(hDC);
	r->left=offx-radius; r->top=offy+radius; r->right=offx+radius; r->bottom=offy-radius;
	r->left-=h; r->top+=h; r->bottom-=h; r->right+=h;
}
void CircleText::calccontour(HDC hDC)
{
	SIZE size;
	HFONT hFont,oldfont;
	int rad;
	OUTLINETEXTMETRIC otm;
	BOOL hdc;
	if(!text) {conta=angle; contb=0; return;}
	if(hDC==NULL) {
		hDC=CreateCompatibleDC(NULL);
		hdc=TRUE;
	}else hdc=FALSE;
	hFont=CreateFont(h,bthick ? thick : 0,0,0,bold ? 700 : 400,italic,underline,FALSE,
	                 ANSI_CHARSET,OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,
	                 PROOF_QUALITY,DEFAULT_PITCH|FF_DONTCARE,fname);
	oldfont=SelectObject(hDC,hFont);
	SetTextCharacterExtra(hDC,space);
	SetTextJustification(hDC,0,0);
	SetTextAlign(hDC,TA_BASELINE|TA_CENTER);
	GetOutlineTextMetrics(hDC,sizeof(OUTLINETEXTMETRIC),&otm);
	if(org) rad=radius+otm.otmMacAscent; else rad=radius;
	if(rad==0) rad=1;
	if(align==3) {
		conta=angle-(width>>1); contb=width;
	}else{
		GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
		contb=((double)size.cx/(double)rad)*180/3.1415926535;
		if(align==0) conta=angle-contb;
		if(align==1) conta=angle-(contb>>1);
		if(align==2) conta=angle;
	}
	if(org) conta=conta-360;
	SelectObject(hDC,oldfont);
	DeleteObject(hFont);
	if(hdc) DeleteDC(hDC);
}

HWND CircleText::starteditor(HWND hMain)
{
	hMainWnd=hMain;
	hWnd=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_CTEXTEDITOR),hMainWnd,
	                       (DLGPROC)CTextDlgProc,(LPARAM)this);
	if(hWnd==NULL) MessageBox(hMain,"Cannot create dialog",
		                        "ctext.cpp: CircleText::starteditor()",MB_OK);
	return hWnd;
}
BOOL CircleText::endeditor(void)
{
	if(!hWnd) return TRUE;
	if(isChanged()) update();
	DestroyWindow(hWnd);
	hWnd=NULL;
	return TRUE;
}
void CircleText::update(void)
{
	calccontour(NULL);
	SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
void CircleText::initdialog(HWND hDlg)
{
	char txt[256];
	gcvt((float)offx/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERX,txt);
	gcvt((float)offy/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERY,txt);
	SetDlgItemText(hDlg,IDC_TEXT,text);
	int st=0;
	if(italic) st|=2; if(bold) st|=1; if(underline) st|=4;
	SendDlgItemMessage(hDlg,IDC_STYLE,WM_USER,st,0);
	SendDlgItemMessage(hDlg,IDC_ALIGN,WM_USER,align,0);
	gcvt((double)h/10,3,txt); SetDlgItemText(hDlg,IDC_SIZEMM,txt);
	gcvt((double)width,3,txt); SetDlgItemText(hDlg,IDC_WIDTH,txt);
	gcvt((double)space/10,3,txt); SetDlgItemText(hDlg,IDC_SPACING,txt);
	gcvt((double)angle,3,txt); SetDlgItemText(hDlg,IDC_ANGLE,txt);
	gcvt((double)radius/10,3,txt); SetDlgItemText(hDlg,IDC_RADIUS,txt);
	CheckDlgButton(hDlg,IDC_UPDOWN,org ? 1 : 0);
	if(bthick) {
		CheckDlgButton(hDlg,IDC_THICKNESSCHECK,1);
		EnableWindow(GetDlgItem(hDlg,IDC_THICKNESS),TRUE);
		CheckDlgButton(hDlg,IDC_THICKNESSCHECK,1);
	}else{
		CheckDlgButton(hDlg,IDC_THICKNESSCHECK,0);
		EnableWindow(GetDlgItem(hDlg,IDC_THICKNESS),FALSE);
	}
	EnableWindow(GetDlgItem(hDlg,IDC_WIDTH),align==3 ? TRUE : FALSE);
	gcvt((double)thick,3,txt); SetDlgItemText(hDlg,IDC_THICKNESS,txt);
	gcvt((double)h*.3527777777,3,txt); SetDlgItemText(hDlg,IDC_SIZEPT,txt);
	calccontour(NULL);
}
BOOL CALLBACK CTextDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static BOOL mychange=FALSE;
	static BOOL showed=FALSE;
	CircleText* lt=(CircleText*)GetWindowLong(hDlg,GWL_USERDATA);
	int ox,oy,t,w,h;
	RECT rect;
	BOOL success;
	char txt[1024];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,GWL_USERDATA,lParam);
		mychange=TRUE;
		lt=(CircleText*)lParam;
		lt->initdialog(hDlg);
		lt->updated();
		mychange=FALSE;
		FillFontCombobox(lt->getfname(),GetDlgItem(hDlg,IDC_FONT));
		GetWindowRect(GetDlgItem(hDlg,IDC_TEXT),&rect);
		MoveFontViewer(rect.left,rect.top-30);
		showed=FALSE;
		break;
	case WM_SETCURSOR:
		if(lt)
			SendMessage(lt->getmainwnd(),WM_SETSTATUSTEXT,IDS_EDITORTEXT,0);
		return DefWindowProc(hDlg,uMsg,wParam,lParam);
	case WM_MOVEOBJECT:
		mychange=TRUE;
		lt->getoffset(ox,oy);
		if(wParam==0) {
			ox+=(int)((short)LOWORD(lParam)); oy+=(int)((short)HIWORD(lParam));
		}
		gcvt((float)ox/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERX,txt);
		gcvt((float)oy/10,3,txt); SetDlgItemText(hDlg,IDC_CENTERY,txt);
		lt->updated();
		mychange=FALSE;
		break;
	case WM_COMMAND: {
		switch(HIWORD(wParam)) {
		case CBN_DROPDOWN: showed=TRUE;
			SetFontViewer(lt->gettext(),lt->getfname(),lt->getstyle());
			ShowFontViewer();
			break;
		case CBN_SELENDOK: {
			int cursel=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
			SendMessage((HWND)lParam,CB_GETLBTEXT,cursel,(LPARAM)txt);
			SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
			lt->setfname(txt); lt->calccontour(NULL);
			SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
			lt->changed();
		} break;
		case CBN_CLOSEUP: showed=FALSE;
			HideFontViewer();
			break;
		case CBN_SELCHANGE:
			if(showed) {
				int cursel=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
				SendMessage((HWND)lParam,CB_GETLBTEXT,cursel,(LPARAM)txt);
				SetFontViewer(lt->gettext(),txt,lt->getstyle());
			}
			break;
		case 0x1234:
			SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
			lt->setstyle(lParam);
			lt->changed(); lt->calccontour(NULL);
			SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
			break;
		case 0x1235:
			SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
			lt->setalign(lParam);
			EnableWindow(GetDlgItem(hDlg,IDC_WIDTH),lParam==3 ? TRUE : FALSE);
			lt->changed(); lt->calccontour(NULL);
			SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
			break;
		case EN_CHANGE: if(mychange==FALSE)
				switch(LOWORD(wParam)) {
				case IDC_CENTERX:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					lt->getoffset(ox,oy);
					GetDlgItemText(hDlg,IDC_CENTERX,txt,32); ox=atof(txt)*10;
					lt->setoffset(ox,oy); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					lt->changed();
					break;
				case IDC_CENTERY:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					lt->getoffset(ox,oy);
					GetDlgItemText(hDlg,IDC_CENTERY,txt,32); oy=atof(txt)*10;
					lt->setoffset(ox,oy); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					lt->changed();
					break;
				case IDC_RADIUS:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					GetDlgItemText(hDlg,IDC_RADIUS,txt,32); ox=atof(txt)*10;
					lt->setradius(ox); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					lt->changed();
					break;
				case IDC_TEXT:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					GetDlgItemText(hDlg,IDC_TEXT,txt,1000); lt->settext(txt);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					break;
				case IDC_SIZEMM:
					if(!mychange) {
						SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
						GetDlgItemText(hDlg,IDC_SIZEMM,txt,256); h=atof(txt)*10.0;
						lt->setheight(h);
						lt->changed(); lt->calccontour(NULL);
						SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
						mychange=TRUE;
						h=(int)((double)h/.3527777777);
						gcvt((double)h/10,3,txt);
						SetDlgItemText(hDlg,IDC_SIZEPT,txt);
						mychange=FALSE;
					}
					break;
				case IDC_SIZEPT:
					if(!mychange) {
						SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
						GetDlgItemText(hDlg,IDC_SIZEPT,txt,256); h=atof(txt)*10.0;
						h=(int)((double)h*.35277777777);
						lt->setheight(h);
						lt->changed(); lt->calccontour(NULL);
						SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
						mychange=TRUE;
						gcvt((double)h/10,3,txt);
						SetDlgItemText(hDlg,IDC_SIZEMM,txt);
						mychange=FALSE;
					}
					break;
				case IDC_THICKNESS:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					t=GetDlgItemInt(hDlg,IDC_THICKNESS,&success,TRUE);
					lt->setthick(t);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					break;
				case IDC_WIDTH:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					GetDlgItemText(hDlg,IDC_WIDTH,txt,256); w=atof(txt);
					lt->setwidth(w);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					break;
				case IDC_SPACING:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					GetDlgItemText(hDlg,IDC_SPACING,txt,256); w=atof(txt)*10.0;
					lt->setspace(w);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					break;
				case IDC_ANGLE:
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					GetDlgItemText(hDlg,IDC_ANGLE,txt,256); w=atof(txt);
					lt->setangle(w);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					break;
				}   // switch(LOWORD(wParam))
			break;    // case EN_CHANGE
		default:
			switch(LOWORD(wParam)) {
			case IDC_UPDOWN:
				if(SendDlgItemMessage(hDlg,IDC_UPDOWN,BM_GETSTATE,0,0)&3) {
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					lt->setorg(TRUE);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				}else{
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					lt->setorg(FALSE);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
				}
				break;
			case IDC_THICKNESSCHECK:
				if(SendDlgItemMessage(hDlg,IDC_THICKNESSCHECK,BM_GETSTATE,0,0)&3) {
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					t=GetDlgItemInt(hDlg,IDC_THICKNESS,&success,TRUE);
					lt->setbthick(TRUE);
					lt->setthick(t);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					EnableWindow(GetDlgItem(hDlg,IDC_THICKNESS),TRUE);
				}else{
					SendMessage(lt->getmainwnd(),WM_EDITORHIDECONTOUR,0,0);
					lt->setbthick(FALSE);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					EnableWindow(GetDlgItem(hDlg,IDC_THICKNESS),FALSE);
				}
				break;
			case IDOK:
				mychange=TRUE;
				GetDlgItemText(hDlg,IDC_CENTERX,txt,32); ox=atof(txt)*10;
				GetDlgItemText(hDlg,IDC_CENTERY,txt,32); oy=atof(txt)*10;
				GetDlgItemText(hDlg,IDC_TEXT,txt,1000); lt->settext(txt);
				lt->setoffset(ox,oy);
				lt->update();
				mychange=FALSE;
				break;
			case IDCANCEL:
				lt->close();
				break;
			case IDC_DELETE:
				lt->del();
				break;
			}    // switch(LOWORD(wParam))
			break;     // default
		}   // switch(HIWORD(wParam))
	}  // case WM_COMMAND
		               break;
	default:
		return FALSE;
	}
	return TRUE;
}

