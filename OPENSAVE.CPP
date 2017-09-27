//File:opensave.cpp//

#include <windows.h>
#include "commdlg3.h"
#include "list.hpp"
#include "stamp.hpp"
#include "stmp.rh"
// Structure used for all OpenFileName notifications
typedef struct tagNMHDR
{
	HWND hwndFrom;
	UINT idFrom;
	UINT code;            // NM_ code
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;
typedef struct _OFNOTIFY
{
	NMHDR hdr;
	LPOPENFILENAME lpOFN;
	LPSTR pszFile;                        // May be NULL
} OFNOTIFY, FAR *LPOFNOTIFY;
typedef struct
{
	HBITMAP hBmp;
	int offx,offy;
	int sizex,sizey;
	int mx,my;
	BOOL captured;
} DSPWDATA;
LRESULT CALLBACK DlgSmallPreviewWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HBITMAP hBmp,oldbmp;
	BITMAP bmp;
	HDC hMemDC,hDC;
	RECT rect;
	PAINTSTRUCT ps;
	DSPWDATA* dspwd;
	dspwd=(DSPWDATA*)GetWindowLong(hWnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_CREATE:
		dspwd=new DSPWDATA;
		dspwd->captured=FALSE;
		dspwd->offx=dspwd->offy=0;
		dspwd->sizex=dspwd->sizey=0;
		dspwd->mx=dspwd->my=0;
		dspwd->hBmp=NULL;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)dspwd);
		break;
	case WM_USER+1:
		dspwd->hBmp=(HBITMAP)lParam;
		if(lParam!=0) {
			GetObject(dspwd->hBmp,sizeof(BITMAP),&bmp);
			dspwd->offx=dspwd->offy=0;
			dspwd->sizex=bmp.bmWidth; dspwd->sizey=bmp.bmHeight;
		}
		dspwd->captured=FALSE;

		break;
	case WM_MOUSEMOVE:
		if(dspwd->captured==FALSE) break;
		dspwd->offx=dspwd->mx-(int)LOWORD(lParam);
		dspwd->offy=dspwd->my-(int)HIWORD(lParam);
		GetClientRect(hWnd,&rect);
		if(dspwd->offx<0) dspwd->offx=0;
		if(dspwd->offy<0) dspwd->offy=0;
		if(dspwd->offx+rect.right>=dspwd->sizex)
			dspwd->offx=dspwd->sizex-rect.right-1;
		if(dspwd->offy+rect.bottom>=dspwd->sizey)
			dspwd->offy=dspwd->sizey-rect.bottom-1;
	case WM_USER:
		hBmp=dspwd->hBmp;
		hDC=GetDC(hWnd);
		GetClientRect(hWnd,&rect);
		if(hBmp!=NULL) {
			GetObject(hBmp,sizeof(BITMAP),&bmp);
			hMemDC=CreateCompatibleDC(hDC);
			oldbmp=SelectObject(hMemDC,hBmp);
			BitBlt(hDC,0,0,rect.right,rect.bottom,hMemDC,dspwd->offx,dspwd->offy,SRCCOPY);
			SelectObject(hMemDC,oldbmp);
			DeleteDC(hMemDC);
		}else{
			HBRUSH oldbr=SelectObject(hDC,GetStockObject(LTGRAY_BRUSH));
			PatBlt(hDC,0,0,rect.right,rect.bottom,PATCOPY);
			SelectObject(hDC,oldbr);
		}
		ReleaseDC(hWnd,hDC);
		break;
	case WM_LBUTTONDOWN:
		if(dspwd->hBmp!=NULL) {
			dspwd->captured=TRUE;
			SetCapture(hWnd);
			dspwd->mx=LOWORD(lParam)+dspwd->offx;
			dspwd->my=HIWORD(lParam)+dspwd->offy;
		}
		break;
	case WM_LBUTTONUP:
		if(dspwd->captured) {
			dspwd->captured=FALSE;
			ReleaseCapture();
		}
		break;
	case WM_PAINT:
		hBmp=dspwd->hBmp;
		BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rect);
		if(hBmp!=NULL) {
			GetObject(hBmp,sizeof(BITMAP),&bmp);
			hMemDC=CreateCompatibleDC(ps.hdc);
			oldbmp=SelectObject(hMemDC,hBmp);
			BitBlt(ps.hdc,0,0,rect.right,rect.bottom,hMemDC,dspwd->offx,dspwd->offy,SRCCOPY);
			SelectObject(hMemDC,oldbmp);
			DeleteDC(hMemDC);
		}else{
			HBRUSH oldbr=SelectObject(ps.hdc,GetStockObject(LTGRAY_BRUSH));
			PatBlt(ps.hdc,0,0,rect.right,rect.bottom,PATCOPY);
			SelectObject(ps.hdc,oldbr);
		}
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		delete dspwd;
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
BOOL NEAR PASCAL TestNotify(HWND hDlg, LPOFNOTIFY pofn)
{
	LPOPENFILENAME ofn;
	POFNCUSTDATA ofnCustData;
	switch (pofn->hdr.code) {
	case CDN_SELCHANGE: {
		char szFile[MAX_PATH];
		char name[250];
		if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
		                                 szFile, sizeof(szFile)) <= sizeof(szFile)) {
			ofn=(LPOPENFILENAME)GetWindowLong(hDlg,DWL_USER);
			ofnCustData=(POFNCUSTDATA)ofn->lCustData;
			if(ofnCustData->__hBitmapPreview)
				DeleteObject(ofnCustData->__hBitmapPreview);
			ofnCustData->__hBitmapPreview=ReadStampNamePreview(szFile,name);
			SetDlgItemText(hDlg, IDC_STAMPNAME,name);
//        if(ofnCustData->__hBitmapPreview){
			SendDlgItemMessage(hDlg,IDC_PREVIEW,WM_USER+1,0,
			                   (LPARAM)ofnCustData->__hBitmapPreview);
			SendDlgItemMessage(hDlg, IDC_PREVIEW,WM_USER,0,0);
//        }
		}
	}
		                  break;
	}
	return(TRUE);
}
BOOL CALLBACK OpenFileNameHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POFNCUSTDATA ofnCustData;
	LPOPENFILENAME ofn;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		ofn=(LPOPENFILENAME)lParam;
		ofnCustData=(POFNCUSTDATA)ofn->lCustData;
		SetWindowLong(hDlg,DWL_USER,lParam);
		if(ofnCustData->preview==FALSE)
			ShowWindow(GetDlgItem(hDlg,IDC_SAVEPREVIEW),SW_HIDE);
		break;
	case WM_NOTIFY:
		TestNotify(hDlg,(LPOFNOTIFY)lParam);
		return FALSE;
	case WM_DESTROY:
		ofn=(LPOPENFILENAME)GetWindowLong(hDlg,DWL_USER);
		ofnCustData=(POFNCUSTDATA)ofn->lCustData;
		if(ofnCustData->preview) {
			if(IsDlgButtonChecked(hDlg,IDC_SAVEPREVIEW))
				ofnCustData->previewstat=TRUE;
			else
				ofnCustData->previewstat=FALSE;
		}
		if(ofnCustData->__hBitmapPreview)
			DeleteObject(ofnCustData->__hBitmapPreview);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
