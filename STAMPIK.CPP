//File:stampi.cpp//

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "stmp.rh"

#include "about.hpp"
#include "list.hpp"
#include "stamp.hpp"
#include "ltext.hpp"
#include "ctext.hpp"
#include "ttext.hpp"
#include "rect.hpp"
#include "circle.hpp"
#include "line.hpp"
#include "picbmp.hpp"
#include "triangle.hpp"
#include "preview.hpp"
#include "ctree.hpp"
#include "tree.hpp"
#include "panel.hpp"
#include "fontlist.hpp"

HINSTANCE hInst;
HWND WndMain,WndViewer;
HWND WndStatusLine;
HWND WndEdit;
HBITMAP hMainBitmaps;
HICON hMainIcon;
HMENU RecentMenu;
HCURSOR CrossCursor;
char StatusLineText1[128]; int StatusLineID1;
char StatusLineText2[32]; int StatusLineID2;
char StatusLineText3[64]; int StatusLineID3;
static char groupinsert=0;
char *lastfiles[10]; int numlastfiles=0;

WORD StatusText1=0;
StampNode* edited=NULL;
Stamp* stamp;
Preview* preview;
ControlPanel* panel;
LRESULT CALLBACK MainWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK ButtonWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK StatusLineWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK StampConfigProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK TextAlignWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK TextStyleWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

inline int UpdateStatusLine(int ID)
{
	return SendMessage(WndStatusLine,WM_USER+3,ID,0l);
}
inline int InsertStatusLine(int size,BOOL down,char* txt){
	return SendMessage(WndStatusLine,WM_USER,(WPARAM)MAKELONG(size,down),(LPARAM)txt);
}

int ErrorMessage(int msg,char *title,int btn)
{
	char buf[128];
	if(LoadString(hInst,msg,buf,128)>0)
		return MessageBox(WndMain,buf,title,btn);
	else
		return MessageBox(WndMain,"Unknown error",title,btn|MB_ICONSTOP);
}
void loadlastfiles(void)
{
	int i,k;
	char buf[256],key[64];
	if(numlastfiles) {
		for(i=0; i<numlastfiles; i++)
			if(lastfiles[i]) delete lastfiles[i];
	}
	k=0; numlastfiles=GetProfileInt("Stampik","NumberOfLastOpenedFiles",0);
	if(numlastfiles<1) return;
	if(numlastfiles>10) numlastfiles=10;
	for(i=0; i<numlastfiles; i++) {
		wsprintf(key,"last%d",i);
		GetProfileString("Stampik",key,"",buf,256);
		if(buf[0]!=0) {
			lastfiles[i]=new char[lstrlen(buf)+1];
			strcpy(lastfiles[i],buf);
			k++;
		}
	}
	numlastfiles=k;
}
void savelastfiles(void)
{
	char buf[64];
	wsprintf(buf,"NumberOfLastOpenedFiles=%d%c%c",numlastfiles,0,0);
	WriteProfileSection("Stampik",buf);
	for(int i=0; i<numlastfiles; i++) {
		wsprintf(buf,"last%d",i);
		WriteProfileString("Stampik",buf,lastfiles[i]);
	}
}
void addlastfile(char*add)
{
	int i,j;
	char buf[256];
	if(numlastfiles<10) {
		for(i=numlastfiles; i>0; i--) lastfiles[i]=lastfiles[i-1];
		lastfiles[0]=new char[lstrlen(add)+1];
		strcpy(lastfiles[0],add);
		numlastfiles++;
	}else{
		delete lastfiles[9];
		for(i=9; i>0; i--) lastfiles[i]=lastfiles[i-1];
		lastfiles[0]=new char[lstrlen(add)+1];
		strcpy(lastfiles[0],add);
	}
	HMENU FilesMenu=GetSubMenu(GetMenu(WndMain),0);
	j=GetMenuItemCount(RecentMenu);
	for(i=0; i<j; i++)
		RemoveMenu(RecentMenu,0,MF_BYPOSITION);
	if(numlastfiles==0)
		AppendMenu(RecentMenu,MF_GRAYED,CM_LAST1,"Empty");
	else
		for(i=0; i<numlastfiles; i++) {
			if(i<9) wsprintf(buf,"&%d %s",i+1,lastfiles[i]);
			else wsprintf(buf,"1&0 %s",lastfiles[i]);
			AppendMenu(RecentMenu,0,CM_LAST1+i,buf);
		}
}
BOOL InitApp(HINSTANCE hInstance){
	WNDCLASS wc;
	hInst=hInstance;
	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc=MainWndProc;
	wc.cbClsExtra=wc.cbWndExtra=0;
	wc.hInstance=hInst;
	hMainBitmaps=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TREEBITMAPS));
	hMainIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAINICON));
	CrossCursor=LoadCursor(hInst,MAKEINTRESOURCE(PREVIEW_CURSOR));
	wc.hIcon=hMainIcon;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName=MAKEINTRESOURCE(MAINMENU);
	wc.lpszClassName="StampMainWnd";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS1,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.lpfnWndProc=PreviewWndProc;
	wc.hCursor=CrossCursor;//LoadCursor(NULL,IDC_ARROW);//CrossCursor;
	wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="StampPreviewWnd";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS2,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	wc.lpfnWndProc=FontViewerProc;
	wc.lpszClassName="stampfontviewer";
	if(!RegisterClass(&wc)) {
		MessageBox(NULL,"Cannot register class 'stampfontviewer'","Error in stamp.cpp",MB_ICONASTERISK|MB_OK);
		return FALSE;
	}
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.lpfnWndProc=RulerWndProc;
	wc.lpszClassName="StampRulerWnd";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS6,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.lpfnWndProc=DlgSmallPreviewWnd;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="StampDlgPreviewWnd";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS3,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.style|=CS_DBLCLKS;
	wc.lpfnWndProc=TreeWndProc;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="StampTreeWnd";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS5,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.lpfnWndProc=TreeHelpikWndProc;
	wc.hbrBackground=GetStockObject(WHITE_BRUSH);
	wc.lpszClassName="StampTipsWnd";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS8,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.style^=CS_DBLCLKS;
	wc.lpfnWndProc=ButtonWndProc;
	wc.hbrBackground=GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName="StampBtn";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS4,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.lpfnWndProc=StatusLineWndProc;
	wc.lpszClassName="StampStatusLine";
	if(!RegisterClass(&wc)) {
		ErrorMessage(ERRMSG_REGCLASS7,"Error in stampik.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	wc.lpfnWndProc=TextStyleWndProc;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="StampTextStyleWnd";
	if(!RegisterClass(&wc)) {
		MessageBox(NULL,"Cannot register class 'StampTextStyleWnd'","Error in stamp.cpp",MB_ICONASTERISK|MB_OK);
		return FALSE;
	}
	wc.lpfnWndProc=TextAlignWndProc;
	wc.lpszClassName="StampTextAlignWnd";
	if(!RegisterClass(&wc)) {
		MessageBox(NULL,"Cannot register class 'StampTextAlignWnd'","Error in stamp.cpp",MB_ICONASTERISK|MB_OK);
		return FALSE;
	}

	WndMain=CreateWindowEx(WS_EX_DLGMODALFRAME,"StampMainWnd","Stampik 1.02",
	                       WS_OVERLAPPED|WS_DLGFRAME|WS_VISIBLE|
	                       WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|
	                       WS_CLIPCHILDREN,
	                       0,0,
	                       800,600,
	                       NULL,NULL,hInstance,NULL);
	if(!WndMain) {
		ErrorMessage(ERRMSG_CREATEMAINWINDOW,"Error in stamp.cpp",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	WndStatusLine=CreateWindow("StampStatusLine","StatusLine",
	                           WS_CHILD|WS_VISIBLE,
	                           0,536,
	                           794,20,
	                           WndMain,NULL,hInstance,NULL);
	if(!WndStatusLine) {
		ErrorMessage(ERRMSG_CREATESTATUSLINEWINDOW,"Error in stamp.cpp",MB_OK|MB_ICONSTOP);
	}
	StatusLineID1=InsertStatusLine(580,TRUE,StatusLineText1);
	StatusLineID2=InsertStatusLine(135,TRUE,StatusLineText2);
	StatusLineID3=InsertStatusLine(70,TRUE,StatusLineText3);
	RecentMenu=GetSubMenu(GetSubMenu(GetMenu(WndMain),0),5);
	if(numlastfiles>0) RemoveMenu(RecentMenu,0,MF_BYPOSITION);
	char buf[256]; for(int i=0; i<numlastfiles; i++) {
		if(i<9) wsprintf(buf,"&%d %s",i+1,lastfiles[i]);
		else wsprintf(buf,"1&0 %s",lastfiles[i]);
		AppendMenu(RecentMenu,0,CM_LAST1+i,buf);
	}
	WndViewer=CreateFontViewer(hInst,WndMain);
	return TRUE;
}
void StartPrintManager(void){
	PROCESS_INFORMATION pi;
	STARTUPINFO si={
		sizeof(STARTUPINFO),
		NULL,NULL,NULL,
		0,0,800,600,
		0,0,0,
		STARTF_FORCEONFEEDBACK,
		SW_SHOWDEFAULT,
		0,NULL,NULL,NULL,NULL
	};
	if(CreateProcess("printman.exe",NULL,NULL,NULL,FALSE,
	                 0,NULL,NULL,&si,&pi)!=TRUE)
		MessageBox(WndMain,"Cannot run 'printman.exe'","ERROR",MB_OK|MB_ICONASTERISK);
}

#pragma argsused
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpszCmdLine,int nCmdShow)
{
	MSG msg;
	HACCEL hAccel;
	if(hPrevInstance) return 0;
	loadlastfiles();
	if(!InitApp(hInstance)) return 0;
	hAccel=LoadAccelerators(hInstance,MAKEINTRESOURCE(ACCEL));
	while(GetMessage(&msg,NULL,0,0)) {
		TranslateAccelerator(WndMain,hAccel,&msg);
		if(IsDialogMessage(WndMain,&msg)==FALSE) {
			TranslateMessage(&msg);    // Translates virtual key codes.
			DispatchMessage(&msg);     // Dispatches message to window.
		}
	}
	CloseFontViewer();
	int err=0;
	if(!UnregisterClass("stampfontviewer",hInstance)) err=1;
	if(!UnregisterClass("StampTextStyleWnd",hInstance)) err=2;
	if(!UnregisterClass("StampTextAlignWnd",hInstance)) err=3;
	if(!UnregisterClass("StampRulerWnd",hInstance)) err=4;
	if(!UnregisterClass("StampTipsWnd",hInstance)) err=5;
	if(!UnregisterClass("StampStatusLine",hInstance)) err=6;
	if(!UnregisterClass("StampTreeWnd",hInstance)) err=7;
	if(!UnregisterClass("StampBtn",hInstance)) err=8;
	if(!UnregisterClass("StampMainWnd",hInstance)) err=9;
	if(!UnregisterClass("StampPreviewWnd",hInstance)) err=10;
	if(!UnregisterClass("StampDlgPreviewWnd",hInstance)) err=11;
	if(err) {
		char errmsg[256];
		wsprintf(errmsg,"cannot unregister class %d",err);
		MessageBox(NULL,errmsg,NULL,MB_OK);
	}
	DeleteObject(CrossCursor);
	DeleteObject(hMainBitmaps);
	DeleteObject(hMainIcon);
	savelastfiles();
	return msg.wParam;
}
void PreviewInsert(StampNode*sn)
{
	if(sn==NULL) return;
	if(edited!=NULL) {
		if(edited->endeditor()==FALSE) return;
	}else if(WndEdit) DestroyWindow(WndEdit);
	preview->SeekObject(sn);
	WndEdit=sn->starteditor(WndMain);
	if(WndEdit!=NULL) edited=sn; else edited=NULL;
}
void DrawShade(HDC hDC,int x,int y,int x2,int y2,BOOL down)
{
	HPEN pen1,pen2,oldpen;
	pen1=CreatePen(PS_SOLID,0,RGB(138,138,138));
	pen2=CreatePen(PS_SOLID,0,RGB(224,224,224));
	oldpen=SelectObject(hDC,down ? pen2 : pen1);
	MoveToEx(hDC,x+1,y2,NULL); LineTo(hDC,x2,y2); LineTo(hDC,x2,y);
	SelectObject(hDC,down ? pen1 : pen2);
	MoveToEx(hDC,x,y2,NULL); LineTo(hDC,x,y); LineTo(hDC,x2,y);
	SelectObject(hDC,oldpen);
	DeleteObject(pen1);
	DeleteObject(pen2);
}
HWND StartStampConfig(HWND hMain,Stamp*s)
{
	HWND hWnd=CreateDialogParam(hInst,
	                            MAKEINTRESOURCE(IDD_STAMPEDITOR),
	                            hMain,
	                            (DLGPROC)StampConfigProc,
	                            (LPARAM)s);
	if(hWnd==NULL)
		MessageBox(hMain,"Cannot create dialog","stampik.cpp: StartStampConfig()",MB_OK);
}
StampNode* FAR GRectCreator(void)
{
	return new GeomRect;
}
StampNode* FAR PBitmapCreator(void)
{
	return new PicBitmap;
}
StampNode* FAR GCircleCreator(void)
{
	return new GeomCircle;
}
StampNode* FAR GLineCreator(void)
{
	return new GeomLine;
}
StampNode* FAR GTriangleCreator(void)
{
	return new GeomTriangle;
}
LRESULT CALLBACK MainWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	LineText* lt;
	CircleText* ct;
	TriangleText* tt;
	GeomRect* gr;
	GeomLine* gl;
	GeomCircle* gc;
	GeomTriangle* gt;
	PicBitmap* pb;
	switch(uMsg) {
	case WM_CREATE:
		WndMain=hWnd;
		stamp=new Stamp("Super stampik",hWnd);
		stamp->RegisterClass(0,-1,"PICTURES",hMainBitmaps,64,0);
		stamp->RegisterClass(1,-1,"TEXT",hMainBitmaps,48,0);
		stamp->RegisterClass(2,-1,"GEOMETRY",hMainBitmaps,32,0);
		stamp->RegisterClass(-1,0,"Texts on line",hMainBitmaps,160,0);
		stamp->RegisterClass(-1,1,"Texts on circle",hMainBitmaps,144,0);
		stamp->RegisterClass(-1,2,"Texts on triangle",hMainBitmaps,176,0);
		stamp->RegisterClass(-1,5,"Rectangles",hMainBitmaps,96,0,GRectCreator);
		stamp->RegisterClass(-1,3,"Lines",hMainBitmaps,112,0,GLineCreator);
		stamp->RegisterClass(-1,4,"Circles",hMainBitmaps,80,0,GCircleCreator);
		stamp->RegisterClass(-1,6,"Triangles",hMainBitmaps,128,0,GTriangleCreator);
		stamp->RegisterClass(-1,7,"Pictures(BITMAP)",hMainBitmaps,80,0,PBitmapCreator);
		panel=new ControlPanel(hInst,hWnd,stamp);
		preview=new Preview(hInst,hWnd,stamp);
		preview->Selection(NULL);
		strcpy(StatusLineText1,"Ready"); UpdateStatusLine(StatusLineID1);
		strcpy(StatusLineText3,"Not changed"); UpdateStatusLine(StatusLineID3);
		preview->update();
		break;
	case WM_STAMPINSERTED:
		stamp->change();
		strcpy(StatusLineText3,"Modified"); UpdateStatusLine(StatusLineID3);
		break;
	case WM_STAMPREMOVED:
		if(edited==(StampNode*)lParam) {edited=NULL; DestroyWindow(WndEdit); WndEdit=NULL;}
		if(preview->GetSelection()==(StampNode*)lParam) preview->Selection(NULL);
		if(!groupinsert) {
			panel->removeobject((StampNode*)lParam);
			preview->update();
		}
		stamp->change();
		strcpy(StatusLineText3,"Modified"); UpdateStatusLine(StatusLineID3);
		break;
	case WM_PREVIEWMOVECONTOUR:
		if(lParam==0) return 0;
		((StampNode*)lParam)->drag((int)((short)LOWORD(wParam)),(int)((short)HIWORD(wParam)));
		break;
	case WM_PREVIEWMOVEOBJECT:
		if(lParam==0) return 0;
		((StampNode*)lParam)->move((int)((short)LOWORD(wParam)),(int)((short)HIWORD(wParam)));
		stamp->change();
		strcpy(StatusLineText3,"Modified"); UpdateStatusLine(StatusLineID3);
		preview->update();
		break;
	case WM_PREVIEWSELECTED:
		if(lParam==0l) MessageBox(hWnd,"wrong choice","stampik.cpp",MB_OK);
		if(edited!=NULL) {
			if(edited->endeditor()==FALSE) return -1;
		}else if(WndEdit) DestroyWindow(WndEdit);
		WndEdit=((StampNode*)lParam)->starteditor(hWnd);
		if(WndEdit!=NULL) edited=(StampNode*)lParam; else edited=NULL;
		panel->selectobject((StampNode*)lParam);
		return 0;
	case WM_EDITORDELETEOBJECT:
		if(lParam==0) return 0;
		preview->Selection(NULL);
		stamp->remove((StampNode*)lParam);
		break;
	case WM_EDITORUPDATE:
		panel->redraw();
		stamp->change();
		strcpy(StatusLineText3,"Modified"); UpdateStatusLine(StatusLineID3);
		preview->update();
		break;
	case WM_EDITORCLOSE:
		if(edited!=NULL) {
			preview->hidecontour();
			DestroyWindow(WndEdit);
			edited=NULL;
			WndEdit=NULL;
			preview->Selection(NULL);
		}else if(WndEdit) {
			panel->redraw();
			DestroyWindow(WndEdit);
		}
		break;
	case WM_EDITORHIDECONTOUR:
		preview->hidecontour();
		break;
	case WM_EDITORSHOWCONTOUR:
		preview->showcontour();
		break;
	case WM_PANELMENU:
		switch(wParam) {
		case 2: case 0: break;
		case 1:
			if(edited!=NULL) {
				preview->SeekObject(NULL);
				if(edited->endeditor()==FALSE) return 0;
			}else if(WndEdit) DestroyWindow(WndEdit);
			WndEdit=StartStampConfig(hWnd,stamp);
			edited=NULL;
			break;
		}
		break;
	case WM_PANELSELECTED: {
		if(lParam==0l) MessageBox(hWnd,"wrong choice","stampik.cpp",MB_OK);
		if(edited!=NULL) {
			if(edited->endeditor()==FALSE) return 0;
		}else if(WndEdit) DestroyWindow(WndEdit);
		preview->SeekObject((StampNode*)lParam);
		WndEdit=((StampNode*)lParam)->starteditor(hWnd);
		if(WndEdit!=NULL) edited=(StampNode*)lParam; else edited=NULL;
	} break;
	case WM_PANELINSERTED: {
		int x,y;
		preview->GetPosition(x,y);
		switch(wParam) {
		case 0:
			gl=new GeomLine(-100,0,100,0,5); stamp->insert(gl);
			panel->update();
			panel->selectobject(gl);
			PreviewInsert(gl);
			preview->update();
			break;
		case 7:
			pb=new PicBitmap(0,0,100,NULL); stamp->insert(pb);
			panel->update();
			panel->selectobject(pb);
			PreviewInsert(pb);
			preview->update();
			break;
		case 2:
			gt=new GeomTriangle(0,0,100,5); stamp->insert(gt);
			panel->update();
			panel->selectobject(gt);
			PreviewInsert(gt);
			preview->update();
			break;
		case 6: MessageBeep(MB_OK); break;
		case 1:
			gc=new GeomCircle(x,y,250,5); stamp->insert(gc);
			panel->update();
			panel->selectobject(gc);
			PreviewInsert(gc);
			preview->update();
			break;
		case 3:
			gr=new GeomRect(x,y,500,200,7); stamp->insert(gr);
			panel->update();
			panel->selectobject(gr);
			PreviewInsert(gr);
			preview->update();
			break;
		case 4:
			lt=new LineText; lt->settext("text on line"); stamp->insert(lt);
			lt->setoffset(x,y);
			panel->update();
			panel->selectobject(lt);
			PreviewInsert(lt);
			preview->update();
			break;
		case 5:
			ct=new CircleText; ct->settext("circle text"); stamp->insert(ct);
			panel->update();
			panel->selectobject(ct);
			PreviewInsert(ct);
			preview->update();
			break;
/*       case 6:
         tt=new TriangleText; tt->settext("text on triangle"); stamp->insert(tt);
         panel->selectobject(tt);
         PreviewInsert(tt);
         preview->update();
         break;*/
		}
	} break;
	case WM_SETCURSOR:
		if((HWND)wParam==hWnd) {
			if(HIWORD(lParam)==0) {
				if(StatusText1!=1) {
					StatusText1=1;
					strcpy(StatusLineText1,"Choose the necessary item of the menu and press Enter.");
					UpdateStatusLine(StatusLineID1);
				}
			}else
			if(StatusText1!=0) {
				StatusText1=0;
				strcpy(StatusLineText1,"Ready."); UpdateStatusLine(StatusLineID1);
			}
		}
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	case WM_SETSTATUSTEXT:
		if(wParam!=StatusText1) {
			if(wParam>0) {
				if(LoadString(hInst,wParam,StatusLineText1,128)<=0) {
					strcpy(StatusLineText1,"Ready");
					wParam=0;
				}
				UpdateStatusLine(StatusLineID1);
			}else{
				strcpy(StatusLineText1,"Ready"); UpdateStatusLine(StatusLineID1);
			}
			StatusText1=wParam;
		}
		break;
	case WM_UPDATECOORDS:
		sprintf(StatusLineText2,"%.1f:%.1f (%.1f:%.1f) mm",
		        (float)preview->GetMouseX()/10.0,(float)preview->GetMouseY()/10.0,
		        ((float)((short)LOWORD(wParam)))/10.0,((float)((short)HIWORD(wParam)))/10.0
		        );
		UpdateStatusLine(StatusLineID2);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)>=CM_LAST1&&LOWORD(wParam<CM_LAST1+10)) {
			groupinsert=1;
			if(stamp->open(lastfiles[LOWORD(wParam)-CM_LAST1])==0) {
				panel->update(); preview->update();
				strcpy(StatusLineText3,"Not changed"); UpdateStatusLine(StatusLineID3);
			}
			groupinsert=0;
		}else
			switch(LOWORD(wParam)) {
			case CM_FILENEW:
				groupinsert=1;
				if(stamp->close()==0) {
					panel->update(); preview->update();
					strcpy(StatusLineText3,"Not changed"); UpdateStatusLine(StatusLineID3);
				}
				groupinsert=0;
				break;
			case CM_FILESAVE:
				if(stamp->save()==0) {
					strcpy(StatusLineText3,"Not changed"); UpdateStatusLine(StatusLineID3);
					char*fn=stamp->getfname();
					for(int i=0; i<numlastfiles; i++)
						if(strcmp(lastfiles[i],fn)==0) return 0;
					addlastfile(fn);
				}
				break;
			case CM_FILESAVE_AS:
				if(stamp->saveas()==0) {
					strcpy(StatusLineText3,"Not changed"); UpdateStatusLine(StatusLineID3);
					char*fn=stamp->getfname();
					for(int i=0; i<numlastfiles; i++)
						if(strcmp(lastfiles[i],fn)==0) return 0;
					addlastfile(fn);
				}
				break;
			case CM_FILEPRINT:
				StartPrintManager();
				break;
			case CM_FILEOPEN:
				groupinsert=1;
				if(stamp->open()==0) {
					panel->update(); preview->update();
					strcpy(StatusLineText3,"Not changed"); UpdateStatusLine(StatusLineID3);
					char*fn=stamp->getfname();
					for(int i=0; i<numlastfiles; i++)
						if(strcmp(lastfiles[i],fn)==0) {groupinsert=0; return 0;}
					addlastfile(fn);
				}
				groupinsert=0;
				break;
			case CM_HELPABOUT:
				strcpy(StatusLineText1,"About in progress"); UpdateStatusLine(StatusLineID1);
				DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),WndMain,(DLGPROC)AboutDlgProc);
				strcpy(StatusLineText1,"Ready"); UpdateStatusLine(StatusLineID1);
				break;
			case CM_NEXTOBJECT:
				panel->nextobject();
				break;
			case CM_PREVOBJECT:
				panel->prevobject();
				break;
			case CM_PREVGROUP:
				panel->prevgroup();
				break;
			case CM_NEXTGROUP:
				panel->nextgroup();
				break;
			case CM_ENTER: {
				HWND hF=GetFocus();
				SendMessage(hF,WM_LBUTTONDOWN,0,0);
				SendMessage(hF,WM_LBUTTONUP,0,0);
			} break;
			case CM_FILEEXIT: PostMessage(hWnd,WM_CLOSE,0,0l); break;
			case CM_UPDATE: panel->update(); preview->update(); break;
			case CM_EDITORUPDATE: if(WndEdit) SendMessage(WndEdit,WM_COMMAND,IDOK,0); break;
			case CM_EDITORDELETE: if(WndEdit) SendMessage(WndEdit,WM_COMMAND,IDC_DELETE,0); break;
			case CM_EDITORCLOSE: if(WndEdit) SendMessage(WndEdit,WM_COMMAND,IDCANCEL,0); break;
			case CM_STAMPCHANGE:
				if(edited!=NULL) {
					preview->SeekObject(NULL);
					if(edited->endeditor()==FALSE) return 0;
				}else if(WndEdit) DestroyWindow(WndEdit);
				WndEdit=StartStampConfig(hWnd,stamp);
				edited=NULL;
				break;
			}
		break;
	case WM_CLOSE:
		groupinsert=1;
		if(stamp->close()==0) DestroyWindow(hWnd);
		groupinsert=0;
		break;
	case WM_DESTROY:
		groupinsert=1;
		delete panel;
		delete preview;
		delete stamp;
		groupinsert=0;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0l;
}

typedef struct
{
	HBITMAP hBmp1,hBmp2;
	BOOL captured;
	BOOL down;
} BWPDATA;
LRESULT CALLBACK ButtonWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	BWPDATA* bwpd;
	RECT rect;
	HDC hDC;
	HBITMAP oldbmp;
	HBRUSH oldbr;
	PAINTSTRUCT ps;
	HPEN oldpen,pen;
	BITMAP bmp;
	POINT pt;
	bwpd=(BWPDATA*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_CREATE:
		bwpd=new BWPDATA;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)bwpd);
		bwpd->hBmp1=LoadBitmap((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
		                       MAKEINTRESOURCE((int)GetWindowLong(hWnd,GWL_ID)+1000));
		bwpd->hBmp2=LoadBitmap((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
		                       MAKEINTRESOURCE((int)GetWindowLong(hWnd,GWL_ID)+3000));
		GetObject(bwpd->hBmp1,sizeof(BITMAP),&bmp);
		bwpd->captured=FALSE;
		bwpd->down=FALSE;
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		bwpd->captured=TRUE;
		bwpd->down=TRUE;
		SetFocus(hWnd);
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_MOUSEMOVE:
		if(bwpd->captured) {
			pt.x=LOWORD(lParam); pt.y=HIWORD(lParam);
			GetClientRect(hWnd,&rect);
			if(PtInRect(&rect,pt)) {
				if(bwpd->down==FALSE) {
					bwpd->down=TRUE;
					RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
				}
			}else{
				if(bwpd->down==TRUE) {
					bwpd->down=FALSE;
					RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
				}
			}
		}
		break;
	case WM_KEYDOWN:
		if(wParam==VK_SPACE&&bwpd->down==FALSE) {
			bwpd->down=TRUE;
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		if(wParam==VK_RETURN) {
			bwpd->down=TRUE;
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(GetParent(hWnd),WM_COMMAND,GetWindowLong(hWnd,GWL_ID),0);
			bwpd->down=FALSE;
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		break;
	case WM_KEYUP:
		if(wParam==VK_SPACE&&bwpd->down) {
			bwpd->down=FALSE;
			SendMessage(GetParent(hWnd),WM_COMMAND,GetWindowLong(hWnd,GWL_ID),0);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		break;
	case WM_GETDLGCODE: return DLGC_BUTTON;
	case WM_LBUTTONUP:
		if(bwpd->captured) {
			bwpd->captured=FALSE;
			if(bwpd->down)
				SendMessage(GetParent(hWnd),WM_COMMAND,GetWindowLong(hWnd,GWL_ID),0);
			bwpd->down=FALSE;
			ReleaseCapture();
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		break;
	case WM_SETCURSOR:
		SendMessage(WndMain,WM_SETSTATUSTEXT,IDS_BUTTONSTEXT,0);
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	case WM_KILLFOCUS:
		bwpd->down=FALSE;
	case WM_SETFOCUS:
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rect);
		GetObject(bwpd->hBmp1,sizeof(BITMAP),&bmp);
		oldbr=SelectObject(ps.hdc,GetStockObject(HOLLOW_BRUSH));
		if(GetFocus()==hWnd) oldpen=SelectObject(ps.hdc,GetStockObject(BLACK_PEN));
		else{
			pen=CreatePen(PS_SOLID,1,RGB(192,192,192));
			oldpen=SelectObject(ps.hdc,pen);
		}
		Rectangle(ps.hdc,0,0,bmp.bmWidth+2,bmp.bmHeight+2);
		if(GetFocus()==hWnd) DeleteObject(pen);
		SelectObject(ps.hdc,oldpen);
		SelectObject(ps.hdc,oldbr);
		hDC=CreateCompatibleDC(ps.hdc);
		if(bwpd->down) {
			oldbmp=SelectObject(hDC,bwpd->hBmp2);
			GetObject(bwpd->hBmp2,sizeof(BITMAP),&bmp);
		}else{
			oldbmp=SelectObject(hDC,bwpd->hBmp1);
			GetObject(bwpd->hBmp1,sizeof(BITMAP),&bmp);
		}
		BitBlt(ps.hdc,1,1,bmp.bmWidth,bmp.bmHeight,hDC,0,0,SRCCOPY);
		SelectObject(hDC,oldbmp);
		DeleteDC(hDC);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		DeleteObject(bwpd->hBmp2);
		DeleteObject(bwpd->hBmp1);
		delete bwpd;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
typedef struct tagSLOT
{
	int size;
	int id;
	char* text;
	BOOL down;
	struct tagSLOT FAR*_next;
} SLOT,FAR* LPSLOT;
typedef struct
{
	int ids;
	HFONT hFont;
	LPSLOT slots;
} SLDATA,FAR* LPSLDATA;
LRESULT CALLBACK StatusLineWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	LPSLDATA slData=(LPSLDATA)GetWindowLong(hWnd,GWL_USERDATA);
	LPSLOT p,pp;
	PAINTSTRUCT ps;
	HFONT oldfnt;
	HBRUSH oldbr;
	RECT rect;
	int x;
	switch(uMsg) {
	case WM_CREATE:
		slData=new SLDATA;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)slData);
		slData->slots=NULL;
		slData->ids=0;
		slData->hFont=CreateFont(10,0,0,0,0,
		                         FALSE,FALSE,FALSE,
		                         ANSI_CHARSET,
		                         OUT_DEFAULT_PRECIS,
		                         CLIP_DEFAULT_PRECIS,
		                         DEFAULT_QUALITY,
		                         DEFAULT_PITCH|FF_DONTCARE,
		                         "MS Sans Serif");
		break;
	case WM_USER:   // Insert slot, return slot ID
		if(slData->slots==NULL) p=slData->slots=new SLOT;
		else{
			p=slData->slots;
			while(p->_next) p=p->_next;
			p->_next=new SLOT;
			p=p->_next;
		}
		p->id=slData->ids;
		slData->ids++;
		p->_next=NULL;
		p->size=LOWORD(wParam);
		p->down=HIWORD(wParam);
		p->text=(char*)lParam;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		return p->id;
	case WM_USER+1:   // Remove slot by ID
		if(slData->slots==NULL) return -1;
		pp=p=slData->slots;
		while(p->id!=wParam) {if(!(p->_next)) break; pp=p; p=p->_next;}
		if(p->id!=wParam) return -2;
		if(p==slData->slots) slData->slots=p->_next;
		pp->_next=p->_next;
		delete p;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		return 0;
	case WM_USER+2:   // Set slot data (text,size and UP/DOWN)
		if(slData->slots==NULL) return -1;
		p=slData->slots;
		while(p->id!=wParam) {if(!(p->_next)) break; p=p->_next;}
		if(p->id!=wParam) return -2;
		p->size=LOWORD(wParam);
		p->down=HIWORD(wParam);
		p->text=(char*)lParam;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_USER+3:   // update slot ID
		if(slData->slots==NULL) return -1;
		p=slData->slots; x=2;
		while(p->id!=wParam) {if(!(p->_next)) break; x+=p->size+2; p=p->_next;}
		if(p->id!=wParam) return -2;
		GetClientRect(hWnd,&rect);
		rect.left=x+1; rect.top=3; rect.right=x+p->size-1; rect.bottom-=4;
		RedrawWindow(hWnd,&rect,NULL,RDW_INVALIDATE|RDW_ERASENOW);
		break;
	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rect);
		oldfnt=SelectObject(ps.hdc,slData->hFont);
		oldbr=SelectObject(ps.hdc,GetStockObject(LTGRAY_BRUSH));
		PatBlt(ps.hdc,0,0,rect.right-1,rect.bottom-1,PATCOPY);
		SelectObject(ps.hdc,oldbr);
		SetBkMode(ps.hdc,TRANSPARENT);
		DrawShade(ps.hdc,0,0,rect.right-1,rect.bottom-1,FALSE);
		x=2; p=slData->slots; while(p) {
			DrawShade(ps.hdc,x,2,x+p->size,rect.bottom-3,p->down);
			TextOut(ps.hdc,x+2,3,p->text,lstrlen(p->text));
			x+=p->size+2;
			p=p->_next;
		}
		SelectObject(ps.hdc,oldfnt);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		DeleteObject(slData->hFont);
		p=slData->slots; while(p) {
			pp=p->_next;
			delete p;
			p=pp;
		}
		delete slData;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0l;
}
VOID CALLBACK DottedLineProc(int x,int y,LPARAM lpData)
{
	LPDTLPDATA dtldata=(LPDTLPDATA)lpData;
	dtldata->c++; dtldata->c%=2;
	if(dtldata->c)
		SetPixel(dtldata->hDC,x,y,0);
}

typedef struct {
	char btns[3];
	char pos;
} TEXTSTYLEDATA;

LRESULT CALLBACK TextStyleWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC,hDC2;
	HBITMAP hBmp,oldbmp;
	TEXTSTYLEDATA *tsd;
	HBRUSH oldbrush;
	int i,x,xs,ys;
	BOOL wfocus=FALSE;
	tsd=(TEXTSTYLEDATA*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_CREATE:
		tsd=new TEXTSTYLEDATA;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)tsd);
		tsd->pos=0;
		tsd->btns[0]=tsd->btns[1]=tsd->btns[2]=0;
		RegisterHotKey(hWnd,47,MOD_CONTROL,'B');
		RegisterHotKey(hWnd,48,MOD_CONTROL,'I');
		RegisterHotKey(hWnd,49,MOD_CONTROL,'U');
		break;
	case WM_USER:    //  Set data
		tsd->btns[0]=wParam&1 ? 1 : 0;
		tsd->btns[1]=wParam&2 ? 1 : 0;
		tsd->btns[2]=wParam&4 ? 1 : 0;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_USER+1: {   // Get data
		LONG ret=0;
		if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
		return ret;
	}
	case WM_SETFOCUS:
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_KILLFOCUS:
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_LBUTTONDOWN: {
		int x=LOWORD(lParam),y=HIWORD(lParam);
		LONG ret=0;
		if(y>0&&y<16) {
			SetFocus(hWnd);
			if(x>0&&x<16) { tsd->btns[0]^=1; tsd->pos=0; }
			if(x>15&&x<31) {tsd->pos=1; tsd->btns[1]^=1; }
			if(x>30&&x<46) {tsd->pos=2; tsd->btns[2]^=1; }
			if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1234),ret);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
	} break;
	case WM_HOTKEY:
		switch(wParam) {
		case 44: case 47: tsd->btns[0]^=1; tsd->pos=0; break;
		case 45: case 48: tsd->btns[1]^=1; tsd->pos=1; break;
		case 46: case 49: tsd->btns[2]^=1; tsd->pos=2; break;
		}
		if(wParam>=44&&wParam<=49) {
			LONG ret=0;
			if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1234),ret);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		break;
	case WM_KEYDOWN: {
		LONG ret=0;
		switch((int)wParam) {
		case 'b': case 'B': tsd->btns[0]^=1; tsd->pos=0;
			if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1234),ret);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case 'i': case 'I': tsd->btns[1]^=1; tsd->pos=1;
			if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1234),ret);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case 'u': case 'U': tsd->btns[2]^=1; tsd->pos=2;
			if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1234),ret);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case VK_SPACE:
			tsd->btns[tsd->pos]^=1;
			if(tsd->btns[0]) ret|=1; if(tsd->btns[1]) ret|=2; if(tsd->btns[2]) ret|=4;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1234),ret);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case VK_LEFT:
			if(tsd->pos>0) {
				tsd->pos--;
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			break;
		case VK_RIGHT:
			if(tsd->pos<2) {
				tsd->pos++;
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			break;
		}
	} break;
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;  //|DLGC_WANTCHARS;
	case WM_PAINT:
		hBmp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TEXTBIU));
		hDC=BeginPaint(hWnd,&ps);
		hDC2=CreateCompatibleDC(hDC);
		oldbmp=SelectObject(hDC2,hBmp);
		oldbrush=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
		Rectangle(hDC,0,0,47,17);
		wfocus=(hWnd==GetFocus() ? TRUE : FALSE);
		for(i=0,x=0; i<3; i++,x+=15) {
			if(i==tsd->pos&&wfocus) ys=15; else ys=0;
			if(tsd->btns[i]&1) xs=45; else xs=0;
			BitBlt(hDC,x+1,1,15,15,hDC2,xs+x,ys,SRCCOPY);
		}
		SelectObject(hDC,oldbrush);
		SelectObject(hDC2,oldbmp);
		DeleteDC(hDC2);
		EndPaint(hWnd,&ps);
		DeleteObject(hBmp);
		break;
	case WM_DESTROY:
		UnregisterHotKey(hWnd,51); UnregisterHotKey(hWnd,50);
		UnregisterHotKey(hWnd,49); UnregisterHotKey(hWnd,48);
		UnregisterHotKey(hWnd,47); UnregisterHotKey(hWnd,46);
		UnregisterHotKey(hWnd,45); UnregisterHotKey(hWnd,44);
		delete tsd;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0l;
}

typedef struct {
	char btn;
	char pos;
} TEXTALIGNDATA;

LRESULT CALLBACK TextAlignWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC,hDC2;
	HBITMAP hBmp,oldbmp;
	TEXTALIGNDATA *tsd;
	HBRUSH oldbrush;
	int i,x,xs,ys;
	BOOL wfocus;
	tsd=(TEXTALIGNDATA*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_USER:    //  Set data
		tsd->btn=(char)wParam;
		RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		break;
	case WM_USER+1:    // Get data
		return (LONG)(tsd->btn);
	case WM_CREATE:
		tsd=new TEXTALIGNDATA;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)tsd);
		tsd->pos=0;
		tsd->btn=-1;
		RegisterHotKey(hWnd,48,MOD_CONTROL,'L');
		RegisterHotKey(hWnd,49,MOD_CONTROL,'C');
		RegisterHotKey(hWnd,50,MOD_CONTROL,'R');
		RegisterHotKey(hWnd,51,MOD_CONTROL,'F');
		break;
	case WM_SETFOCUS: RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE); break;
	case WM_KILLFOCUS: RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE); break;
	case WM_LBUTTONDOWN: {
		int x=LOWORD(lParam),y=HIWORD(lParam);
		if(y>0&&y<16) {
			SetFocus(hWnd);
			if(x>0&&x<16) { tsd->btn=0; tsd->pos=0; }
			if(x>15&&x<31) { tsd->pos=1; tsd->btn=1; }
			if(x>30&&x<46) { tsd->pos=2; tsd->btn=2; }
			if(x>45&&x<61) { tsd->pos=3; tsd->btn=3; }
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),tsd->btn);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
	} break;
	case WM_HOTKEY:
		switch(wParam) {
		case 48: tsd->btn=0; tsd->pos=0; break;
		case 49: tsd->btn=1; tsd->pos=1; break;
		case 50: tsd->btn=2; tsd->pos=2; break;
		case 51: tsd->btn=3; tsd->pos=3; break;
		}
		if(wParam>=44&&wParam<=51) {
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),tsd->btn);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}
		break;
	case WM_KEYDOWN:
		switch((int)wParam) {
		case 'l': case 'L':
			tsd->btn=0; tsd->pos=0; RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),0);
			break;
		case 'c': case 'C':
			tsd->btn=1; tsd->pos=1; RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),1);
			break;
		case 'r': case 'R':
			tsd->btn=2; tsd->pos=2; RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),2);
			break;
		case 'f': case 'F':
			tsd->btn=3; tsd->pos=3; RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),3);
			break;
		case VK_SPACE:
			tsd->btn=tsd->pos;
			SendMessage(GetParent(hWnd),WM_COMMAND,MAKELONG(GetDlgCtrlID(hWnd),0x1235),tsd->btn);
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			break;
		case VK_LEFT:
			if(tsd->pos>0) {
				tsd->pos--; RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			break;
		case VK_RIGHT:
			if(tsd->pos<3) {
				tsd->pos++; RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			break;
		}
		break;
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;  //|DLGC_WANTCHARS;
	case WM_PAINT:
		hBmp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TEXTALIGN));
		hDC=BeginPaint(hWnd,&ps);
		hDC2=CreateCompatibleDC(hDC);
		oldbmp=SelectObject(hDC2,hBmp);
		oldbrush=SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
		Rectangle(hDC,0,0,62,17);
		wfocus=(hWnd==GetFocus() ? TRUE : FALSE);
		for(i=0,x=0; i<4; i++,x+=15) {
			if(i==tsd->pos&&wfocus) ys=15; else ys=0;
			if(tsd->btn==i) xs=60; else xs=0;
			BitBlt(hDC,x+1,1,15,15,hDC2,xs+x,ys,SRCCOPY);
		}
		SelectObject(hDC,oldbrush);
		SelectObject(hDC2,oldbmp);
		DeleteDC(hDC2);
		EndPaint(hWnd,&ps);
		DeleteObject(hBmp);
		break;
	case WM_DESTROY:
		UnregisterHotKey(hWnd,51); UnregisterHotKey(hWnd,50);
		UnregisterHotKey(hWnd,49); UnregisterHotKey(hWnd,48);
		UnregisterHotKey(hWnd,47); UnregisterHotKey(hWnd,46);
		UnregisterHotKey(hWnd,45); UnregisterHotKey(hWnd,44);
		delete tsd;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0l;
}
void SCPUPDATE(HWND hWnd,Stamp*s)
{
	char buf[256];
	SetDlgItemText(hWnd,IDC_STAMPNAME,s->getname());
	SetDlgItemText(hWnd,IDC_PATH,s->getfname());
	wsprintf(buf,"%d",s->numberofobjects());
	SetDlgItemText(hWnd,IDC_NUMOBJECTS,buf);
	CheckDlgButton(hWnd,IDC_PREVIEW,s->previewExist());
	CheckDlgButton(hWnd,IDC_INVERT,s->inverted());
}
void SCPUPDATE2(HWND hWnd,Stamp*s)
{
	char buf[256];
	if(IsDlgButtonChecked(hWnd,IDC_INVERT))
		s->setinvert(TRUE);
	else s->setinvert(FALSE);
	if(IsDlgButtonChecked(hWnd,IDC_SAVEPREVIEW)) {
		strcpy(buf,StatusLineText1);
		strcpy(StatusLineText1,"Generating preview. Please wait...");
		UpdateStatusLine(StatusLineID1);
		s->setpreview(TRUE);
		strcpy(StatusLineText1,buf);
		UpdateStatusLine(StatusLineID1);
	}else s->setpreview(FALSE);
	GetDlgItemText(hWnd,IDC_STAMPNAME,buf,256);
	s->setname(buf);
}
BOOL CALLBACK StampConfigProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	Stamp* s=(Stamp*)GetWindowLong(hWnd,DWL_USER);
	char buf[256];
	HDC hDC;
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hWnd,DWL_USER,lParam);
		s=(Stamp*)lParam;
		SetDlgItemText(hWnd,IDC_STAMPNAME,s->getname());
		SetDlgItemText(hWnd,IDC_PATH,s->getfname());
		wsprintf(buf,"%d",s->numberofobjects());
		SetDlgItemText(hWnd,IDC_NUMOBJECTS,buf);
		CheckDlgButton(hWnd,IDC_PREVIEW,s->previewExist());
		CheckDlgButton(hWnd,IDC_INVERT,s->inverted());
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_NEW:
			PostMessage(WndMain,WM_COMMAND,CM_FILENEW,0);
			SCPUPDATE(hWnd,s);
			break;
		case IDC_SAVE:
			PostMessage(WndMain,WM_COMMAND,CM_FILESAVE,0);
			SCPUPDATE2(hWnd,s);
			break;
		case IDC_SAVEAS:
			PostMessage(WndMain,WM_COMMAND,CM_FILESAVE_AS,0);
			SCPUPDATE2(hWnd,s);
			break;
		case IDC_OPEN:
			PostMessage(WndMain,WM_COMMAND,CM_FILEOPEN,0);
			SCPUPDATE(hWnd,s);
			break;
		case IDOK:
			SCPUPDATE2(hWnd,s);
			panel->redraw();
			preview->update();
			break;
		case IDCANCEL:
			WndEdit=NULL;
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_CLOSE:
		WndEdit=NULL;
		DestroyWindow(hWnd);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
