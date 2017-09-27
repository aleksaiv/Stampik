//File:ltext.hpp//

class LineText : public StampNode
{
char *text;
int offx,offy;
int h,thick;
int align,angle;
int width,space;
char fname[LF_FACESIZE];
char forprint;
BOOL italic,underline,bold,bthick;
RECT cont,contdescent;
HWND hMainWnd,hWnd;
BOOL _upd;
public:
LineText();
HWND getmainwnd(void);
void getoffset(int&,int&);
void setthick(int t);
void setbthick(BOOL b);
void cleartextptr(void);
void settext(char*);
char* gettext(void);
void setfname(char*);
char* getfname(void);
void setheight(int _h);
int getheight(void);
void setspace(int);
virtual ~LineText();
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

inline void LineText::setangle(int s)
{
	angle=s;
}
inline void LineText::setspace(int s)
{
	space=s;
}
inline void LineText::setwidth(int ww)
{
	width=ww;
}
inline void LineText::setthick(int t)
{
	thick=t;
}
inline void LineText::setbthick(BOOL b)
{
	bthick=b;
}
inline void LineText::setheight(int _h)
{
	h=_h;
}
inline int LineText::getheight(void)
{
	return h;
}
inline char* LineText::getfname(void)
{
	return fname;
}
inline void LineText::setfname(char*f)
{
	strcpy(fname,f);
}
inline int LineText::getstyle(void){
	int ret=0;
	if(bold) ret|=1; if(italic) ret|=2; if(underline) ret|=4;
	return ret;
}
inline void LineText::setstyle(long st){
	if(st&1) bold=TRUE; else bold=FALSE;
	if(st&2) italic=TRUE; else italic=FALSE;
	if(st&4) underline=TRUE; else underline=FALSE;
}
inline void LineText::setalign(long a){
	align=(int)a;
}
inline void LineText::updated(void){
	_upd=FALSE;
}
inline void LineText::changed(void){
	_upd=TRUE;
}
inline BOOL LineText::isChanged(void){
	return _upd;
}
inline void LineText::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
	if(_upd) SendMessage(hMainWnd,WM_EDITORUPDATE,0,(LPARAM)this);
	_upd=FALSE;
}
inline void LineText::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}
inline void LineText::getoffset(int&x,int&y)
{
	x=offx; y=offy;
}
inline void LineText::setoffset(int x,int y)
{
	offx=x; offy=y;
}
inline HWND LineText::getmainwnd(void)
{
	return hMainWnd;
}
inline void LineText::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void LineText::move(int dx,int dy){
	offx+=dx; offy+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}

inline void LineText::getsname(char* s)
{
	if(lstrlen(text)>30) {
		strncpy(s,text,30); s[30]='.'; s[31]='.'; s[32]='.'; s[33]=0;
	}else strcpy(s,text);
}

inline void LineText::cleartextptr(void)
{
	text=NULL;
}
inline char* LineText::gettext(void)
{
	return text;
}
