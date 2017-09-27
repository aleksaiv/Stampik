//File:ctree.hpp//

class CTreeNode
{
CTreeNode *_prev,*_next,*_child,*_owner;
public:
friend class CTree;
CTreeNode();
virtual ~CTreeNode();
};
inline CTreeNode::CTreeNode() : _prev(NULL),_next(NULL),_child(NULL),_owner(NULL)
{
}
class CTree
{
CTreeNode *win;
public:
CTree();
virtual ~CTree();
CTreeNode* up(void);
CTreeNode* down(void);
CTreeNode* next(void);
CTreeNode* prev(void);
CTreeNode* getowner(void);
CTreeNode* getchild(void);
CTreeNode* getprev(void);
CTreeNode* getnext(void);
CTreeNode* getwin(void);
CTreeNode* first(void);
CTreeNode* last(void);
void setwin(CTreeNode*);
short isEmpty(void);
CTreeNode* toroot(void);
void clear(void);
void seeknode(CTreeNode*);
void insert(CTreeNode*);
void insertchild(CTreeNode*);
void remove();
};

inline short CTree::isEmpty(void)
{
	if(win) return 0;
	else return 1;
}
inline CTreeNode* CTree::getowner(void)
{
	if(win)
		return win->_owner;
	else
		return NULL;
}
inline CTreeNode* CTree::getchild(void)
{
	if(win)
		return win->_child;
	else return NULL;
}
inline CTreeNode* CTree::getprev(void)
{
	if(win) return win->_prev;
	else return NULL;
}
inline CTreeNode* CTree::getnext(void)
{
	if(win) return win->_next;
	else return NULL;
}
inline CTree::CTree() : win(NULL)
{
}
inline CTreeNode* CTree::getwin(void)
{
	return win;
}
inline void CTree::setwin(CTreeNode* ctn)
{
	win=ctn;
}
