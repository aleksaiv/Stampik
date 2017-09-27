//File:panel.hpp//

BOOL CALLBACK PanelWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
class ControlPanel
{
HWND hMainWnd;
HWND hPanelWnd;
HWND hTreeWnd;
Stamp* stmp;
Tree* tree;
HINSTANCE hInst;
public:
ControlPanel(HINSTANCE,HWND,Stamp*);
void insertobject(StampNode*);
void removeobject(StampNode*);
void selectobject(StampNode*);
void nextobject(void);
void nextgroup(void);
void prevobject(void);
void prevgroup(void);
void update(void);
void redraw(void);
~ControlPanel();
};
inline void ControlPanel::selectobject(StampNode*sn)
{
	if(tree) tree->seekobject(sn);
}
inline void ControlPanel::update(void)
{
	tree->reconstruct();
}
inline void ControlPanel::insertobject(StampNode* sn)
{
	if(tree) tree->insertobject(sn);
}
inline void ControlPanel::removeobject(StampNode* sn)
{
	if(tree) tree->removeobject(sn);
}
inline ControlPanel::ControlPanel(HINSTANCE hI,HWND hMain,Stamp* s)
{
	hInst=hI;
	hPanelWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_PANEL),hMain,(DLGPROC)PanelWndProc);
	SetWindowLong(hPanelWnd,DWL_USER,(LONG)hMain);
	hTreeWnd=GetDlgItem(hPanelWnd,IDC_TREE);
	stmp=s;
	tree=new Tree(hI,hMain,hTreeWnd,stmp);
	hMainWnd=hMain;
}
inline ControlPanel::~ControlPanel()
{
	DestroyWindow(hPanelWnd);
	delete tree;
}

inline void ControlPanel::nextobject(void)
{
	if(tree) {
		if(!tree->next()) !tree->first();
		if(((TreeNode*)tree->getwin())->getnode()==NULL) {tree->reconstruct(); return;}
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
		SendMessage(hMainWnd,WM_PANELSELECTED,0,(LPARAM)(((TreeNode*)tree->getwin())->getnode()));
	}
}
inline void ControlPanel::prevobject(void)
{
	if(tree) {
		if(!tree->prev()) tree->last();
		if(((TreeNode*)tree->getwin())->getnode()==NULL) {tree->reconstruct(); return;}
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
		SendMessage(hMainWnd,WM_PANELSELECTED,0,(LPARAM)(((TreeNode*)tree->getwin())->getnode()));
	}
}
inline void ControlPanel::nextgroup(void)
{
	if(tree) {
		if(tree->nextgroup()==0) {
			RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(hMainWnd,WM_PANELSELECTED,0,(LPARAM)(((TreeNode*)tree->getwin())->getnode()));
		}
	}
}
inline void ControlPanel::prevgroup(void)
{
	if(tree) {
		if(tree->prevgroup()==0) {
			RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
			SendMessage(hMainWnd,WM_PANELSELECTED,0,(LPARAM)(((TreeNode*)tree->getwin())->getnode()));
		}
	}
}

