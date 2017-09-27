//File:tree.hpp//

extern HBITMAP hMainIconBitmap;
LRESULT CALLBACK TreeHelpikWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK TreeWndProc(HWND,UINT,WPARAM,LPARAM);
class TreeNode : public CTreeNode
{
StampNode* sn;
short opened;
short type;
public:
void setnode(StampNode*);
StampNode* getnode(void);
virtual HBITMAP geticon(void);
virtual int geticonx(void);
virtual int geticony(void);
virtual void gettext(char*);
virtual short gettype(void);
short isOpened(void);
void expand(void);
void shrink(void);
TreeNode(StampNode*,short type);
};
inline short TreeNode::isOpened(void)
{
	return opened;
}
inline void TreeNode::expand(void)
{
	if(opened==0) opened=1;
}
inline void TreeNode::shrink(void)
{
	if(opened==1) opened=0;
}
inline TreeNode::TreeNode(StampNode* s,short t=0)
{
	sn=s;
	type=t;
	if(s==NULL)
		opened=0;
	else
		opened=-1; // this is a group or a node?
}
inline HBITMAP TreeNode::geticon(void)
{
	if(sn) return sn->geticon(); else return NULL;
}
inline int TreeNode::geticonx(void)
{
	if(sn) return sn->geticonx(); else return 0;
}
inline int TreeNode::geticony(void)
{
	if(sn) return sn->geticony(); else return 0;
}
inline void TreeNode::setnode(StampNode* s)
{
	sn=s;
}
inline StampNode* TreeNode::getnode(void)
{
	return sn;
}
inline short TreeNode::gettype(void)
{
	return type;
}
inline void TreeNode::gettext(char* txt)
{
	if(sn) sn->getsname(txt);
}

class TreeGroupNode : public TreeNode
{
Stamp* stmp;
short g;
public:
TreeGroupNode(Stamp*,StampNode*);
short getval(void);
virtual void gettext(char*);
virtual HBITMAP geticon(void);
virtual int geticonx(void);
virtual int geticony(void);
};
inline HBITMAP TreeGroupNode::geticon(void)
{
	return stmp->geticon(g,-1);
}
inline int TreeGroupNode::geticonx(void)
{
	return stmp->geticonx(g,-1);
}
inline int TreeGroupNode::geticony(void)
{
	return stmp->geticony(g,-1);
}
inline TreeGroupNode::TreeGroupNode(Stamp* s,StampNode* sn) : TreeNode(NULL,2)
{
	stmp=s;
	g=sn->getgroup();
}
inline short TreeGroupNode::getval(void)
{
	return g;
}
inline void TreeGroupNode::gettext(char* buf)
{
	char* tt=stmp->getgtname(g,-1);
	if(tt!=NULL) strcpy(buf,tt); else strcpy(buf,"");
}
class TreeTypeNode : public TreeNode
{
Stamp* stmp;
short t;
public:
TreeTypeNode(Stamp*,StampNode*);
short getval(void);
virtual void gettext(char*);
virtual HBITMAP geticon(void);
virtual int geticonx(void);
virtual int geticony(void);
};
inline TreeTypeNode::TreeTypeNode(Stamp* s,StampNode* sn) : TreeNode(NULL,3)
{
	stmp=s;
	t=sn->gettype();
}
inline HBITMAP TreeTypeNode::geticon(void)
{
	return stmp->geticon(-1,t);
}
inline int TreeTypeNode::geticonx(void)
{
	return stmp->geticonx(-1,t);
}
inline int TreeTypeNode::geticony(void)
{
	return stmp->geticony(-1,t);
}
inline short TreeTypeNode::getval(void)
{
	return t;
}
inline void TreeTypeNode::gettext(char* buf)
{
	char* tt=stmp->getgtname(-1,t);
	if(tt!=NULL) strcpy(buf,tt); else strcpy(buf,"");
}

class HeaderNode : public TreeNode
{
Stamp* stmp;
public:
HeaderNode(Stamp* s);
virtual void gettext(char*);
virtual int geticonx(void);
virtual int geticony(void);
virtual HBITMAP geticon(void);
};
inline HeaderNode::HeaderNode(Stamp* s) : TreeNode(NULL,1)
{
	stmp=s;
}
inline void HeaderNode::gettext(char* str)
{
	strcpy(str,stmp->getname());
}
inline HBITMAP HeaderNode::geticon(void)
{
	return hMainBitmaps;
}
inline int HeaderNode::geticonx(void)
{
	return 192;
}
inline int HeaderNode::geticony(void)
{
	return 0;
}

class TreeListNode : public ListNode
{
TreeNode* tn;
RECT rect;
public:
TreeListNode(TreeNode*,RECT*);
TreeNode* getnode(void);
void getrect(RECT*);
};
inline void TreeListNode::getrect(RECT* r)
{
	CopyMemory(r,&rect,sizeof(RECT));
}
inline TreeListNode::TreeListNode(TreeNode* n,RECT* r)
{
	tn=n;
	rect.left=r->left; rect.top=r->top;
	rect.right=r->right; rect.bottom=r->bottom;
}
inline TreeNode* TreeListNode::getnode(void)
{
	return tn;
}

class TreeList : public List
{
int num;
public:
TreeList();
short getoff(void);
void setoff(short);
int size(void);
void setsize(int);
};
inline TreeList::TreeList(){
}
inline void TreeList::setsize(int n)
{
	num=n;
}

class Tree : public CTree {
Stamp* stmp;
HINSTANCE hInst;
HWND hMainWnd;
HWND hTipsWnd;
HWND hTreeWnd;
HBITMAP bmps;
HFONT hFont;
int yoff;
TreeList* tlist;
int wndpos;
public:
Tree(HINSTANCE,HWND,HWND,Stamp*);
~Tree();
int nextgroup(void);
int prevgroup(void);
void expand(void);
void shrink(void);
void reconstruct(void);
void initlist(void);
void insertobject(StampNode*);
void removeobject(StampNode*);
BOOL findgroup(short);
BOOL findtype(short);
void seekobject(StampNode*);
HWND GetMainWnd(void);
void draw(HDC);
void keyup(void);
void keydown(void);
void keyleft(void);
void keyright(void);
void keyenter(void);
void setkillfocus(void);
void rclick(int x,int y);
void lclick(int x,int y);
void ldblclk(int x,int y);
void vscroll(int wParam);
void mousemove(int x,int y);
void keymenu(void);
};
inline HWND Tree::GetMainWnd(void){
	return hMainWnd;
}
inline Tree::Tree(HINSTANCE hI,HWND main,HWND my,Stamp* s)
{
	hInst=hI;
	hMainWnd=main; hTreeWnd=my;
	SetWindowLong(hTreeWnd,GWL_USERDATA,(LONG)this);
	tlist=new TreeList;
	insert(new HeaderNode(s));
	bmps=LoadBitmap(hI,MAKEINTRESOURCE(IDB_TREEBITMAPS));
	hFont=CreateFont(10,0,0,0,0,
	                 FALSE,FALSE,FALSE,
	                 ANSI_CHARSET,
	                 OUT_DEFAULT_PRECIS,
	                 CLIP_DEFAULT_PRECIS,
	                 DEFAULT_QUALITY,
	                 DEFAULT_PITCH|FF_DONTCARE,
	                 "MS Sans Serif");
	stmp=s;
	initlist();
	hTipsWnd=CreateWindow("StampTipsWnd","no text",
	                      WS_POPUP,
	                      0,0,10,10,
	                      hMainWnd,NULL,hInst,NULL
	                      );
	wndpos=-1;
}

inline Tree::~Tree()
{
	DestroyWindow(hTipsWnd);
	DeleteObject(bmps);
	DeleteObject(hFont);
}
