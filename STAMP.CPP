//File:stamp.cpp//

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "stmp.rh"

#include "list.hpp"
#include "stamp.hpp"
#include "ltext.hpp"
#include "ctext.hpp"
#include "ttext.hpp"
#include "rect.hpp"
#include "preview.hpp"

#include "commdlg3.h"
#include "opensave.hpp"
#pragma argsused

extern HWND WndMain;
extern HINSTANCE hInst;
BOOL CALLBACK OpenFileNameHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK DottedLineProc(int x,int y,LPARAM lpData);

HWND StampNode::starteditor(HWND hMain)
{
	return NULL;
}
BOOL StampNode::endeditor(void)
{
	return TRUE;
}
StampClassNode* Stamp::findclass(short g,short t)
{
	if(!classes->first()) return NULL;
	StampClassNode* scn;
	do {
		scn=(StampClassNode*)classes->getwin();
		if(scn->gettype()==t&&scn->getgroup()==g) return scn;
	} while(classes->next());
	return NULL;
}
Stamp::Stamp(char *n,HWND hMain)
{
	_changed=FALSE;
	hMainWnd=hMain;
	if(n) {
		name=new char[lstrlen(n)+1];
		strcpy(name,n);
	}else{name=new char[8]; strcpy(name,"unnamed");}
	fname=NULL;
	bmpPreview=NULL;
	showmask=0xffffffff;
	classes=new StampClass;
	hDevNames=NULL; hDevModes=NULL;
}
Stamp::~Stamp(){
	if(hDevNames) GlobalFree(hDevNames);
	if(hDevModes) GlobalFree(hDevModes);
	if(bmpPreview) DeleteObject(bmpPreview);
	if(name) delete name;
	if(fname) delete fname;
	delete classes;
}
int Stamp::numberofobjects(void)
{
	int c=0;
	if(!first()) return 0;
	do {
		c++;
	} while(next());
	return c;
}
void Stamp::showhidetype(short t)
{
	showmask^=1<<(DWORD)t;
}
BOOL Stamp::showthistype(short t)
{
	if(showmask&(1<<(DWORD)t)) return TRUE; else return FALSE;
}

typedef struct
{
	LONG Height;
	LONG Width;
	LONG Size;
} BMPPREVIEW;
HBITMAP ReadStampNamePreview(char* fn,char* name)
{
	HANDLE hFile;
	DWORD dwWritten;
	LPVOID pBits;
	char tmp;
	STAMPHEADER header;
	BMPPREVIEW bmpPreview;
	HBITMAP hBmp;
	if(name!=NULL) strcpy(name,"bad file");
	hFile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
	                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile==INVALID_HANDLE_VALUE) return NULL; // возвращаем ошибку
	ReadFile(hFile,&header,sizeof(header),&dwWritten,NULL); // Read file header
	if(header.id!=0x504d5453) {
		if(name!=NULL) strcpy(name,"not a stamp file");
		CloseHandle(hFile);
		return NULL; // Not a stamp file
	}
	if(header.version<STAMP_VERSION) {
		if(name!=NULL) strcpy(name,"wrong file version");
		CloseHandle(hFile);
		return NULL;
	}
	if(name!=NULL) {
		ReadFile(hFile,&tmp,sizeof(char),&dwWritten,NULL);
		ReadFile(hFile,name,tmp,&dwWritten,NULL); name[tmp]=0;
	}
	if(header.preview==0) {
		CloseHandle(hFile);
		return NULL;
	}
	if(SetFilePointer(hFile,header.previewOFF,NULL,FILE_BEGIN)==0xFFFFFFFF) {
		CloseHandle(hFile);
		return NULL; // Bad file
	}
	ReadFile(hFile,&bmpPreview,sizeof(BMPPREVIEW),&dwWritten,NULL);
	pBits=(LPVOID)LocalAlloc(LPTR,bmpPreview.Size);
	ReadFile(hFile,pBits,bmpPreview.Size,&dwWritten,NULL);
	hBmp=CreateBitmap(bmpPreview.Width,bmpPreview.Height,1,1,pBits);
	if(hBmp==NULL) LocalFree((HLOCAL)pBits);
	CloseHandle(hFile);
	return hBmp;
}
long WriteStampPreview(HANDLE hFile,HBITMAP hBmp)
{
	BMPPREVIEW bmpPreview;
	BITMAP bmp;
	LPVOID pBits;
	DWORD dwWritten;
	GetObject(hBmp,sizeof(BITMAP),&bmp);
	bmpPreview.Width=bmp.bmWidth;
	bmpPreview.Height=bmp.bmHeight;
	bmpPreview.Size=bmp.bmHeight*bmp.bmWidthBytes;
	WriteFile(hFile,&bmpPreview,sizeof(BMPPREVIEW),&dwWritten,NULL);
	pBits=(LPVOID)LocalAlloc(LPTR,bmpPreview.Size);
	GetBitmapBits(hBmp,bmpPreview.Size,pBits);
	WriteFile(hFile,pBits,bmpPreview.Size,&dwWritten,NULL);
	LocalFree((HLOCAL)pBits);
	return (bmpPreview.Size+sizeof(BMPPREVIEW));
}
int Stamp::save_file(char *fn)
{
	HANDLE hFile;
	StampNode *n;
	DWORD dwWritten;
	int numobj=0;
	char tmp;
	STAMPHEADER header;
	hFile=CreateFile(fn,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,
	                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile==INVALID_HANDLE_VALUE) return -1; // возвращаем ошибку
	if(!first())
		numobj=0;
	else{
		do {numobj++;} while(next());
		first();
	}
	tmp=(char)lstrlen(name);
	header.id=0x504d5453; // STMP
	header.version=STAMP_VERSION;
	header.inverse=inverse;
	header.showmask=showmask;
	header.preview=0;
	header.previewOFF=0;
	header.dataOFF=sizeof(header)+1+tmp;
	header.numobj=numobj;
	WriteFile(hFile,&header,sizeof(header),&dwWritten,NULL);
	WriteFile(hFile,&tmp,sizeof(char),&dwWritten,NULL);
	WriteFile(hFile,name,tmp,&dwWritten,NULL);
	if(numobj>0)
		do {
			n=(StampNode*)getwin();
			writeTypeGroup(hFile,n->gettype(),n->getgroup());
			saveobject(hFile,n);
		} while(next());
	if(bmpPreview!=NULL) {
		header.previewOFF=SetFilePointer(hFile,0,NULL,FILE_CURRENT);
		header.preview=1;
		WriteStampPreview(hFile,bmpPreview);
		SetFilePointer(hFile,0,NULL,FILE_BEGIN);
		WriteFile(hFile,&header,sizeof(header),&dwWritten,NULL);
	}
	CloseHandle(hFile);
	_changed=FALSE;
	return 0;
}
int Stamp::open_file(char *fn)
{
	// 1 - Проверяем заголовок
	// 2 - Проверяем версию
	// 4 - пропускаем, если есть, превью
	// 3 - Определяем количество объектов
	// 5 - загружаем все объекты
	StampNode* n;
	HANDLE hFile;
	DWORD dwWritten;
	char tmp;
	int i;
	short type,group,length;
	STAMPHEADER header;
	BMPPREVIEW bmpP;
	LPVOID pBits;
	hFile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
	                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return -1; // возвращаем ошибку
	}
	ReadFile(hFile,&header,sizeof(header),&dwWritten,NULL); // Read file header
	if(header.id!=0x504d5453) {
		CloseHandle(hFile);
		return -2; // Not a stamp file
	}
	if(header.version<STAMP_VERSION) {
		CloseHandle(hFile);
		return -4;
	}
	ReadFile(hFile,&tmp,sizeof(char),&dwWritten,NULL);
	inverse=header.inverse; showmask=header.showmask;
	if(name) delete name;
	name=new char[tmp+1];
	ReadFile(hFile,name,tmp,&dwWritten,NULL); name[tmp]=0;
	if(SetFilePointer(hFile,header.dataOFF,NULL,FILE_BEGIN)==0xFFFFFFFF) {
		CloseHandle(hFile);
		return -3; // Bad file
	}
	for(i=0; i<header.numobj; i++) {
		readTypeGroup(hFile,type,group);
		n=loadobject(hFile,type,group);
		if(n) insert(n);
		else{
			if(MessageBox(WndMain,"Do you wish to continue loading?",
			              "Founded unknown object!",MB_YESNO|MB_ICONSTOP)==IDNO)
				break;
			ReadFile(hFile,&length,sizeof(length),&dwWritten,NULL);
			SetFilePointer(hFile,length,NULL,FILE_CURRENT);
		}
	}
	if(bmpPreview) DeleteObject(bmpPreview);
	if(header.preview!=0) {
		if(SetFilePointer(hFile,header.previewOFF,NULL,FILE_BEGIN)==0xFFFFFFFF) {
			CloseHandle(hFile);
			return -3; // Bad file
		}
		ReadFile(hFile,&bmpP,sizeof(BMPPREVIEW),&dwWritten,NULL);
		pBits=(LPVOID)LocalAlloc(LPTR,bmpP.Size);
		ReadFile(hFile,pBits,bmpP.Size,&dwWritten,NULL);
		bmpPreview=CreateBitmap(bmpP.Width,bmpP.Height,1,1,pBits);
		if(bmpPreview==NULL) LocalFree((HLOCAL)pBits);
	}else bmpPreview=NULL;
	CloseHandle(hFile);
	_changed=FALSE;
	return 0;
}
int Stamp::save(void)
{
	if(bmpPreview) generatepreview();
	if(fname==NULL) return saveas();
	save_file(fname);
	return 0;
}
int Stamp::saveas(void)
{
	char szTemp[] = "Stamp Files (*.stm)\0*.stm\0All Files (*.*)\0*.*\0";
	char szName[256];
	OFNCUSTDATA myCustData;
	OPENFILENAME ofn;
	myCustData.preview=TRUE; myCustData.previewstat=FALSE;
	if(fname) strncpy(szName,fname,255); else strcpy(szName,"");
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
	ofn.lpstrTitle = "Save as...";  // Title for dialog
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST |
	            OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = "stm";
	ofn.lCustData = (LPARAM)&myCustData;
	ofn.lpfnHook = (LPOFNHOOKPROC)OpenFileNameHook;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENFILEDIALOG);
	if(GetSaveFileName( &ofn ) != TRUE)
	{
		if(CommDlgExtendedError()!=0) // 0 value means user selected Cancel
			MessageBox(WndMain,"Cannot get name of file","WARNING",MB_OK|MB_ICONSTOP);
		else return 1;
		return -1;
	}
	if(fname) delete fname;
	fname=new char[lstrlen(szName)+1];
	strcpy(fname,szName);
	if(myCustData.previewstat) {
		generatepreview();
	}else if(bmpPreview) {DeleteObject(bmpPreview); bmpPreview=NULL;}
	return save();
}
int Stamp::open(char*defname)
{
	char szTemp[] = "Stamp Files (*.stm)\0*.stm\0All Files (*.*)\0*.*\0";
	char szName[256];
	OPENFILENAME ofn;
	OFNCUSTDATA myCustData;
	HANDLE hFile;
	BOOL c=FALSE;
	if(_changed) {
		switch(MessageBox(WndMain,"Stamp has been changed.\n\nSave before closing?",
		                  "Stampik",MB_YESNOCANCEL|MB_ICONQUESTION))
		{
		case IDYES:
			if(save()) return -1;
			break;
		case IDCANCEL: return -3;
		}
		c=TRUE;
	}
	strcpy(szName,"*.stm");
	if(defname!=NULL) {
		hFile=CreateFile(defname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
		                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(hFile==INVALID_HANDLE_VALUE) {
			return -1; // возвращаем ошибку
		}
		CloseHandle(hFile);
		if(c) _changed=FALSE;
		if(close()) return -2;
		if(fname) delete fname;
		fname=new char[lstrlen(defname)+1];
		strcpy(fname,defname);
		return open_file(fname);
	}
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
	ofn.lpstrTitle = "Open stamp";  // Title for dialog
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST |
	            OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	myCustData.preview=FALSE;
	ofn.lCustData = (LPARAM)&myCustData;
	ofn.lpfnHook = (LPOFNHOOKPROC)OpenFileNameHook;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENFILEDIALOG);
	if(GetOpenFileName( &ofn ) != TRUE)
	{
		if(CommDlgExtendedError()!=0) // 0 value means user selected Cancel
			MessageBox(WndMain,"Cannot get name of file","WARNING",MB_OK|MB_ICONSTOP);
		else return 1;
		return -1;
	}
	if(c) _changed=FALSE;
	if(close()) return -2;
	if(fname) delete fname;
	fname=new char[lstrlen(szName)+1];
	strcpy(fname,szName);
	return open_file(fname);
}
int Stamp::close(void)
{
	if(_changed) {
		switch(MessageBox(WndMain,"Stamp has been changed.\n\nSave before closing?",
		                  "Stampik",MB_YESNOCANCEL|MB_ICONQUESTION))
		{
		case IDYES:
			if(save()) return -1;
			break;
		case IDCANCEL: return -3;
		}
	}
	clear();
	_changed=FALSE;
	if(name) delete name;
	name=new char[8];
	strcpy(name,"unknown");
	if(fname) delete fname;
	fname=NULL;
	return 0;
}

int Stamp::saveobject(HANDLE hFile,StampNode* obj)
{
	obj->save(hFile);
	return 0;
}
#pragma argsused

StampNode* Stamp::loadobject(HANDLE hFile,short type,short group)
{
	switch(type) {
	case 0: {
		LineText* lt=new LineText;
		lt->load(hFile);
		return lt;
	}
	case 1: {
		CircleText* ct=new CircleText;
		ct->load(hFile);
		return ct;
	}
	case 2: {
		TriangleText* tt=new TriangleText;
		tt->load(hFile);
		return tt;
	}
	default:
		StampClassNode* scn=findclass((short)-1,type);
		if(!scn) return NULL;
		return scn->Load(hFile);
	}
}

void Stamp::generatepreview(void)
{
	if(bmpPreview) DeleteObject(bmpPreview);
	HDC hDC,hMemDC;
	HBITMAP oldbmp2,oldbmp1,bmpMem;
	hDC=CreateCompatibleDC(NULL);
	hMemDC=CreateCompatibleDC(NULL);
	bmpMem=CreateCompatibleBitmap(hDC,STAMP_WIDTH,STAMP_HEIGHT);
	bmpPreview=CreateCompatibleBitmap(hMemDC,PREVIEW_WIDTH,PREVIEW_HEIGHT);
	oldbmp1=SelectObject(hDC,bmpMem);
	oldbmp2=SelectObject(hMemDC,bmpPreview);
	SetMapMode(hDC,MM_ISOTROPIC);
	SetWindowExtEx(hDC,STAMP_WIDTH,-STAMP_HEIGHT,NULL);
	SetViewportExtEx(hDC,STAMP_WIDTH,STAMP_HEIGHT,NULL);
	SetWindowOrgEx(hDC,0,0,NULL);
	SetViewportOrgEx(hDC,STAMP_WIDTH>>1,STAMP_HEIGHT>>1,NULL);
	draw(hDC);
	SetMapMode(hDC,MM_TEXT);
	SetWindowOrgEx(hDC,0,0,NULL);
	SetViewportOrgEx(hDC,0,0,NULL);
	StretchBlt(hMemDC,0,0,PREVIEW_WIDTH,PREVIEW_HEIGHT,
	           hDC,0,0,STAMP_WIDTH,STAMP_HEIGHT,
	           SRCCOPY);
	SelectObject(hDC,oldbmp1);
	SelectObject(hMemDC,oldbmp2);
	DeleteDC(hDC);
	DeleteDC(hMemDC);
	DeleteObject(bmpMem);
}
void Stamp::draw(HDC hDC)
{
	char level=0;
	short t,g;
	POINT pt;
	DTLPDATA dtlData;
	SetViewportOrgEx(hDC,0,STAMP_HEIGHT,&pt); SetWindowOrgEx(hDC,0,0,NULL);
	PatBlt(hDC,0,0,STAMP_WIDTH-1,STAMP_HEIGHT-1,WHITENESS);
	dtlData.hDC=hDC; dtlData.c=0;
	SetWindowOrgEx(hDC,0,0,NULL);
	LineDDA(0,STAMP_HEIGHT>>1,STAMP_WIDTH,STAMP_HEIGHT>>1,(LINEDDAPROC)DottedLineProc,(LPARAM)&dtlData);
	LineDDA(STAMP_WIDTH>>1,0,STAMP_WIDTH>>1,STAMP_HEIGHT,(LINEDDAPROC)DottedLineProc,(LPARAM)&dtlData);
	SetViewportOrgEx(hDC,pt.x,pt.y,NULL);
	if(!first()) return;
	SetBkMode(hDC,TRANSPARENT);
	SetROP2(hDC,R2_COPYPEN);
	level=0;
	while(level<3) {
		do {
			g=((StampNode*)getwin())->getgroup();
			t=((StampNode*)getwin())->gettype();
			if((1<<(DWORD)t)&showmask) // show object of this type ?
				if(g==level) // 2 - geometry, 1 - text, 0 - pics
					((StampNode*)getwin())->draw(hDC);
		} while(next());
		first();
		level++;
		if(level>1&&inverse) {
			SetROP2(hDC,R2_MERGEPENNOT);
		}
	}
}

inline int max(int a,int b){
	return a>b ? a : b;
}
inline int min(int a,int b){
	return a<b ? a : b;
}
void Stamp::calcrect(RECT*r)
{
	RECT r2;
	short t,g;
	r->left=r->right=r->bottom=r->top=0;
	if(!first()) return;
	r->left=STAMP_WIDTH>>1; r->top=-(STAMP_HEIGHT>>1);
	r->right=-(STAMP_WIDTH>>1); r->bottom=STAMP_HEIGHT>>1;
	do {
		g=((StampNode*)getwin())->getgroup();
		t=((StampNode*)getwin())->gettype();
		if((1<<(DWORD)t)&showmask) // show object of this type ?
			((StampNode*)getwin())->calcrect(&r2);
		r->left=min(r2.left,r->left);
		r->top=max(r2.top,r->top);
		r->right=max(r2.right,r->right);
		r->bottom=min(r2.bottom,r->bottom);
	} while(next());
	first();
}
void Stamp::print_stamp(HDC hDC)
{
	char level=0;
	short t,g;
//  POINT pt;
	if(!first()) return;
	SetBkMode(hDC,TRANSPARENT);
	while(level<3) {
		do {
			g=((StampNode*)getwin())->getgroup();
			t=((StampNode*)getwin())->gettype();
			if((1<<(DWORD)t)&showmask) // show object of this type ?
				if(g==level) // 2 - geometry, 1 - text, 0 - pics
					((StampNode*)getwin())->print(hDC);
		} while(next());
		first();
		level++;
		if(level>1&&inverse) {
			SetROP2(hDC,R2_MERGEPENNOT);
		}
	}
}

