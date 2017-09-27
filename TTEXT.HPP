//File:ttext.hpp//

class TriangleText : public StampNode
{
char *text;
public:
TriangleText();
void cleartextptr(void);
void settext(char*);
char* gettext(void);
virtual ~TriangleText();
virtual BOOL load(HANDLE);
virtual BOOL save(HANDLE);
virtual void getsname(char*);
};

inline void TriangleText::getsname(char* s)
{
	strcpy(s,text);
}

inline void TriangleText::cleartextptr(void)
{
	text=NULL;
}

inline char* TriangleText::gettext(void)
{
	return text;
}
