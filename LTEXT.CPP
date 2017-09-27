//File:ltext.cpp//

#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include "stmp.rh"
#include "list.hpp"
#include "stamp.hpp"
#include "ltext.hpp"
#include "fontlist.hpp"

BOOL CALLBACK LTextDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
LineText::LineText() : text(NULL) {
	settype(0);
	setgroup(1);
	strcpy(fname,"Arial Cyr");
	align=1; h=32; width=490;
	space=0; offx=offy=0;
	bold=FALSE; italic=FALSE; underline=FALSE;
	angle=0; thick=0; bthick=FALSE;
	forprint=0;
	calccontour(NULL);
}
LineText::~LineText()
{
	if(text) delete text;
}
void LineText::settext(char* t)
{
	if(text) delete text;
	if(!t) return;
	text=new char[lstrlen(t)+1];
	if(text) strcpy(text,t);
}
BOOL LineText::save(HANDLE hFile)
{
	short length=0,t,g;
	DWORD dwWritten;
	length=sizeof(t)+sizeof(g)+sizeof(fname)+sizeof(offx)+sizeof(offy)+sizeof(h)+
	        sizeof(angle)+sizeof(align)+sizeof(bold)+sizeof(italic)+sizeof(underline)+
	        sizeof(width)+sizeof(space)+sizeof(bthick)+sizeof(thick)+sizeof(length)+lstrlen(text);
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
BOOL LineText::load(HANDLE hFile)
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
void LineText::print(HDC hDC)
{
	forprint=1;
	draw(hDC);
	forprint=0;
}
void LineText::draw(HDC hDC)
{
	if(!text) return;
	OUTLINETEXTMETRIC otm;
	TEXTMETRIC tm;
	SIZE size;
	HFONT hFont,oldfont;
	if(forprint) angle=-angle;
	hFont=CreateFont(h,bthick ? thick : 0,angle,0,bold ? 700 : 400,
	                 italic,underline,FALSE,
	                 ANSI_CHARSET,
	                 OUT_CHARACTER_PRECIS,
	                 CLIP_CHARACTER_PRECIS,
	                 PROOF_QUALITY,
	                 DEFAULT_PITCH|FF_DONTCARE,
	                 fname);
	oldfont=SelectObject(hDC,hFont);
	SetTextCharacterExtra(hDC,space);
	SetTextJustification(hDC,0,0);
	int ta=TA_LEFT;
	switch(align) {
	case 1: ta=TA_CENTER; break;
	case 2: ta=TA_RIGHT; break;
	}
	SetTextAlign(hDC,TA_BASELINE|ta);
	GetOutlineTextMetrics(hDC,sizeof(OUTLINETEXTMETRIC),&otm);
//  cont.top=offy+(otm.otmTextMetrics.tmHeight+otm.otmMacDescent);
	cont.bottom=offy+otm.otmMacDescent+1;
	cont.top=cont.bottom+otm.otmTextMetrics.tmHeight;
	if(align==3) {
		for(int i=0,bc=0; i<lstrlen(text); i++) if(text[i]==otm.otmTextMetrics.tmBreakChar) bc++;
		GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
		SetTextJustification(hDC,bc>0 ? width-size.cx+space : 0,bc);
	}
//  SetTextCharacterExtra(hDC,space);
	if(!text) size.cx=0; else GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
	size.cx-=space;
	switch(align) {
	case 1: cont.left=offx-((size.cx+1)>>1)-1; cont.right=cont.left+size.cx+2; break;
	case 2: cont.left=offx-size.cx-1; cont.right=offx+1; break;
	default: cont.left=offx-1; cont.right=offx+size.cx+1; break;
	}
	TextOut(hDC,offx,offy,text,lstrlen(text));
	SelectObject(hDC,oldfont);
	DeleteObject(hFont);
	cont.top=cont.top+1; cont.bottom=cont.bottom-1;
	double a=(double)angle/1800*(-3.141592653585);
	double x1,x2,y1,y2;
	contdescent.left=offx+(cont.left-offx)*cos(a)-(cont.bottom-offy-otm.otmMacDescent-1)*sin(a);
	contdescent.right=offx+(cont.right-offx)*cos(a)-(cont.bottom-offy-otm.otmMacDescent-1)*sin(a);
	contdescent.top=offy+(cont.left-offx)*sin(a)+(cont.bottom-offy-otm.otmMacDescent-1)*cos(a);
	contdescent.bottom=offy+(cont.right-offx)*sin(a)+(cont.bottom-offy-otm.otmMacDescent-1)*cos(a);
	x1=offx+(cont.left-offx)*cos(a)-(cont.top-offy)*sin(a);
	x2=offx+(cont.right-offx)*cos(a)-(cont.bottom-offy)*sin(a);
	y1=offy+(cont.left-offx)*sin(a)+(cont.top-offy)*cos(a);
	y2=offy+(cont.right-offx)*sin(a)+(cont.bottom-offy)*cos(a);
	cont.left=min(x1,x2); cont.top=max(y1,y2);
	cont.right=max(x1,x2); cont.bottom=min(y1,y2);
	if(forprint) angle=-angle;
}

BOOL LineText::hittest(int x,int y)
{
	if(x>=cont.left&&y>=cont.bottom&&x<=cont.right&&y<=cont.top) return TRUE;
	return FALSE;
}
void LineText::drawcontour(HDC hDC)
{
	HBRUSH oldbr=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	HPEN oldpen,hPen;
	hPen=CreatePen(PS_DOT,0,RGB(0,255,255));
	oldpen=SelectObject(hDC,hPen);
	Rectangle(hDC,cont.left,cont.top,cont.right,cont.bottom);
	MoveToEx(hDC,contdescent.left,contdescent.top,NULL);
	LineTo(hDC,contdescent.right,contdescent.bottom);
	SelectObject(hDC,oldpen);
	SelectObject(hDC,oldbr);
	DeleteObject(hPen);
}
void LineText::calcrect(RECT*r,HDC hDC)
{
	calccontour(hDC);
	CopyMemory(r,&cont,sizeof(RECT));
}
void LineText::calccontour(HDC hDC)
{
	SIZE size;
	int x1,y1,x2,y2,ta=TA_LEFT;
	HFONT hFont,oldfont;
	OUTLINETEXTMETRIC otm;
	BOOL hdc;
	if(!text) return;
	if(hDC==NULL) {
		hDC=CreateCompatibleDC(NULL);
		hdc=TRUE;
	}else hdc=FALSE;
	hFont=CreateFont(h,bthick ? thick : 0,angle,0,bold ? 700 : 400,italic,underline,FALSE,
	                 ANSI_CHARSET,OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,
	                 PROOF_QUALITY,DEFAULT_PITCH|FF_DONTCARE,fname);
	oldfont=SelectObject(hDC,hFont);
	SetTextCharacterExtra(hDC,space);
	SetTextJustification(hDC,0,0);
	switch(align) {
	case 1: ta=TA_CENTER; break;
	case 2: ta=TA_RIGHT; break;
	}
	SetTextAlign(hDC,TA_BASELINE|ta);
	GetOutlineTextMetrics(hDC,sizeof(OUTLINETEXTMETRIC),&otm);
	y2=offy+otm.otmMacDescent+1;
	y1=y2+otm.otmTextMetrics.tmHeight;
	if(align==3) {
		for(int i=0,bc=0; i<lstrlen(text); i++) if(text[i]==otm.otmTextMetrics.tmBreakChar) bc++;
		GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
		SetTextJustification(hDC,bc>0 ? width-size.cx+space : 0,bc);
	}
	if(!text) size.cx=0; else GetTextExtentPoint32(hDC,text,lstrlen(text),&size);
	size.cx-=space;
	switch(align) {
	case 1: x1=offx-((size.cx+1)>>1); x2=x1+size.cx; break;
	case 2: x1=offx-size.cx; x2=offx; break;
	default: x1=offx; x2=offx+size.cx; break;
	}
	cont.left=x1-1; cont.right=x2+1; cont.top=y1+1; cont.bottom=y2-1;
	SelectObject(hDC,oldfont);
	DeleteObject(hFont);
	if(hdc) DeleteDC(hDC);
	double a=(double)angle/1800*(-3.141592653585);
	double xx1,xx2,yy1,yy2;
	contdescent.left=offx+(cont.left-offx)*cos(a)-(cont.bottom-offy-otm.otmMacDescent-1)*sin(a);
	contdescent.right=offx+(cont.right-offx)*cos(a)-(cont.bottom-offy-otm.otmMacDescent-1)*sin(a);
	contdescent.top=offy+(cont.left-offx)*sin(a)+(cont.bottom-offy-otm.otmMacDescent-1)*cos(a);
	contdescent.bottom=offy+(cont.right-offx)*sin(a)+(cont.bottom-offy-otm.otmMacDescent-1)*cos(a);
	xx1=offx+(cont.left-offx)*cos(a)-(cont.top-offy)*sin(a);
	xx2=offx+(cont.right-offx)*cos(a)-(cont.bottom-offy)*sin(a);
	yy1=offy+(cont.left-offx)*sin(a)+(cont.top-offy)*cos(a);
	yy2=offy+(cont.right-offx)*sin(a)+(cont.bottom-offy)*cos(a);
	cont.left=min(xx1,xx2); cont.top=max(yy1,yy2);
	cont.right=max(xx1,xx2); cont.bottom=min(yy1,yy2);
}

HWND LineText::starteditor(HWND hMain)
{
	hMainWnd=hMain;
	hWnd=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_LTEXTEDITOR),hMainWnd,(DLGPROC)LTextDlgProc,(LPARAM)this);
	if(hWnd==NULL) MessageBox(hMain,"Cannot create dialog","ltext.cpp: GeomRect::starteditor()",MB_OK);
	return hWnd;
}
BOOL LineText::endeditor(void)
{
	if(!hWnd) return TRUE;
	if(isChanged()) update();
	DestroyWindow(hWnd);
	hWnd=NULL;
	return TRUE;
}
void LineText::update(void)
{
	calccontour(NULL);
	SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
void LineText::initdialog(HWND hDlg)
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
	gcvt((double)width/10,3,txt); SetDlgItemText(hDlg,IDC_WIDTH,txt);
	gcvt((double)space/10,3,txt); SetDlgItemText(hDlg,IDC_SPACING,txt);
	gcvt((double)angle/10,3,txt); SetDlgItemText(hDlg,IDC_ANGLE,txt);
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
BOOL CALLBACK LTextDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static BOOL mychange=FALSE;
	static BOOL showed=FALSE;
	LineText* lt=(LineText*)GetWindowLong(hDlg,GWL_USERDATA);
	int ox,oy,t,w,h;
	RECT rect;
	BOOL success;
	char txt[1024];
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg,GWL_USERDATA,lParam);
		mychange=TRUE;
		lt=(LineText*)lParam;
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
					GetDlgItemText(hDlg,IDC_WIDTH,txt,256); w=atof(txt)*10.0;
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
					GetDlgItemText(hDlg,IDC_ANGLE,txt,256); w=atof(txt)*10.0;
					lt->setangle(w);
					lt->changed(); lt->calccontour(NULL);
					SendMessage(lt->getmainwnd(),WM_EDITORSHOWCONTOUR,0,0);
					break;
				}   // switch(LOWORD(wParam))
			break;    // case EN_CHANGE
		default:
			switch(LOWORD(wParam)) {
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

