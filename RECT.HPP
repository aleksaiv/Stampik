//File:rect.hpp//

BOOL CALLBACK GRectDlgProc(HWND,UINT,WPARAM,LPARAM);
class GeomRect : public StampNode
{
int offx,offy,w,h,thick;
HWND hWnd,hMainWnd;
BOOL _upd;
public:
GeomRect();
GeomRect(int,int,int,int,int);
void setoffset(int,int);
void getoffset(int*,int*);
void setsize(int,int,int);
void getsize(int*,int*,int*);
virtual BOOL load(HANDLE);
virtual BOOL save(HANDLE);
virtual void draw(HDC);
virtual void getsname(char*);
virtual void drawcontour(HDC);
virtual BOOL hittest(int,int);
virtual HWND starteditor(HWND);
virtual BOOL endeditor(void);
virtual void move(int,int);
virtual void drag(int,int);
virtual void calcrect(RECT*,HDC hDC=NULL);
HWND getmainwnd(void);
void update(void);
void del(void);
void close(void);
void updated(void);
void changed(void);
BOOL isChanged(void);
};
inline HWND GeomRect::getmainwnd(void)
{
	return hMainWnd;
}
inline void GeomRect::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}
inline void GeomRect::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
}
inline void GeomRect::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void GeomRect::move(int dx,int dy){
	offx+=dx; offy+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}
inline GeomRect::GeomRect() : offx(0),offy(0),w(0),h(0),thick(0),_upd(FALSE){
}
inline void GeomRect::getsname(char* s)
{
	sprintf(s,"Rectangle(%.1fx%.1f, thickness=%.1f)",(float)w/10,(float)h/10,(float)thick/10);
}

inline void GeomRect::setsize(int _x,int _y,int t=-1)
{
	w=_x; h=_y; if(t!=-1) thick=t;
}
inline void GeomRect::getsize(int*_x,int*_y,int*t)
{
	if(_x) *_x=w;
	if(_y) *_y=h;
	if(t) *t=thick;
}

inline void GeomRect::setoffset(int _x,int _y)
{
	offx=_x; offy=_y;
}
inline void GeomRect::getoffset(int*_x,int*_y)
{
	if(_x) *_x=offx;
	if(_y) *_y=offy;
}
inline void GeomRect::updated(void)
{
	_upd=FALSE;
}
inline void GeomRect::changed(void)
{
	_upd=TRUE;
}
inline BOOL GeomRect::isChanged(void)
{
	return _upd;
}
