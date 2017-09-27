//File: cmytree.hpp//

LRESULT CALLBACK TreeHelpikWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK TreeWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
#define TREECLASSNAME "StampTreeWnd"

#define TM_OPEN WM_USER+1
#define TM_CLOSE WM_USER+2
#define TM_CLOSEALL WM_USER+3
#define TM_CLEAR WM_USER+4
#define TM_INSERT WM_USER+5
#define TM_REMOVE WM_USER+6
#define TM_DELETENODE WM_USER+7
#define TM_SHOWNODE WM_USER+8
#define TM_SELECTNODE WM_USER+9

inline void TreeOpen(HWND hWnd)
{
	SendMessage(hWnd,TM_OPEN,0,0l);
}
inline void TreeClose(HWND hWnd)
{
	SendMessage(hWnd,TM_CLOSE,0,0l);
}
inline void TreeCloseAll(HWND hWnd)
{
	SendMessage(hWnd,TM_CLOSEALL,0,0l);
}
inline void TreeClear(HWND hWnd)
{
	SendMessage(hWnd,TM_CLEAR,0,0l);
}
inline void TreeRemove(HWND hWnd,ListNode *n)
{
	SendMessage(hWnd,TM_REMOVE,0,(LPARAM)n);
}
inline void TreeInsert(HWND hWnd,ListNode *n)
{
	SendMessage(hWnd,TM_INSERT,0,(LPARAM)n);
}
class CMyTreeNode : public CTreeNode
{
char *text;
char type,group;
ListNode *lnode;
public:
bool _opened;
CMyTreeNode(char*str,ListNode *ln,ListNode *child=NULL){
	text=new char[strlen(str)+1];
	strcpy(text,str);
	lnode=ln;
	if(ln) {
		type=ln->GetType();
		group=ln->GetGroup();
	}else if(child) {
		type=child->GetType();
		group=child->GetGroup();
	}
}
char GetType(void){
	return type;
}
char GetGroup(void){
	return group;
}
virtual ~CMyTreeNode(){
	delete text;
}
char *gettext(void){
	return text;
}
ListNode *getlnode(void){
	return lnode;
}
};
class CMyTree : public CTree
{
public:
CMyTree(){
}
virtual ~CMyTree();
bool seekNode(char *);
void create(class CStamp*);
void ins(ListNode*n);
void del(ListNode*n);
void expand(void){
	((CMyTreeNode*)getnode())->_opened=TRUE;
}
void shrink(void){
	((CMyTreeNode*)getnode())->_opened=FALSE;
}
bool opened(void){
	return ((CMyTreeNode*)getnode())->_opened;
}
};
