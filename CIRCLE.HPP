//File: circle.hpp//

BOOL CALLBACK GCircleDlgProc(HWND,UINT,WPARAM,LPARAM);
class GeomCircle : public StampNode
{
int offx,offy,rad,thick;
HWND hWnd,hMainWnd;
BOOL _upd;
public:
GeomCircle();
GeomCircle(int,int,int,int);
void setoffset(int,int);
void getoffset(int*,int*);
void setsize(int,int);
void getsize(int*,int*);
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
inline HWND GeomCircle::getmainwnd(void)
{
	return hMainWnd;
}
inline void GeomCircle::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}
inline void GeomCircle::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
}
inline void GeomCircle::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void GeomCircle::move(int dx,int dy){
	offx+=dx; offy+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}
inline GeomCircle::GeomCircle() : offx(0),offy(0),rad(0),thick(0),_upd(FALSE){
}
inline void GeomCircle::getsname(char* s)
{
	sprintf(s,"Circle(at %.1f,%.1f, radius=%.1f, thickness=%.1f)",
	        (float)offx/10,(float)offy/10,(float)rad/10,(float)thick/10);
}

inline void GeomCircle::setsize(int _r,int t=-1)
{
	rad=_r; if(t!=-1) thick=t;
}
inline void GeomCircle::getsize(int*_x,int*t)
{
	if(_x) *_x=rad;
	if(t) *t=thick;
}

inline void GeomCircle::setoffset(int _x,int _y)
{
	offx=_x; offy=_y;
}
inline void GeomCircle::getoffset(int*_x,int*_y)
{
	if(_x) *_x=offx;
	if(_y) *_y=offy;
}
inline void GeomCircle::updated(void)
{
	_upd=FALSE;
}
inline void GeomCircle::changed(void)
{
	_upd=TRUE;
}
inline BOOL GeomCircle::isChanged(void)
{
	return _upd;
}
