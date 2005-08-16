//////////////////////////////////////////////////////////////////////
// OTItemEditor
//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#define __FILE_REVISION "0.0.1 CVS"
#include "../client/definitions.hpp"
#include "resource.h"

#ifdef STRICT
#undef STRICT
#endif

#include "gui_win.hpp"

extern HINSTANCE g_instance;
extern Items *g_items;

#ifdef __OLDINCLUDE__
extern "C" WINGDIAPI BOOL  WINAPI TransparentBlt(IN HDC,IN int,IN int,IN int,IN int,IN HDC,IN int,IN int,IN int,IN int,IN UINT);
#endif
GUIWin::GUIWin()
{
};

void GUIWin::initGUI()
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icc);

	//InitCommonControls();
	DialogBoxParam(g_instance,MAKEINTRESOURCE(DLG_MAIN),NULL,reinterpret_cast<DLGPROC>(GUIWin::DlgProcMain),NULL);
}

bool GUIWin::loadSpriteInternal(const unsigned char *dump, const unsigned long size, InternalSprite *sprite)
{
	unsigned long i, state, pos;
	unsigned short npix;
	unsigned char tmprgb[32*32*3];
	
	unsigned char *rgb = new unsigned char[32*32*4];
	memset(tmprgb,0x11,32*32*3);

	state=0;
	pos=0;
	for(i=0; i < size;)
	{
		memcpy(&npix, dump+i, 2); // number of pixels (transparent or defined)
		i += 2;
		switch(state)
		{
		default: // state 0, drawing transparent pixels
			state=1;
			break;
		case 1: // state 1, drawing defined pixels
			ASSERT(pos*3 < 32*32*3);
			memcpy(tmprgb+(pos*3), dump+i, npix*3);
			i += npix*3;
			state=0;
			break;
		}
		pos += npix;
	}
	//reverse rgb
	unsigned long j;
	for(i=0; i<32;i++)
	{
		for(j=0; j < 32;j++){
			rgb[i*32*4+j*4] = tmprgb[(32-i-1)*32*3+j*3+2]; //blue
			rgb[i*32*4+j*4+1] = tmprgb[(32-i-1)*32*3+j*3+1];//green
			rgb[i*32*4+j*4+2] = tmprgb[(32-i-1)*32*3+j*3];//red
			rgb[i*32*4+j*4+3] = 0;
		}
	}
	
	*sprite = rgb;
	return true;
}


void GUIWin::loadSpriteInternalTransparent(unsigned long color,  InternalSprite *sprite)
{
	unsigned char *rgb = new unsigned char[32*32*4];
	int i,j;
	memset(rgb,0x11,32*32*4);
	for(i = 0; i < 32; i = i + 4){
		for(j = 0;j < 32; j = j + 4){
			*((unsigned long*)&rgb[(i*32+j)*4]) = color;
		}
	}
	*sprite = rgb;
}


void GUIWin::unloadSpriteInternal(InternalSprite sprite)
{
	//delete sprite;
}

LRESULT CALLBACK GUIWin::DlgProcMain(HWND h, UINT Msg,WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		createEditorTree(GetDlgItem(h, IDC_EDITOR_TREE));
		return TRUE;
		break;
	case WM_MOUSEMOVE:
		//return onMainMouseMove(h,wParam,lParam);
		return TRUE;
		break;
	case WM_LBUTTONDOWN:
		//return onMainMouseLDown(h,wParam,lParam);
		break;
	case WM_NOTIFY:
		if(LOWORD(wParam) == IDC_EDITOR_TREE){
			switch(((NMHDR*)lParam)->code){
			case NM_CUSTOMDRAW:
				return onTreeCustomDraw(h, (NMTVCUSTOMDRAW*)lParam);
			case TVN_SELCHANGING:
				//return onTreeSelChange(h, (NMTREEVIEW*)lParam);
				break;
			case TVN_ITEMEXPANDED:
				//return onTreeExpand(h, (NMTREEVIEW*)lParam);
				break;
			}
		}
		return TRUE;
		break;
	case WM_CLOSE:
		EndDialog(h,NULL);
		break;
	case WM_SIZE:
		//return onMainSize(h,wParam,lParam);
		return TRUE;
		break;
	}
	return FALSE;
}

void GUIWin::createEditorTree(HWND htree)
{
	long item_height;
	long entry_size;
	HTREEITEM root;

	item_height = SendMessage(htree, TVM_GETITEMHEIGHT, 0, 0);
	entry_size = 32/item_height+1;

	root = insterTreeItem(htree, "Ground", NULL, 1, ITEM_GROUP_GROUND);
	root = insterTreeItem(htree, "Container", NULL, 1, ITEM_GROUP_CONTAINER);
	root = insterTreeItem(htree, "Splash", NULL, 1, ITEM_GROUP_SPLASH);
	root = insterTreeItem(htree, "Other", NULL, 1, ITEM_GROUP_NONE);
}

HTREEITEM GUIWin::insterTreeItem(HWND h, char* name, HTREEITEM parent, long size, long entryID)
{
	TVINSERTSTRUCT itemstruct;
	itemstruct.hParent = parent;
	itemstruct.hInsertAfter = TVI_LAST;
	itemstruct.itemex.mask = TVIF_TEXT | TVIF_INTEGRAL | TVIF_PARAM;
	itemstruct.itemex.pszText = name;
	itemstruct.itemex.iIntegral = size;
	itemstruct.itemex.lParam = entryID;
	itemstruct.itemex.cchTextMax = strlen(name);
	return (HTREEITEM)SendMessage(h, TVM_INSERTITEM, 0, (long)&itemstruct); 
}

bool GUIWin::onTreeCustomDraw(HWND h, NMTVCUSTOMDRAW* pCD)
{
	if(pCD->nmcd.dwDrawStage == CDDS_PREPAINT){
		SetWindowLong(h, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW );
		return TRUE;
	}
	else if(pCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT){
		SetWindowLong(h, DWL_MSGRESULT, CDRF_NOTIFYPOSTPAINT );
		return TRUE;
	}
	else if(pCD->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT){
		/*
		if(pCD->nmcd.lItemlParam > ITEM_GROUP_GROUND_LAST ){
			//paint item
			long center_y = (pCD->nmcd.rc.bottom + pCD->nmcd.rc.top - 32)/2;
			long center_x = pCD->nmcd.rc.right - 32 - 16;
			//drawItem(pCD->nmcd.hdc, center_x, center_y , pCD->nmcd.rc.right, pCD->nmcd.rc.bottom, (ItemBase*)pCD->nmcd.lItemlParam);
		}
		*/
		return TRUE;
	}
	return FALSE;
}


