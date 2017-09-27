//File:panel.cpp//

#include <windows.h>
#include "stmp.rh"

#include "list.hpp"
#include "stamp.hpp"
#include "ctree.hpp"
#include "tree.hpp"
#include "panel.hpp"

#pragma argsused
void ControlPanel::redraw(void)
{
	if(tree) {
		tree->reconstruct();
		RedrawWindow(hTreeWnd,NULL,NULL,RDW_INVALIDATE);
	}
}

BOOL CALLBACK PanelWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG:

		break;
	case WM_COMMAND:
		if(wParam>=50&&wParam<=57)
			SendMessage((HWND)GetWindowLong(hWnd,DWL_USER),WM_PANELINSERTED,wParam-50,0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


