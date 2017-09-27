//File: line.hpp//

class GeomLine : public StampNode
{
int x1,y1,x2,y2;
int thick,style;
BOOL _upd;
HWND hWnd,hMainWnd;
public:
GeomLine();
GeomLine(int,int,int,int,int,int);
void setstyle(int);
int getstyle(void);
void setthick(int);
int getthick(void);
void getcoord(int&,int&,int&,int&);
void setcoord(int,int,int,int);

virtual HWND starteditor(HWND);
virtual BOOL endeditor(void);
virtual void move(int,int);
virtual void drag(int,int);

virtual void getsname(char*);
virtual void draw(HDC);
virtual void drawcontour(HDC);
virtual BOOL hittest(int,int);
virtual BOOL save(HANDLE);
virtual BOOL load(HANDLE);
HWND getmainwnd(void);
void update(void);
void close(void);
void updated(void);
void changed(void);
BOOL isChanged(void);
void del(void);
};
inline GeomLine::GeomLine(){
	x1=y1=x2=y2=0;
	thick=0; style=PS_SOLID;
	settype(3);
	setgroup(2);
	_upd=FALSE;
}
inline GeomLine::GeomLine(int _x1,int _y1,int _x2,int _y2,int t=1,int s=PS_SOLID){
	_upd=FALSE;
	settype(3); setgroup(2);
	x1=_x1; y1=_y1; x2=_x2; y2=_y2;
	thick=t; style=s;
}
inline void GeomLine::setstyle(int s){
	style=s;
}
inline int GeomLine::getstyle(void){
	return style;
}
inline void GeomLine::setthick(int t){
	thick=t;
}
inline int GeomLine::getthick(void){
	return thick;
}
inline void GeomLine::getcoord(int&_x1,int&_y1,int&_x2,int&_y2){
	_x1=x1; _y1=y1; _x2=x2; _y2=y2;
}
inline void GeomLine::setcoord(int _x1,int _y1,int _x2,int _y2){
	x1=_x1; y1=_y1; x2=_x2; y2=_y2;
}
inline void GeomLine::getsname(char*s)
{
	sprintf(s,"Line(%.1f, %.1f; %.1f, %.1f; thick=%.1f)",
	        (float)x1/10,(float)y1/10,(float)x2/10,(float)y2/10,(float)thick/10);
}
inline void GeomLine::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
}
inline void GeomLine::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void GeomLine::move(int dx,int dy){
	x1+=dx; x2+=dx; y1+=dy; y2+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}
inline void GeomLine::updated(void)
{
	_upd=FALSE;
}
inline void GeomLine::changed(void)
{
	_upd=TRUE;
}
inline BOOL GeomLine::isChanged(void)
{
	return _upd;
}
inline HWND GeomLine::getmainwnd(void){
	return hMainWnd;
}
inline void GeomLine::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}

