//File:about.cpp//
#include <windows.h>

#define WIDTHBYTES(bits)      ((((bits) + 31) & ~31) >> 3)
typedef struct
{
	HPALETTE hPalette;
	HBITMAP hBitmap;
	LPVOID lpvBits;
	HANDLE bmiHeader;
} DIBITMAP;
HBITMAP LoadDIBitmap(LPSTR lpFileName,DIBITMAP *dib)
{
	HDC hdc;
	HANDLE hFile;
	HBITMAP hBitmap;
	BITMAPFILEHEADER bmfHeader;
	UINT nNumColors;
	HANDLE hDIBNew;
	LPBITMAPINFOHEADER lpbih;
	LPBITMAPINFO lpbmi;
	HANDLE hLogPal;
	LPLOGPALETTE lpPal;
	DWORD dwBytesRead;
	int i;

	hFile = CreateFile(lpFileName,
	                   GENERIC_READ,
	                   FILE_SHARE_READ,
	                   NULL,
	                   OPEN_EXISTING,
	                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
	                   NULL);
	if (hFile == INVALID_HANDLE_VALUE) return NULL;
	if (!ReadFile(hFile,
	              &bmfHeader,
	              sizeof(BITMAPFILEHEADER),
	              &dwBytesRead,
	              NULL)||dwBytesRead != sizeof(BITMAPFILEHEADER))
		goto ErrExit;
	if (bmfHeader.bfType != 0x4d42)    // 'BM'
		goto ErrExit;
	dib->bmiHeader = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	if (!dib->bmiHeader) goto ErrExit;
	lpbih = (LPBITMAPINFOHEADER)GlobalLock(dib->bmiHeader);
	if (!lpbih) goto ErrExitFree;
	lpbmi=(LPBITMAPINFO)lpbih;
	if (!ReadFile(hFile,
	              lpbih,
	              sizeof(BITMAPINFOHEADER),
	              &dwBytesRead,
	              NULL)||dwBytesRead != sizeof(BITMAPINFOHEADER))
		goto ErrExitUnlock;
	if (lpbih->biSize != sizeof(BITMAPINFOHEADER))
		goto ErrExitUnlock;
	switch (lpbih->biBitCount)
	{
	case 1: case 4: case 8: case 16: case 24: case 32:
		break;
	default:
		goto ErrExitUnlock;
	}
	if (!(nNumColors = (UINT)lpbih->biClrUsed))
		if (lpbih->biBitCount <= 8)
			nNumColors = 1 << lpbih->biBitCount;       // standard size table
	if (lpbih->biClrUsed == 0)
		lpbih->biClrUsed = nNumColors;
	if (lpbih->biSizeImage == 0)
		lpbih->biSizeImage = WIDTHBYTES(lpbih->biWidth * lpbih->biBitCount)
		                     * lpbih->biHeight;
	if (!ReadFile(hFile,
	              (LPSTR)(lpbih) + lpbih->biSize,
	              nNumColors * sizeof(RGBQUAD),
	              &dwBytesRead,
	              NULL)|| dwBytesRead != nNumColors * sizeof(RGBQUAD))
		goto ErrExitUnlock;
	hdc = CreateCompatibleDC(NULL);
	hBitmap = CreateDIBSection(hdc,
	                           (LPBITMAPINFO)lpbih,
	                           DIB_RGB_COLORS,
	                           &(dib->lpvBits),
	                           0,
	                           0L);
	DeleteDC(hdc);
	if (!hBitmap || !dib->lpvBits)
	{
		if (hBitmap) DeleteObject(hBitmap);
		goto ErrExitUnlock;
	}
	SetFilePointer(hFile, bmfHeader.bfOffBits, NULL, FILE_BEGIN);
	if (!ReadFile(hFile,
	              dib->lpvBits,
	              lpbih->biSizeImage,
	              &dwBytesRead,
	              NULL)|| dwBytesRead != lpbih->biSizeImage)
	{
		DeleteObject(hBitmap);
		goto ErrExitUnlock;
	}

	hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) +
	                      sizeof(PALETTEENTRY) * nNumColors);
	lpPal = (LPLOGPALETTE)GlobalLock(hLogPal);
	lpPal->palVersion = 0x300;
	lpPal->palNumEntries = nNumColors;
	for ( i = 0; i < nNumColors; i++)
	{
		lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
		lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
		lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
		lpPal->palPalEntry[i].peFlags = 0;
	}
	dib->hPalette = CreatePalette(lpPal);
	if (!dib->hPalette)
	{
		GlobalUnlock(hLogPal);
		GlobalFree(hLogPal);
		goto ErrExitUnlock;
	}
	GlobalUnlock(hLogPal);
	GlobalFree(hLogPal);

	GlobalUnlock(dib->bmiHeader);
	CloseHandle(hFile);
	dib->hBitmap=hBitmap;
	return hBitmap;

ErrExitUnlock:
	GlobalUnlock(dib->bmiHeader);
ErrExitFree:
	GlobalFree(dib->bmiHeader);
ErrExit:
	CloseHandle(hFile);
	return NULL;
}
DIBITMAP AboutDIB;
BOOL APIENTRY AboutDlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	LPBITMAPINFO pbmiHeader;
	HPALETTE holdPal;
	HMENU hmenu;
	static BOOL paintbitmap=TRUE;
	switch(uMsg) {
	case WM_SYSCOMMAND:
		if(LOWORD(wParam)==0x74F0) {
			paintbitmap=paintbitmap ? FALSE : TRUE;
			RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
		}else return DefWindowProc(hWnd,uMsg,wParam,lParam);
		break;
	case WM_CLOSE:
	case WM_COMMAND:
		if(LOWORD(wParam)==IDOK||uMsg==WM_CLOSE) {
			DeleteObject(AboutDIB.hBitmap);
			DeleteObject(AboutDIB.hPalette);
			GlobalFree(AboutDIB.bmiHeader);
			EndDialog(hWnd,0);
		}
		break;
	case WM_INITDIALOG:
		LoadDIBitmap("about.bmp",&AboutDIB);
		hmenu=GetSystemMenu(hWnd,FALSE);
		AppendMenu(hmenu,MF_STRING,0x74F0,"Show hidden information");
		break;
	case WM_PAINT:
		hDC=BeginPaint(hWnd,&ps);
		holdPal=SelectPalette(hDC,AboutDIB.hPalette,TRUE);
		RealizePalette(hDC);
		pbmiHeader=(LPBITMAPINFO)GlobalLock(AboutDIB.bmiHeader);
		SetDIBitsToDevice(hDC,0,0,260,158,0,0,0,158,
		                  AboutDIB.lpvBits,pbmiHeader,DIB_RGB_COLORS);
		GlobalUnlock(AboutDIB.bmiHeader);
		SelectObject(hDC,holdPal);
		if(!paintbitmap) {
			SetBkMode(hDC,TRANSPARENT);
			SetTextColor(hDC,RGB(0,128,0));
			TextOut(hDC,6,21,"The Stampik",11);
			SetTextColor(hDC,RGB(0,103,0));
			TextOut(hDC,6,38,"Version 1.01",12);
			SetTextColor(hDC,RGB(0,78,0));
			TextOut(hDC,6,55,"Copyright (C) 1998 by",21);
			SetTextColor(hDC,RGB(0,53,0));
			TextOut(hDC,6,72,"Alex Ivanov",11);
			SetTextColor(hDC,RGB(0,28,0));
			TextOut(hDC,6,89,"Russia. Chelyabinsk.",20);
			SetTextColor(hDC,RGB(0,255,0));
			TextOut(hDC,5,20,"The Stampik",11);
			SetTextColor(hDC,RGB(0,255,0));
			TextOut(hDC,5,37,"Version 1.01",12);
			SetTextColor(hDC,RGB(0,230,0));
			TextOut(hDC,5,54,"Copyright (C) 1998 by",21);
			SetTextColor(hDC,RGB(0,205,0));
			TextOut(hDC,5,71,"Alex Ivanov.",11);
			SetTextColor(hDC,RGB(0,180,0));
			TextOut(hDC,5,88,"Russia. Chelyabinsk.",20);
		}
		EndPaint(hWnd,&ps);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}