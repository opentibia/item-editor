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
extern ItemsSprites *g_itemsSprites;
extern ItemsTypes *g_itemsTypes;

#ifdef __OLDINCLUDE__
extern "C" WINGDIAPI BOOL  WINAPI TransparentBlt(IN HDC,IN int,IN int,IN int,IN int,IN HDC,IN int,IN int,IN int,IN int,IN UINT);
#endif

GUIDraw* GUIWin::drawEngine = NULL;
long GUIWin::curItemClientId = 0;
HTREEITEM GUIWin::curItem = NULL;
long GUIWin::curItemServerId = 0;
bool GUIWin::m_dragging = false;
HWND GUIWin::m_hwndTree = 0;
HTREEITEM GUIWin::m_dragItem = NULL;
HTREEITEM GUIWin::rootItems[ITEM_GROUP_LAST] = {NULL};

GUIWin::GUIWin()
{
	drawEngine = new GUIDraw();
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

void GUIWin::messageBox(const char *text, MesageBoxType_t type)
{
	UINT mtype;
	switch(type){
	case MESSAGE_TYPE_FATAL_ERROR:
		mtype = MB_OK | MB_ICONERROR;
		break;
	case MESSAGE_TYPE_ERROR:
		mtype = MB_OK | MB_ICONEXCLAMATION;
		break;
	case MESSAGE_TYPE_INFO:
		mtype = MB_OK | MB_ICONINFORMATION;
		break;
	case MESSAGE_TYPE_NO_ICON:
		mtype = MB_OK;
	default:
		mtype = MB_OK;
		break;
	}
	MessageBox(NULL, text, NULL, mtype);
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
		return onInitDialog(h);
		break;
	case WM_NOTIFY:
		if(LOWORD(wParam) == IDC_EDITOR_TREE){
			switch(((NMHDR*)lParam)->code){
			case TVN_SELCHANGING:
				return onTreeSelChange(h, (NMTREEVIEW*)lParam);
				break;
			case TVN_BEGINDRAG:
				return onDragBegin(h, (NMTREEVIEW*)lParam);
				break;
			}
		}
		break;
	case WM_MOUSEMOVE:
		if(m_dragging){
			return onDragMove(h, lParam);
		}
		break;
	case WM_LBUTTONUP:
		if(m_dragging){
			return onDragEnd();
		}
		break;
	case WM_VSCROLL:
		return onSpinScroll(h, (HWND)lParam);
		break;
	case WM_CLOSE:
		EndDialog(h,NULL);
		break;
	case WM_PAINT:
		HDC tmp;
		tmp = GetDC(GetDlgItem(h,IDC_ITEM_PIC));
		
		HWND hwnd;
		hwnd = GetDlgItem(h, IDC_ITEM_PIC);
		RECT rect;
		GetClientRect(hwnd, &rect);
		
		Rectangle(tmp, 0, 0, rect.right, rect.bottom);
		drawEngine->drawSprite(tmp, 34, 34, rect.right, rect.bottom, curItemClientId);
		drawEngine->releaseBitmaps();

		ReleaseDC(GetDlgItem(h,IDC_ITEM_PIC),tmp);
		return FALSE;
		break;
	case WM_COMMAND:
		switch((wParam & 0xFFFF0000 >> 16)){
		case IDC_EDITCID:
			if((wParam & 0xFFFF0000) >> 16 == EN_CHANGE){
				return onClientIdChange(h, (HWND)lParam);
			}
			break;
		case ID_FILE_EXIT:
			EndDialog(h,NULL);
			break;
		case ID_HELP_ABOUT:
			break;
		}
		break;
	}
	return FALSE;
}

bool GUIWin::onDragBegin(HWND h, NMTREEVIEW* nmTree)
{
	long id = nmTree->itemNew.lParam;
	if(id > 100){
		m_dragItem = nmTree->itemNew.hItem;
		m_dragging = true;
		SetCapture(h);
	}
	return TRUE;
}

bool GUIWin::onDragMove(HWND h, LPARAM lParam)
{
	HTREEITEM hitTarget;
	TVHITTESTINFO tvht;
	TV_ITEM itemInfo;
	tvht.pt.x = lParam & 0xFFFF; 
	tvht.pt.y = (lParam & 0xFFFF0000) >> 16;
	tvht.flags = TVHT_ONITEM;
	ClientToScreen(h, &tvht.pt);
	ScreenToClient(m_hwndTree, &tvht.pt);
	if((hitTarget = TreeView_HitTest(m_hwndTree, &tvht)) != NULL){
		itemInfo.mask = TVIF_HANDLE;
		itemInfo.hItem = hitTarget;
		TreeView_GetItem(m_hwndTree, &itemInfo);
		if(itemInfo.lParam < 100)
			TreeView_SelectDropTarget(m_hwndTree, hitTarget);
	} 
	return TRUE;
}

bool GUIWin::onDragEnd()
{
	TV_ITEM itemInfo;
	char *buffer;
	long id;
	HTREEITEM hitTarget = TreeView_GetDropHilight(m_hwndTree);
	if(hitTarget){
		TreeView_SelectDropTarget(m_hwndTree, NULL);
		//change parent
		itemInfo.mask = TVIF_TEXT | TVIF_PARAM;
		itemInfo.hItem = m_dragItem;
		buffer = new char[128];
		itemInfo.pszText = buffer;
		itemInfo.cchTextMax = 128;
		TreeView_GetItem(m_hwndTree, &itemInfo);
		id = itemInfo.lParam;
		insterTreeItem(m_hwndTree, itemInfo.pszText, hitTarget, itemInfo.lParam);
		TreeView_DeleteItem(m_hwndTree, m_dragItem);
		TreeView_SortChildren(m_hwndTree, hitTarget, 0);
		delete buffer;
		//update type
		itemInfo.mask = TVIF_PARAM;
		itemInfo.hItem = hitTarget;
		TreeView_GetItem(m_hwndTree, &itemInfo);
		g_itemsTypes->setGroup(id, (itemgroup_t)itemInfo.lParam);
		if(id == curItemServerId){
			saveCurrentItem();
			loadItem();
			updateControls();
		}
	}
	m_dragging = false;
	m_dragItem = NULL;
	ReleaseCapture();
	return TRUE;
}

bool GUIWin::onTreeSelChange(HWND h, NMTREEVIEW* nmTree)
{
	saveCurrentItem();

	if(nmTree->itemNew.lParam >= 100){
		curItem = nmTree->itemNew.hItem;
		curItemServerId = nmTree->itemNew.lParam;
	}
	else{
		curItem = NULL;
		curItemServerId = 0;
	}

	loadItem();
	updateControls();

	return true;
}

bool GUIWin::onClientIdChange(HWND h, HWND hEdit)
{
	char *tmp;
	long new_id;
	long len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
	tmp = new char[len+1];
	SendMessage(hEdit, WM_GETTEXT , len, (LPARAM)tmp);
	if(sscanf(tmp, "%d", &new_id) == 1){
		if(new_id <= SpriteType::maxClientId && new_id >= SpriteType::minClientId){
			curItemClientId = new_id;
		}
		else{
			curItemClientId = 0;
		}
		invalidateSprite(h);
	}
	delete tmp;
	return true;
}


bool GUIWin::onInitDialog(HWND h)
{

	m_hwndTree = GetDlgItem(h, IDC_EDITOR_TREE);
	createGroupsTree(m_hwndTree);

	SendMessage(GetDlgItem(h, IDC_SPINCID),UDM_SETRANGE,0,MAKELONG((short) SpriteType::maxClientId, (short)SpriteType::minClientId));

	//floor change
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "No change", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Down", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up North", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up South", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up East", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up West", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up NE", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up NW", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up SW", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up SE", 0);

	//slot
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Default", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Head", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Body", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Legs", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Backpack", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "weapon", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Wwo hand", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Boots", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Amulet", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Ring", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Hand", 0);

	//skills
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Sword", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Club", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Axe", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Shield", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Distance", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Magic", 0);

	//amu
	createItemCombo(GetDlgItem(h, IDC_COMBO_AMU), "None", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_AMU), "Bolt", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_AMU), "Arrow", 0);

	//shoot
	SendMessage(GetDlgItem(h, IDC_COMBO_SHOOT), CB_SETDROPPEDWIDTH, 96, 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "None", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Throwing Star", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Throwing Knife", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Small Stone", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Sudden Death", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Large Rock", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Snowball", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Spear", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Fire", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Energy", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Bolt", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Power Bolt", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Arrow", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Poison Arrow", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Burst Arrow", 0);

	updateControls();
	
	return TRUE;
}

void GUIWin::createItemCombo(HWND hcombo, char* name, long value)
{
	long index = SendMessage(hcombo,CB_ADDSTRING, 0, (long)name);
	SendMessage(hcombo, CB_SETITEMDATA, index, value);
}

void GUIWin::createGroupsTree(HWND htree)
{
	long item_height;
	long entry_size;

	item_height = SendMessage(htree, TVM_GETITEMHEIGHT, 0, 0);
	entry_size = 32/item_height+1;

	rootItems[ITEM_GROUP_GROUND] = insterTreeItem(htree, "Ground", NULL, ITEM_GROUP_GROUND);
	rootItems[ITEM_GROUP_CONTAINER] = insterTreeItem(htree, "Container", NULL, ITEM_GROUP_CONTAINER);
	rootItems[ITEM_GROUP_WEAPON] = insterTreeItem(htree, "Weapon", NULL, ITEM_GROUP_WEAPON);
	rootItems[ITEM_GROUP_AMMUNITION] = insterTreeItem(htree, "Ammunition", NULL, ITEM_GROUP_AMMUNITION);
	rootItems[ITEM_GROUP_ARMOR] = insterTreeItem(htree, "Armor", NULL, ITEM_GROUP_ARMOR);
	rootItems[ITEM_GROUP_RUNE] = insterTreeItem(htree, "Rune", NULL, ITEM_GROUP_RUNE);
	rootItems[ITEM_GROUP_TELEPORT] = insterTreeItem(htree, "Teleport", NULL, ITEM_GROUP_TELEPORT);
	rootItems[ITEM_GROUP_MAGICFIELD] = insterTreeItem(htree, "Magic Field", NULL, ITEM_GROUP_MAGICFIELD);
	rootItems[ITEM_GROUP_WRITEABLE] = insterTreeItem(htree, "Writeable", NULL, ITEM_GROUP_WRITEABLE);
	rootItems[ITEM_GROUP_KEY] = insterTreeItem(htree, "Key", NULL, ITEM_GROUP_KEY);
	rootItems[ITEM_GROUP_SPLASH] = insterTreeItem(htree, "Splash", NULL, ITEM_GROUP_SPLASH);
	rootItems[ITEM_GROUP_NONE] = insterTreeItem(htree, "Other", NULL, ITEM_GROUP_NONE);

	insterTreeItem(htree, "Container 1", rootItems[ITEM_GROUP_NONE], 1988);
	insterTreeItem(htree, "Container 2", rootItems[ITEM_GROUP_NONE],  1987);

}

HTREEITEM GUIWin::insterTreeItem(HWND h, char* name, HTREEITEM parent, long entryID)
{
	TVINSERTSTRUCT itemstruct;
	itemstruct.hParent = parent;
	itemstruct.hInsertAfter = TVI_LAST;
	itemstruct.itemex.mask = TVIF_TEXT | TVIF_PARAM;
	itemstruct.itemex.pszText = name;
	itemstruct.itemex.lParam = entryID;
	itemstruct.itemex.cchTextMax = strlen(name);
	return (HTREEITEM)SendMessage(h, TVM_INSERTITEM, 0, (long)&itemstruct); 
}

bool GUIWin::onSpinScroll(HWND h, HWND spin)
{
	long pos = SendMessage(spin,UDM_GETPOS, 0,0);
	return true;
}

void GUIWin::invalidateSprite(HWND h)
{
	HWND hwnd = GetDlgItem(h, IDC_ITEM_PIC);
	RECT rect;
	GetWindowRect(hwnd, &rect);
	POINT pt;
	pt.x = rect.right;
	pt.y = rect.bottom;
	ScreenToClient(h, &pt);
	rect.right = pt.x;
	rect.bottom = pt.y;
	pt.x = rect.left;
	pt.y = rect.top;
	ScreenToClient(h, &pt);
	rect.left = pt.x;
	rect.top = pt.y;

	InvalidateRect(h, &rect, false);
}

void GUIWin::saveCurrentItem()
{
	if(!curItemServerId)
		return;

	//validate and save values to itemType[curItemServerId] map
	//change name in tree
}

void GUIWin::loadItem()
{
	//load ItemType[curItemServerId] options in gui
	if(curItemServerId){

	}
	else{

	}
}

void GUIWin::updateControls()
{
	//update controls depending on curItemServerId
	//EnableWindow(GetDlgItem(h, IDC_SPINCID),false);
	//EnableWindow(GetDlgItem(h, IDC_EDITCID),false);
}

void GUIWin::loadTreeItemTypes()
{
	//delete all items in the tree
	//load itemType map
}

/////////////////////////////////////////////////////////////////////////////////////////////
// class GUIDraw
/////////////////////////////////////////////////////////////////////////////////////////////
BitmapMap GUIDraw::m_bitmaps;
HDC GUIDraw::m_auxHDC = 0;
HBITMAP GUIDraw::m_oldauxBMP = 0;
HBITMAP GUIDraw::m_auxBMP = 0;

GUIDraw::GUIDraw()
{
	HDC tmp = GetDC(NULL);
	m_auxHDC = CreateCompatibleDC(tmp);
	ReleaseDC(NULL,tmp);


	m_auxBMP = CreateCompatibleBitmap(m_auxHDC,1,1);
	m_oldauxBMP = (HBITMAP)SelectObject(m_auxHDC, m_auxBMP);
}


GUIDraw::~GUIDraw(){
	releaseBitmaps();
	SelectObject(m_auxHDC, m_oldauxBMP);
	DeleteObject(m_auxBMP);
	DeleteDC(m_auxHDC);
}

bool GUIDraw::drawSprite(HDC desthdc, long x, long y, long maxx, long maxy, unsigned long itemid, bool drawFrame /*= false*/)
{

	const SpriteType& it = (*g_itemsSprites)[itemid];

	unsigned long spriteBase;
	unsigned long spriteSize;

	spriteSize = it.width*it.height*it.blendframes;
	spriteBase = 0;
	
	//draw sprite
	for(long frame = 0; frame < it.blendframes; frame++) {
		for(long cy = 0; cy < it.height; cy++) {
			for(long cx = 0; cx < it.width; cx++) {
				unsigned long frameindex = spriteBase + cx + cy*it.width + frame*it.width*it.height;
				InternalSprite sprite = g_itemsSprites->getSpriteInternalFormat(itemid,frameindex);
				HBITMAP itembmp = getBitmap(sprite);
				if(sprite){
					SelectObject(m_auxHDC,itembmp);
					if(x - cx*32 >= 0 && y - cy*32 >= 0){
						long x_draw = max(x - cx*32,0);
						long y_draw = max(y - cy*32,0);
						long w_draw = min(32 ,maxx - x + cx*32);
						long h_draw = min(32 ,maxy - y + cy*32);
						TransparentBlt(desthdc,x_draw,y_draw,w_draw, h_draw, m_auxHDC,0,0,w_draw,h_draw,0x111111);
					}
				}
			}
		}
	}
	if(drawFrame){
		RECT rect;
		rect.top = y;
		rect.bottom = y + 32;
		rect.left =  x;
		rect.right = x + 32;
		HBRUSH white_brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		FrameRect(desthdc, &rect, white_brush);
	}
	SelectObject(m_auxHDC, m_auxBMP);
	return true;
}


HBITMAP GUIDraw::getBitmap(const InternalSprite sprite)
{
	BITMAPINFO binfo;
	HDC hdc;
	void * dest;

	BitmapMap::iterator it = m_bitmaps.find((unsigned long)sprite);
	
	if(it != m_bitmaps.end())
	{
		return it->second;
	}
	else 
	{
		hdc = CreateCompatibleDC(NULL);
		memset(&binfo,0,sizeof(BITMAPINFO));
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biWidth = 32;
		binfo.bmiHeader.biHeight = 32;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biBitCount = 32;
		binfo.bmiHeader.biSizeImage = 32*32*4;
		binfo.bmiHeader.biCompression = BI_RGB;
	
		HBITMAP bitmap = CreateDIBSection(hdc, &binfo, DIB_RGB_COLORS, &dest, NULL,0);
		ASSERT(dest != NULL && bitmap != 0);
		if(dest){
			if(sprite){
				memcpy(dest, sprite, 32*32*4);
			}
			else{
				memset(dest, 0, 32*32*4);
			}
		}

		DeleteDC(hdc);
		m_bitmaps[(unsigned long)sprite] = bitmap;
		return bitmap;
	}
}

void GUIDraw::releaseBitmaps()
{
	BitmapMap::iterator it;
	for(it = m_bitmaps.begin(); it != m_bitmaps.end();)
	{
		DeleteObject(it->second);
		it = m_bitmaps.erase(it);
	}

}
