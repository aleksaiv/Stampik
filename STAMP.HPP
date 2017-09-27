//File:stamp.hpp//

#define STAMP_WIDTH 1000
#define STAMP_HEIGHT 1000
#define PREVIEW_WIDTH 500
#define PREVIEW_HEIGHT 500
#define STAMP_VERSION 0x101

#define WM_SETSTATUSTEXT WM_USER+10
//messages from mainwindow to editor
#define WM_MOVEOBJECT WM_USER+11
//messages to main window from stamp
#define WM_STAMPINSERTED WM_USER+12
#define WM_STAMPREMOVED WM_USER+13
// ...  from panel
#define WM_PANELINSERTED WM_USER+14
#define WM_PANELSELECTED WM_USER+15
#define WM_PANELMENU WM_USER+16
// ... from preview
#define WM_PREVIEWSELECTED WM_USER+17
#define WM_PREVIEWMOVEOBJECT WM_USER+18
#define WM_PREVIEWMOVECONTOUR WM_USER+19
// messages from editor
#define WM_EDITORHIDECONTOUR WM_USER+20
#define WM_EDITORSHOWCONTOUR WM_USER+21
#define WM_EDITORUPDATE WM_USER+22
#define WM_EDITORDELETEOBJECT WM_USER+23
#define WM_EDITORCLOSE WM_USER+24
// messages to editor window
#define WM_EDITEND WM_USER+25
#define WM_UPDATECOORDS WM_USER+26


extern HINSTANCE hInst;

typedef struct tagSTAMPHEADER
{
	long id;
	short version;
	BOOL inverse;
	DWORD showmask;
	char preview;
	DWORD previewOFF;
	DWORD dataOFF;
	int numobj;
} STAMPHEADER,*PSTAMPHEADER;
typedef struct
{
	HDC hDC;
	int c;
} DTLPDATA,FAR* LPDTLPDATA;
typedef struct
{
	BOOL preview;
	BOOL previewstat;
	HBITMAP __hBitmapPreview;
} OFNCUSTDATA,FAR*POFNCUSTDATA;

class StampNode : public ListNode
{
short type,group;
public:
StampNode();
void settype(short t);
void setgroup(short g);
short gettype(void);
short getgroup(void);
DLGPROC getdlgproc(void);
virtual HWND starteditor(HWND);
virtual BOOL endeditor(void);
virtual BOOL save(HANDLE);
virtual BOOL load(HANDLE);
virtual void getstype(char* str);
virtual void getsgroup(char* str);
virtual void getsname(char* str);
virtual HBITMAP geticon(void);
virtual int geticonx(void);
virtual int geticony(void);
virtual ~StampNode();
virtual BOOL hittest(int,int);
virtual void draw(HDC hDC);
virtual void print(HDC hDC);
virtual void drawcontour(HDC hDC);
virtual void calcrect(RECT*,HDC hDC=NULL);
virtual void move(int,int);
virtual void drag(int,int);
};
inline void StampNode::move(int dx,int dy){
}
inline void StampNode::drag(int dx,int dy){
}
inline BOOL StampNode::hittest(int x,int y)
{
	return FALSE;
}
inline void StampNode::print(HDC hDC)
{
	draw(hDC);
}
inline void StampNode::drawcontour(HDC hDC)
{
}
inline void StampNode::calcrect(RECT*r,HDC hDC)
{
	r->left=r->right=r->bottom=r->top=0;
}
extern HBITMAP hMainBitmaps;
inline HBITMAP StampNode::geticon(void)
{
	return NULL;
}
inline int StampNode::geticonx(void)
{
	return 0;
}
inline int StampNode::geticony(void)
{
	return 0;
}
inline BOOL StampNode::load(HANDLE hFile)
{
	return FALSE;
}
inline BOOL StampNode::save(HANDLE hFile)
{
	return FALSE;
}
inline StampNode::~StampNode(){
}
inline StampNode::StampNode(){
}

inline void StampNode::draw(HDC hDC){
}

inline void StampNode::getstype(char* str)
{
	strcpy(str,"unknown");
}
inline void StampNode::getsgroup(char* str)
{
	strcpy(str,"unknown");
}
inline void StampNode::getsname(char *str)
{
	strcpy(str,"StampNode");
}
inline void StampNode::settype(short t)
{
	type=t;
}
inline void StampNode::setgroup(short g)
{
	group=g;
}
inline short StampNode::gettype(void)
{
	return type;
}
inline short StampNode::getgroup(void)
{
	return group;
}

extern HWND WndMain;
typedef StampNode* (FAR* LoaderProc)(void);
class StampClassNode : public ListNode
{
short t,g;
int x,y;
HBITMAP hBmp;
char* name;
LoaderProc ldrProc;
public:
StampClassNode(short t,short g,char*,HBITMAP bmp,int x,int y,LoaderProc ldrPrc);
~StampClassNode();
int getx(void);
int gety(void);
short gettype(void);
short getgroup(void);
char* getname(void);
StampNode* Load(HANDLE);
HBITMAP getbitmap(void);
};
inline StampNode* StampClassNode::Load(HANDLE hFile)
{
	if(ldrProc==NULL) return NULL;
	StampNode* sn;
	sn=ldrProc();
	if(sn==NULL) return NULL;
	sn->load(hFile);
	return sn;
}
inline StampClassNode::~StampClassNode()
{
	delete name;
}
inline StampClassNode::StampClassNode(short _t,short _g,
                                      char* n,HBITMAP bmp,int _x,int _y,
                                      LoaderProc ldrPrc)
{
	t=_t; g=_g; x=_x; y=_y;
	hBmp=bmp;
	name=new char[lstrlen(n)+1];
	strcpy(name,n);
	ldrProc=ldrPrc;
}
inline char* StampClassNode::getname(void){
	return name;
}
inline int StampClassNode::getx(void){
	return x;
}
inline int StampClassNode::gety(void){
	return y;
}
inline HBITMAP StampClassNode::getbitmap(void){
	return hBmp;
}
inline short StampClassNode::gettype(void){
	return t;
}
inline short StampClassNode::getgroup(void){
	return g;
}

class StampClass : public List
{
public:
StampClass();
};
inline StampClass::StampClass(){
}

class Stamp : public List
{
HWND hMainWnd;
BOOL _changed;             // Были ли внесены изменения
char *name;                // Название штампа
char *fname;               // Имя файла
HBITMAP bmpPreview;        // Хранит картинку для предварительного просмотра файла
short version;             // Версия штампа
DWORD showmask;            // Определяет какие объекты необходимо отображать
BOOL inverse;              // Разрешена ли иверсия объектов при их наложении
void readTypeGroup(HANDLE,short&,short&);        // Читает из файла тип и группу объекта
void writeTypeGroup(HANDLE,short,short);          // Записывает в файл тип и группу
StampClass* classes;
HANDLE hDevModes;
HANDLE hDevNames;
public:
int save_file(char *fn);   // Сохраняет штамп в файл с именем "fn"
int open_file(char *fn);   // Загружает из файла fn штамп
Stamp(char *n=NULL,HWND hWnd=NULL);      // Конструктор
virtual ~Stamp();          // Деструктор
void change(void);         // Устанавливает флаг _changed, что означает внесение изменений
BOOL isChanged(void);      // Проверяет, были ли внесены изменения
void updated(void);
void setver(short ver);    // Устанавливает версию файла штампа
short getver(void);        // Возвращает версию
int save(void);            // Сохраняет штамп, либо вызывает saveas если не задано имя файла
int saveas(void);          // Запрашивает имя файла и вызывает save
int open(char*defname=NULL);            // Запращиввет имя файла и загружает из него штамп
int close(void);           // Удаляет все объекты, т.е. создает новый штамп
char* getname(void);       // Получает название штампа
char* getfname(void);      // Получает имя файла
void setname(char*);       // Устанавливает название штампа
void setfname(char*);      // Присваивает новое имя файла
int numberofobjects(void);    // Определяет количество объектов в штампе
void generatepreview(void);   // Создает картинку для предварительного просмотра
DWORD getshowmask(void);
void setshowmask(DWORD);
void showhidetype(short);
BOOL showthistype(short);
virtual int saveobject(HANDLE,StampNode*);           // Сохраняет в файл объект
virtual StampNode* loadobject(HANDLE,short,short);   // Загружает из файла объект
virtual void draw(HDC);    // Отрисовывает в hDC объекты штампа
virtual void calcrect(RECT*);
virtual void print_stamp(HDC);
virtual void insertdummy(ListNode*);
virtual void removedummy(ListNode*);
BOOL RegisterClass(short g,short t,char*,HBITMAP bmp,int x,int y,LoaderProc);
StampClassNode* findclass(short g,short t);
HBITMAP geticon(short g,short t);
int geticony(short g,short t);
int geticonx(short g,short t);
char* getgtname(short g,short t);
BOOL previewExist(void);
void setpreview(BOOL s);

BOOL inverted(void);
void setinvert(BOOL i);
};
inline BOOL Stamp::previewExist(void)
{
	if(bmpPreview) return TRUE; else return FALSE;
}
inline void Stamp::setpreview(BOOL s)
{
	if(s==FALSE) {
		if(bmpPreview) DeleteObject(bmpPreview);
		bmpPreview=NULL;
	}else{
		if(!bmpPreview) generatepreview();
	}
}
inline void Stamp::setinvert(BOOL i)
{
	inverse=i;
}
inline BOOL Stamp::inverted(void)
{
	return inverse;
}
inline BOOL Stamp::RegisterClass(short g,short t,
                                 char*n,HBITMAP bmp,int x,int y,
                                 LoaderProc ldrProc=NULL)
{
	if(findclass(g,t)) return FALSE;
	classes->insert(new StampClassNode(t,g,n,bmp,x,y,ldrProc));
	return TRUE;
}
inline HBITMAP Stamp::geticon(short g,short t)
{
	StampClassNode* scn=findclass(g,t);
	if(!scn) return NULL;
	return scn->getbitmap();
}
inline char* Stamp::getgtname(short g,short t)
{
	StampClassNode* scn=findclass(g,t);
	if(!scn) return NULL;
	return scn->getname();
}
inline int Stamp::geticonx(short g,short t)
{
	StampClassNode* scn=findclass(g,t);
	if(!scn) return NULL;
	return scn->getx();
}
inline int Stamp::geticony(short g,short t)
{
	StampClassNode* scn=findclass(g,t);
	if(!scn) return NULL;
	return scn->gety();
}
inline void Stamp::insertdummy(ListNode* ln)
{
	SendMessage(hMainWnd,WM_STAMPINSERTED,0,(LPARAM)ln);
}
inline void Stamp::removedummy(ListNode* ln)
{
	SendMessage(hMainWnd,WM_STAMPREMOVED,0,(LPARAM)ln);
}
inline DWORD Stamp::getshowmask(void)
{
	return showmask;
}
inline void Stamp::setshowmask(DWORD m)
{
	showmask=m;
}
inline void Stamp::setname(char* n)
{
	if(name) delete name;
	name=new char[lstrlen(n)+1];
	strcpy(name,n);
}
inline void Stamp::setfname(char* n)
{
	if(fname) delete fname;
	fname=new char[lstrlen(n)+1];
	strcpy(fname,n);
}
inline char* Stamp::getname(void)
{
	return name;
}
inline char* Stamp::getfname(void)
{
	return fname;
}
inline void Stamp::setver(short ver)
{
	version=ver;
	change();
}
inline short Stamp::getver(void)
{
	return version;
}
inline void Stamp::updated(void){
	_changed=FALSE;
}
inline void Stamp::change(void)
{
	_changed=TRUE;
}
inline BOOL Stamp::isChanged(void)
{
	return _changed;
}
inline void Stamp::readTypeGroup(HANDLE hFile,short& t,short& g)
{
	DWORD dwRead;
	short buf[2];
	ReadFile(hFile,buf,sizeof(buf),&dwRead,NULL);
	t=buf[0];
	g=buf[1];
}
inline void Stamp::writeTypeGroup(HANDLE hFile,short t,short g)
{
	DWORD dwWritten;
	short buf[2];
	buf[0]=t; buf[1]=g;
	WriteFile(hFile,buf,sizeof(buf),&dwWritten,NULL);
}

LRESULT CALLBACK DlgSmallPreviewWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
HBITMAP ReadStampNamePreview(char* fn,char* name);
long WriteStampPreview(HANDLE hFile,HBITMAP hBmp);

