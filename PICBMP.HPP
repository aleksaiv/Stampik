//File:picbmp.hpp//

BOOL CALLBACK PBitmapDlgProc(HWND,UINT,WPARAM,LPARAM);
class PicBitmap : public StampNode
{
int offx,offy,rad;
char*fname;
int w,h;
int zw,zh;
HBITMAP bmp;
HWND hWnd,hMainWnd;
BOOL _upd;
public:
PicBitmap();
PicBitmap(int ox,int oy,int r,char*fname);
virtual ~PicBitmap();
void setoffset(int,int);
void getoffset(int*,int*);
void setsize(int);
void getsize(int*);
void setwh(int,int);
void getwh(int&,int&);
char*getfname(void);
void setfname(char*);
int reload(void);
void resize(void);
HBITMAP getbitmap(void);
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
inline PicBitmap::~PicBitmap(void)
{
	if(bmp) DeleteObject(bmp);
}
inline void PicBitmap::setwh(int _w,int _h){
	w=_w; h=_h;
}
inline void PicBitmap::getwh(int&_w,int&_h)
{
	_w=w; _h=h;
}
inline void PicBitmap::resize(void)
{
	double z;
	if(rad==0) {
		zw=1; zh=1;
	}else{
		if(h+w==0) z=1; else
			z=sqrt((double)(h*h+w*w)/4.0)/(double)rad;
		zw=(double)w/z;
		zh=(double)h/z;
	}
}
inline char* PicBitmap::getfname(void)
{
	return fname;
}
inline void PicBitmap::setfname(char*f)
{
	if(fname) delete fname;
	if(f) {
		fname=new char[lstrlen(f)+1];
		strcpy(fname,f);
	}else fname=NULL;
}
inline HBITMAP PicBitmap::getbitmap(void)
{
	return bmp;
}
inline HWND PicBitmap::getmainwnd(void)
{
	return hMainWnd;
}
inline void PicBitmap::del(void)
{
	SendMessage(hMainWnd,WM_EDITORDELETEOBJECT,0,(LPARAM)this);
}
inline void PicBitmap::close(void)
{
	SendMessage(hMainWnd,WM_EDITORCLOSE,0,(LPARAM)this);
}
inline void PicBitmap::drag(int dx,int dy){
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,0,MAKELONG(dx,dy));
}
inline void PicBitmap::move(int dx,int dy){
	offx+=dx; offy+=dy;
	if(hWnd) SendMessage(hWnd,WM_MOVEOBJECT,1,MAKELONG(dx,dy));
}
inline PicBitmap::PicBitmap() : offx(0),offy(0),rad(100),fname(NULL),_upd(FALSE){
}
inline void PicBitmap::getsname(char* s)
{
	sprintf(s,"Bitmap('%s',at %.1f:%.1f, radius=%.1f)",
	        fname,(float)offx/10,(float)offy/10,(float)rad/10);
}

inline void PicBitmap::setsize(int _r)
{
	rad=_r;
}
inline void PicBitmap::getsize(int*_r)
{
	if(_r) *_r=rad;
}

inline void PicBitmap::setoffset(int _x,int _y)
{
	offx=_x; offy=_y;
}
inline void PicBitmap::getoffset(int*_x,int*_y)
{
	if(_x) *_x=offx;
	if(_y) *_y=offy;
}
inline void PicBitmap::updated(void)
{
	_upd=FALSE;
}
inline void PicBitmap::changed(void)
{
	_upd=TRUE;
}
inline BOOL PicBitmap::isChanged(void)
{
	return _upd;
}
