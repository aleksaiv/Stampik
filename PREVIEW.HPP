//File:preview.hpp//

class Preview
{
HWND hMainWnd;
HINSTANCE hInst;
HWND hWnd;
HWND hPreviewWnd;
HWND HRul,VRul;
int VIEW_X,VIEW_Y;
int VIEW_WIDTH,VIEW_HEIGHT;
int msx,msy;
int offx,offy;
int posx,posy;
int cx,cy;
Stamp* stmp;
HBITMAP hSurface;
StampNode* sel;
public:
Preview(HINSTANCE,HWND,Stamp*);
~Preview();
int GetViewWidth(void);
int GetViewHeight(void);
void SetOffset(int,int);
void GetOffset(int&,int&);
void MoveTo(int,int);
void SetMouseCoord(int,int);
int GetMouseX(void);
int GetMouseY(void);
void Selection(StampNode*);
StampNode* GetSelection(void);
void SeekObject(StampNode*);
void GetPosition(int&,int&);
void update(void);      // устанавливает флаг перерисовки
HWND GetPreviewWnd(void);
HWND GetMainWnd(void);
HWND GetHRuler(void);
HWND GetVRuler(void);
void redraw(void);
void draw(HDC);
BOOL hittest(int,int);
void reldrawcontour(int,int);
void moveobject(void);
void movecontour(int,int);
void drawcontour(HDC);
void dragcontour(void);
void hidecontour(void);
void showcontour(void);
void drawcross(HDC);
void movecross(int,int);
void preparedc(HDC);
};
inline HWND Preview::GetMainWnd(void)
{
	return hMainWnd;
}
inline void Preview::dragcontour(void)
{
	SendMessage(hMainWnd,WM_PREVIEWMOVECONTOUR,MAKELONG(cx,cy),(LPARAM)sel);
}
inline void Preview::moveobject(void)
{
	int ox=cx,oy=cy;
	cx=cy=0;
	SendMessage(hMainWnd,WM_PREVIEWMOVEOBJECT,MAKELONG(ox,oy),(LPARAM)sel);
}
inline void Preview::movecontour(int x,int y)
{
	cx=x; cy=y;
}
inline void Preview::Selection(StampNode*sn)
{
	sel=sn;
}
inline StampNode* Preview::GetSelection(void)
{
	return sel;
}
inline void Preview::SetMouseCoord(int x,int y)
{
	msx=x; msy=y;
	SendMessage(hMainWnd,WM_UPDATECOORDS,MAKELONG(x-posx,y-posy),MAKELONG(x,y));
}
inline int Preview::GetMouseX(void)
{
	return msx;
}
inline int Preview::GetMouseY(void)
{
	return msy;
}
inline HWND Preview::GetHRuler(void)
{
	return HRul;
}
inline HWND Preview::GetVRuler(void)
{
	return VRul;
}
inline int Preview::GetViewWidth(void)
{
	return VIEW_WIDTH;
}
inline int Preview::GetViewHeight(void)
{
	return VIEW_HEIGHT;
}
inline void Preview::SetOffset(int x,int y)
{
	offx=x; offy=y;
}
inline void Preview::GetOffset(int&x,int&y)
{
	x=offx; y=offy;
}
inline HWND Preview::GetPreviewWnd(void)
{
	return hPreviewWnd;
}
inline void Preview::MoveTo(int x,int y)
{
	posx=x; posy=y;
}
inline void Preview::GetPosition(int& x,int&y)
{
	x=posx; y=posy;
}
LRESULT CALLBACK PreviewWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK RulerWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
