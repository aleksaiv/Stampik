//File:ttext.cpp//

#include <windows.h>

#include "list.hpp"
#include "stamp.hpp"
#include "ttext.hpp"

TriangleText::TriangleText() : text(NULL) {
	settype(2);
	setgroup(1);
}
TriangleText::~TriangleText()
{
	if(text) delete text;
}
BOOL TriangleText::save(HANDLE hFile)
{
	short length=0,t,g;
	DWORD dwWritten;
	length=sizeof(t)+sizeof(g)+sizeof(length)+lstrlen(text);
	WriteFile(hFile,&length,sizeof(length),&dwWritten,NULL);
	if(text) length=(short)lstrlen(text);
	WriteFile(hFile,&length,sizeof(short),&dwWritten,NULL);
	if(text) WriteFile(hFile,text,length,&dwWritten,NULL);
	t=gettype(); g=getgroup();
	WriteFile(hFile,&t,sizeof(t),&dwWritten,NULL);
	WriteFile(hFile,&g,sizeof(g),&dwWritten,NULL);
}
BOOL TriangleText::load(HANDLE hFile)
{
	short length=0,t;
	DWORD tg;
	DWORD dwReaded;
	if(text) delete text;
	ReadFile(hFile,&length,sizeof(short),&dwReaded,NULL);
	ReadFile(hFile,&length,sizeof(short),&dwReaded,NULL);
	if(length>0) {
		text=new char[length+1];
		ReadFile(hFile,text,length,&dwReaded,NULL); text[length]=0;
	}
	ReadFile(hFile,&tg,sizeof(DWORD),&dwReaded,NULL);
	settype(LOWORD(tg));
	setgroup(HIWORD(tg));
}

void TriangleText::settext(char* t)
{
	if(text) delete text;
	if(!t) return;
	text=new char[lstrlen(t)+1];
	strcpy(text,t);
}

