//File:opensave.hpp//

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
} OFNOTIFY,FAR *LPOFNOTIFY;
typedef struct
{
	HBITMAP hBmp;
	int offx,offy;
	int sizex,sizey;
	int mx,my;
	BOOL captured;
} DSPWDATA;
LRESULT CALLBACK DlgSmallPreviewWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL NEAR PASCAL TestNotify(HWND hDlg,LPOFNOTIFY pofn);
BOOL CALLBACK OpenFileNameHook(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

