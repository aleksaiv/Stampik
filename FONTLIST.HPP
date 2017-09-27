//File:fontlist.hpp//

void FillFontCombobox(char*defname,HWND hWnd);
void FillFontListbox(char*defname,HWND hWnd);
HWND CreateFontViewer(HINSTANCE hI,HWND hMain);
void CloseFontViewer(void);
void SetFontViewer(char*txt,char*fname,int st);
void ShowFontViewer(void);
void HideFontViewer(void);
LRESULT CALLBACK FontViewerProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
void SetFontViewerSize(int h);
void MoveFontViewer(int x,int y);
