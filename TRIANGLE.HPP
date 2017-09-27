//File:triangle.hpp//

BOOL CALLBACK GTriangleDlgProc(HWND,UINT,WPARAM,LPARAM);
class GeomTriangle : public StampNode
{
int offx,offy,side,thick;
HWND hWnd,hMainWnd;
BOOL _upd;
public:
GeomTriangle();
GeomTriangle(int,int,int,int);
void setoffset(int,int);
void getoffset(int*,int*);
void setsize(int,int);
void getsize(int*,int*);
int getoradius(void);
int getiradius(void);
void setoradius(int);
void setiradius(int);
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
inline HWND GeomTriangle::getmainwnd(void)
{
	return hMainWnd;
}
inline void GeomTriangle::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}
inline void GeomTriangle::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
}
inline void GeomTriangle::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void GeomTriangle::move(int dx,int dy){
	offx+=dx; offy+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}
inline GeomTriangle::GeomTriangle() : offx(0),offy(0),side(10),thick(0),_upd(FALSE){
}
inline void GeomTriangle::getsname(char* s)
{
	sprintf(s,"Triangle(%.1f, thickness=%.1f)",(float)side/10,(float)thick/10);
}

inline void GeomTriangle::setsize(int _side,int t=-1)
{
	side=_side; if(t!=-1) thick=t;
}
inline void GeomTriangle::getsize(int*_side,int*t)
{
	if(_side) *_side=side;
	if(t) *t=thick;
}

inline void GeomTriangle::setoffset(int _x,int _y)
{
	offx=_x; offy=_y;
}
inline void GeomTriangle::getoffset(int*_x,int*_y)
{
	if(_x) *_x=offx;
	if(_y) *_y=offy;
}
inline void GeomTriangle::updated(void)
{
	_upd=FALSE;
}
inline void GeomTriangle::changed(void)
{
	_upd=TRUE;
}
inline BOOL GeomTriangle::isChanged(void)
{
	return _upd;
}
inline int GeomTriangle::getoradius(void)
{
	return (int)((double)side*0.577350269);
}
inline int GeomTriangle::getiradius(void)
{
	return (int)((double)side*0.288675134);
}
inline void GeomTriangle::setoradius(int or)
{
	side=(int)((double)or/0.577350269);
}
inline void GeomTriangle::setiradius(int ir)
{
	side=(int)((double)ir/0.288675134);
}

