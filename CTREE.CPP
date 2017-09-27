//File:ctree.cpp//

#define NULL 0l
#include "ctree.hpp"

CTreeNode::~CTreeNode()
{
	if(_child) delete _child;
	if(_prev) _prev->_next=_next;
	if(_next) _next->_prev=_prev;
}
void CTree::seeknode(CTreeNode*)
{
}

void CTree::insert(CTreeNode* tn)
{
	if(win)
	{
		tn->_owner=win->_owner;
		tn->_prev=win;
		tn->_next=win->_next;
		if(win->_next) win->_next->_prev=tn;
		win->_next=tn;
	}
	win=tn;
}
void CTree::insertchild(CTreeNode* tn)
{
	if(win==NULL) return;
	tn->_owner=win;
	if(win->_child) {win=win->_child; insert(tn);}
	else {win->_child=tn; win=tn;}
}
void CTree::remove(void)
{
	if(win==NULL) return;
	CTreeNode*s;
	if(win->_owner) {
		if(win->_owner->_child==win)
			win->_owner->_child=win->_next;
	}
	s=win;
	if(win->_prev)
		win=win->_prev;
	else
	if(win->_next)
		win=win->_next;
	else
		win=win->_owner;
	delete s;
}
CTreeNode* CTree::up(void)
{
	if(!win) return NULL;
	if(win->_owner==NULL) return NULL;
	win=win->_owner;
	return win;
}
CTreeNode* CTree::down(void)
{
	if(!win) return NULL;
	if(win->_child==NULL) return NULL;
	win=win->_child;
	return win;
}

CTreeNode* CTree::prev(void)
{
	if(win) if(win->_prev) win=win->_prev; else return NULL;
	return win;
}
CTreeNode* CTree::next(void)
{
	if(win) if(win->_next) win=win->_next; else return NULL;
	return win;
}

CTreeNode* CTree::toroot(void)
{
	if(!win) return NULL;
	while(win->_owner) win=win->_owner;
	return first();
}
CTreeNode* CTree::first(void)
{
	if(!win) return NULL;
	while(win->_prev) win=win->_prev;
	return win;
}
CTreeNode* CTree::last(void)
{
	if(!win) return NULL;
	while(win->_next) win=win->_next;
	return win;
}
CTree::~CTree()
{
	CTreeNode*n;
	if(toroot())
		do {
			n=win->_next;
			delete win;
			win=n;
		} while(n);
}

void CTree::clear(void)
{
	CTreeNode*n;
	if(toroot())
		do {
			n=win->_next;
			delete win;
			win=n;
		} while(n);
}
