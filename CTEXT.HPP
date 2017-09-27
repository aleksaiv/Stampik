//File:ctext.hpp//

class CircleText : public StampNode
{
char *text;
int offx,offy;
int h,thick;
int align,angle;
int width,space,radius;
BOOL org;
char fname[LF_FACESIZE];
char forprint;
BOOL italic,underline,bold,bthick;
int conta,contb;
HWND hMainWnd,hWnd;
BOOL _upd;
public:
CircleText();
HWND getmainwnd(void);
void getoffset(int&,int&);
void setthick(int t);
void setbthick(BOOL b);
void cleartextptr(void);
void settext(char*);
void setradius(int);
int getradius(void);
BOOL getorg(void);
void setorg(BOOL);
char* gettext(void);
void setfname(char*);
char* getfname(void);
void setheight(int _h);
int getheight(void);
void setspace(int);
virtual ~CircleText();
virtual BOOL load(HANDLE);
virtual BOOL save(HANDLE);
virtual void draw(HDC);
virtual void print(HDC);
virtual void getsname(char*);
virtual void drawcontour(HDC);
virtual BOOL hittest(int,int);
virtual HWND starteditor(HWND);
virtual BOOL endeditor(void);
virtual void move(int,int);
virtual void drag(int,int);
virtual void calcrect(RECT*,HDC hDC=NULL);
void setoffset(int,int);
void update(void);
void changed(void);
void updated(void);
BOOL isChanged(void);
void del(void);
void close(void);
void calccontour(HDC hDC=NULL);
void initdialog(HWND);
void setstyle(long st);
void setalign(long a);
int getstyle(void);
void setwidth(int);
void setangle(int);
};

inline void CircleText::setorg(BOOL o){
	org=o;
}
inline BOOL CircleText::getorg(void){
	return org;
}
inline void CircleText::setradius(int r){
	radius=r;
}
inline int CircleText::getradius(void){
	return radius;
}
inline void CircleText::setangle(int s)
{
	angle=s;
}
inline void CircleText::setspace(int s)
{
	space=s;
}
inline void CircleText::setwidth(int ww)
{
	width=ww;
}
inline void CircleText::setthick(int t)
{
	thick=t;
}
inline void CircleText::setbthick(BOOL b)
{
	bthick=b;
}
inline void CircleText::setheight(int _h)
{
	h=_h;
}
inline int CircleText::getheight(void)
{
	return h;
}
inline char* CircleText::getfname(void)
{
	return fname;
}
inline void CircleText::setfname(char*f)
{
	strcpy(fname,f);
}
inline int CircleText::getstyle(void){
	int ret=0;
	if(bold) ret|=1; if(italic) ret|=2; if(underline) ret|=4;
	return ret;
}
inline void CircleText::setstyle(long st){
	if(st&1) bold=TRUE; else bold=FALSE;
	if(st&2) italic=TRUE; else italic=FALSE;
	if(st&4) underline=TRUE; else underline=FALSE;
}
inline void CircleText::setalign(long a){
	align=(int)a;
}
inline void CircleText::updated(void){
	_upd=FALSE;
}
inline void CircleText::changed(void){
	_upd=TRUE;
}
inline BOOL CircleText::isChanged(void){
	return _upd;
}
inline void CircleText::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
	if(_upd) SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
inline void CircleText::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}
inline void CircleText::getoffset(int&x,int&y)
{
	x=offx; y=offy;
}
inline void CircleText::setoffset(int x,int y)
{
	offx=x; offy=y;
}
inline HWND CircleText::getmainwnd(void)
{
	return hMainWnd;
}
inline void CircleText::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void CircleText::move(int dx,int dy){
	offx+=dx; offy+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}

inline void CircleText::getsname(char* s)
{
	if(lstrlen(text)>30) {
		strncpy(s,text,30); s[30]='.'; s[31]='.'; s[32]='.'; s[33]=0;
	}else strcpy(s,text);
}

inline void CircleText::cleartextptr(void)
{
	text=NULL;
}
inline char* CircleText::gettext(void)
{
	return text;
}
