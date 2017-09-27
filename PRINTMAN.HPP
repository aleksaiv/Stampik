//File:printman.hpp//

extern HINSTANCE hInst;
LRESULT CALLBACK PrintManagerPreviewProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

class PMListNode : public ListNode
{
char* fname;
char* name;
int id;
RECT rect;
POINT offset;
public:
PMListNode(char*,char*,RECT*,POINT*);
virtual ~PMListNode();
char* getname(void);
char* getfname(void);
int getid(void);
void setid(int);
void getrect(RECT*);
void getoffset(POINT*);
void setoffset(POINT);
void setoffset(int,int);
};
inline void PMListNode::setoffset(int _x,int _y)
{
	offset.x=_x; offset.y=_y;
}
inline void PMListNode::setoffset(POINT pt)
{
	offset.x=pt.x; offset.y=pt.y;
}
inline void PMListNode::getrect(RECT*r)
{
	CopyMemory(r,&rect,sizeof(RECT));
}
inline void PMListNode::getoffset(POINT* pt)
{
	pt->x=offset.x; pt->y=offset.y;
}
inline int PMListNode::getid(void)
{
	return id;
}
inline void PMListNode::setid(int i)
{
	id=i;
}
inline PMListNode::PMListNode(char*filename,char*n,RECT*r,POINT*p)
{
	fname=new char[lstrlen(filename)+1];
	strcpy(fname,filename);
	name=new char[lstrlen(n)+1];
	strcpy(name,n);
	CopyMemory(&rect,r,sizeof(RECT));
	CopyMemory(&offset,p,sizeof(POINT));
}
inline PMListNode::~PMListNode()
{
	delete fname;
}
inline char* PMListNode::getfname(void)
{
	return fname;
}
inline char* PMListNode::getname(void)
{
	return name;
}

class PMList : public List
{
HWND hWnd;
public:
PMList(HWND);
void setwnd(HWND);
virtual void insertdummy(ListNode* ln);
virtual void removedummy(ListNode* ln);
};
inline PMList::PMList(HWND hlw)
{
	hWnd=hlw;
}
inline void PMList::setwnd(HWND w)
{
	hWnd=w;
}

class PrintManager
{
HWND hWnd;
HANDLE hDevMode;
HANDLE hDevNames;
BOOL _changed;
char *fname;
public:
float zoom;
char moved;
int pagewidth,pageheight;
int pwidth,pheight;
int mx,my,oldx,oldy;
BOOL capt;
PMList* pmlist;
PrintManager();
virtual ~PrintManager();
void pagetoscreen(POINT*);
void screentopage(POINT*);
void getpagesize(POINT*);
void Changed(void);
void setwnd(HWND);
HWND getwnd(void);
void getprintername(char*s);
void setupprinter(void);
void drawpage(HDC hDC);
void append(void);
void remove(void);
BOOL clear(void);
BOOL open(void);
BOOL save(void);
BOOL saveas(void);
void print(void);
void listsel(int id);
BOOL savefile(char*);
BOOL openfile(char*);
void getdefaultprinter(void);
BOOL intersect(void);
};
inline void PrintManager::Changed(void)
{
	_changed=TRUE;
}
inline void PrintManager::setwnd(HWND w)
{
	hWnd=w;
	pmlist->setwnd(GetDlgItem(w,IDC_FILES));
}
inline PrintManager::~PrintManager()
{
	if(fname) delete fname;
	delete pmlist;
	if(hDevMode) GlobalFree(hDevMode);
	if(hDevNames) GlobalFree(hDevNames);
}
inline HWND PrintManager::getwnd(void)
{
	return hWnd;
}
inline void PrintManager::getpagesize(POINT*p)
{
	p->x=pagewidth; p->y=pageheight;
}
void StartPrintManager(void);
