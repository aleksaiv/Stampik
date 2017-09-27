//File: list.cpp//

#define NULL 0l
#include "list.hpp"

List::~List()
{
	ListNode*n;
	n=first();
	while(n) {
		n=n->_next;
		delete _win;
		_win=n;
	}
	_head=_win=NULL;
}
ListNode* List::next(void)
{
	if(_win->_next) return _win=_win->_next; else return NULL;
}
ListNode* List::prev(void)
{
	if(_win->_prev) return _win=_win->_prev; else return NULL;
}
ListNode* List::getlast(void)
{
	ListNode*n=_win;
	while(n->_next) n=n->_next;
	return n;
}
short List::isEmpty(void)
{
	if(_win==NULL) return 1; else return 0;
}
short List::isFirst(void)
{
	if(_win==_head) return 1; else return 0;
}
short List::isLast(void)
{
	if(_win==getlast()) return 1; else return 0;
}
void List::insert(ListNode *ln)
{
//  while(_lock>0);
//  _lock=1;
	ln->_next=NULL; ln->_prev=NULL;
	if(_win) _win->insert(ln); else _head=ln;
	_win=ln;
//  _lock=0;
	insertdummy(ln);
}
void List::append(ListNode *ln)
{
//  while(_lock>0);
//  _lock++;
	ln->_next=NULL; ln->_prev=NULL;
	if(_win) getlast()->insert(ln);
	else _head=_win=ln;
	_win=ln;
//  _lock--;if(_lock<0)_lock=0;
	appenddummy(ln);
}
void List::remove(void)
{
	if(_win==NULL) return;
//  while(_lock>0);
//  _lock++;
	ListNode *s=_win->_prev;
	if(s==NULL) s=_win->_next, _head=s;
	_win->remove();
	removedummy(_win);
	delete _win;
	_win=s;
//  _lock--;if(_lock<0)_lock=0;
}
void List::remove(ListNode *ln)
{
	if(_win==NULL) return;
//  while(_lock>0);
//  _lock++;
	if(ln==_head) _head=ln->_next;
	if(ln==_win) {
		if(_win->_prev!=NULL)
			_win=_win->_prev;
		else _win=_win->_next;
	}
	ln->remove();
	removedummy(ln);
	delete ln;
//  _lock--;if(_lock<0)_lock=0;
}
void List::clear(void)
{
	ListNode*n;
//  while(_lock>0);
//  _lock++;
	n=first();
	while(n) {
		n=_win->_next;
		_win->remove();
		removedummy(_win);
		delete _win;
		_win=n;
	}
	_head=_win=NULL;
//  _lock--;if(_lock<0)_lock=0;
}
