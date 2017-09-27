//File:list.hpp//

class ListNode
{
ListNode *_prev,*_next;
public:
friend class List;
ListNode();
virtual void insert(ListNode* ln);
virtual void remove(void);
virtual ~ListNode();
};
inline ListNode::ListNode() : _prev(NULL),_next(NULL)
{
}
inline ListNode::~ListNode()
{
}
inline void ListNode::insert(ListNode* ln)
{
	if(_next) _next->_prev=ln;
	ln->_prev=this; ln->_next=_next;
	_next=ln;
}
inline void ListNode::remove(void)
{
	if(_prev) _prev->_next=_next;
	if(_next) _next->_prev=_prev;
	_next=_prev=NULL;
}

class List
{
ListNode* _head;
ListNode* _win;
public:
List();
virtual ~List();
virtual void insert(ListNode* ln);
virtual void append(ListNode* ln);
virtual void remove(ListNode* ln);
virtual void remove(void);
void clear(void);
virtual void insertdummy(ListNode* ln){
}
virtual void appenddummy(ListNode* ln){
}
virtual void removedummy(ListNode* ln){
}
ListNode* first(void);
ListNode* last(void);
ListNode* next(void);
ListNode* prev(void);
ListNode* getnext(void);
ListNode* getprev(void);
ListNode* getfirst(void);
ListNode* getlast(void);
ListNode* getwin(void);
void setwin(ListNode* ln);
short isFirst(void);
short isLast(void);
short isEmpty(void);
};

inline List::List() : _head(NULL),_win(NULL) {
}

inline ListNode* List::first(void)
{
	_win=_head;
	return _win;
}
inline ListNode* List::last(void)
{
	_win=getlast();
	return _win;
}
inline ListNode* List::getnext(void)
{
	return _win->_next;
}
inline ListNode* List::getprev(void)
{
	return _win->_prev;
}
inline ListNode* List::getwin(void)
{
	return _win;
}
inline void List::setwin(ListNode* ln)
{
	_win=ln;
}
inline ListNode* List::getfirst(void)
{
	return _head;
}

