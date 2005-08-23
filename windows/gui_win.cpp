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

inline bool getFlagState(unsigned long n, unsigned long flag)
{
	if((n & flag) != 0)
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// class GUIWin
/////////////////////////////////////////////////////////////////////////////////////////////

GUIDraw* GUIWin::drawEngine = NULL;
long GUIWin::curItemClientId = 0;
HTREEITEM GUIWin::curItem = NULL;
long GUIWin::curItemServerId = 0;
bool GUIWin::m_dragging = false;
HWND GUIWin::m_hwndTree = 0;
HTREEITEM GUIWin::m_dragItem = NULL;
HTREEITEM GUIWin::rootItems[ITEM_GROUP_LAST] = {NULL};
unsigned long GUIWin::menuGroups[ITEM_GROUP_LAST] = {NULL};
HMENU GUIWin::popupMenu = 0;

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

	//DialogBoxParam(g_instance,MAKEINTRESOURCE(DLG_MAIN),NULL,reinterpret_cast<DLGPROC>(GUIWin::DlgProcMain),NULL);
	
	MSG msg; 
	int ret;
	HACCEL haccel = LoadAccelerators(g_instance, MAKEINTRESOURCE(IDR_POPUP_ITEM)); 
	HWND hDlg = CreateDialog(g_instance, MAKEINTRESOURCE(DLG_MAIN), NULL, reinterpret_cast<DLGPROC>(GUIWin::DlgProcMain)); 
	ShowWindow(hDlg, SW_SHOW);
	while(ret = GetMessage (&msg, NULL, 0, 0)){
		if(ret == -1){
			return;
		}
		if((!TranslateAccelerator(hDlg, haccel, &msg)) && !IsDialogMessage(hDlg, &msg)){ 
			TranslateMessage (&msg); 
			DispatchMessage (&msg); 
		} 
	}
	DestroyWindow(hDlg);
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
			NMHDR *ahh;
			ahh = (NMHDR*)lParam;
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
			return onDragEnd(h);
		}
		break;
	case WM_CONTEXTMENU:
		if((HWND)wParam == m_hwndTree){
			return onContextMenu(h, (unsigned long)lParam);
		}
		break;
	case WM_VSCROLL:
		return onSpinScroll(h, (HWND)lParam);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
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
		int controlid = (wParam & 0xFFFF0000 >> 16);
		switch(controlid){
		case IDC_EDITCID:
			if((wParam & 0xFFFF0000) >> 16 == EN_CHANGE){
				return onClientIdChange(h, (HWND)lParam);
			}
			break;
		case ID_FILE_EXIT:
			PostQuitMessage(0);
			break;
		case ID_FILE_IMPORTOLD:
			return onImportOld(h);
			break;
		case ID_FILE_SAVEAS:
			g_itemsTypes->saveOtb("C:\\tmp.otb");
			break;
		case ID_TOOLS_VERIFYITEMS:
			g_itemsTypes->loadOtb("C:\\tmp.otb");
			loadTreeItemTypes(h);
			break;
		case ID_TOOLS_FINDMISSINGITEMS:
			break;
		case ID_HELP_ABOUT:
			break;
		default:
			//context menu
			for(int i = 0; i < ITEM_GROUP_LAST; i++){
				if(controlid == menuGroups[i]){
					return onContextMenuClick(h, i);
				}
			}
			break;
		}
		break;
	}
	return FALSE;
}

LRESULT GUIWin::onDragBegin(HWND h, const NMTREEVIEW* nmTree)
{
	long id = nmTree->itemNew.lParam;
	if(id > 100){
		m_dragItem = nmTree->itemNew.hItem;
		m_dragging = true;
		SetCapture(h);
	}
	return TRUE;
}

LRESULT GUIWin::onDragMove(HWND h, LPARAM lParam)
{
	HTREEITEM hitTarget;
	TVHITTESTINFO tvht;
	TVITEM itemInfo;
	tvht.pt.x = lParam & 0xFFFF; 
	tvht.pt.y = (lParam & 0xFFFF0000) >> 16;
	tvht.flags = TVHT_ONITEM;
	ClientToScreen(h, &tvht.pt);
	ScreenToClient(m_hwndTree, &tvht.pt);
	if((hitTarget = TreeView_HitTest(m_hwndTree, &tvht)) != NULL){
		itemInfo.mask = TVIF_HANDLE;
		itemInfo.hItem = hitTarget;
		TreeView_GetItem(m_hwndTree, &itemInfo);
		if(itemInfo.lParam < 100){
			TreeView_SelectDropTarget(m_hwndTree, hitTarget);
		}
		else{
			TreeView_SelectDropTarget(m_hwndTree,  rootItems[g_itemsTypes->getGroup(itemInfo.lParam)]);
		}
	} 
	return TRUE;
}

LRESULT GUIWin::onDragEnd(HWND h)
{
	TVITEM itemInfo;
	long id;
	HTREEITEM hitTarget = TreeView_GetDropHilight(m_hwndTree);
	if(hitTarget){
		TreeView_SelectDropTarget(m_hwndTree, NULL);
		
		itemInfo.mask = TVIF_PARAM;
		itemInfo.hItem = m_dragItem;
		TreeView_GetItem(m_hwndTree, &itemInfo);
		id = itemInfo.lParam;

		itemInfo.hItem = hitTarget;
		TreeView_GetItem(m_hwndTree, &itemInfo);

		if((itemgroup_t)itemInfo.lParam != g_itemsTypes->getGroup(id)){
			if(TreeView_SelectItem(m_hwndTree, hitTarget)){
				changeGroup(h, m_dragItem, hitTarget);
			}
		}
	}
	
	m_dragging = false;
	m_dragItem = NULL;
	ReleaseCapture();
	return TRUE;
}

LRESULT GUIWin::onContextMenuClick(HWND h, unsigned long newgroup)
{
	if(!curItemServerId)
		return TRUE;

	if(newgroup != g_itemsTypes->getGroup(curItemServerId)){
		HTREEITEM nextSel;
		nextSel = TreeView_GetNextSibling(m_hwndTree, curItem);
		if(!nextSel){
			nextSel = TreeView_GetPrevSibling(m_hwndTree, curItem);
			if(!nextSel){
				nextSel = TreeView_GetParent(m_hwndTree, curItem);
			}
		}
		changeGroup(h, curItem, rootItems[newgroup]);
		TreeView_SelectItem(m_hwndTree, nextSel);
	}
	return TRUE;
}

LRESULT GUIWin::onTreeSelChange(HWND h, const NMTREEVIEW* nmTree)
{
	if(!saveCurrentItem(h)){
		SetWindowLong(h, DWL_MSGRESULT, TRUE);
		return TRUE;
	}

	if(nmTree->itemNew.lParam >= 100){
		curItem = nmTree->itemNew.hItem;
		curItemServerId = nmTree->itemNew.lParam;
	}
	else{
		curItem = NULL;
		curItemServerId = 0;
	}

	loadItem(h);
	updateControls(h);

	return TRUE;
}

LRESULT GUIWin::onClientIdChange(HWND h, HWND hEdit)
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
	return TRUE;
}

LRESULT GUIWin::onContextMenu(HWND h, unsigned long lParam)
{
	HTREEITEM hitTarget;
	TVHITTESTINFO tvht;
	TVITEM itemInfo;
	POINT screen_coords;
	screen_coords.x = lParam & 0xFFFF;
	screen_coords.y = (lParam & 0xFFFF0000) >> 16;
	tvht.pt.x = lParam & 0xFFFF;
	tvht.pt.y = (lParam & 0xFFFF0000) >> 16;
	ScreenToClient(m_hwndTree, &tvht.pt);
	tvht.flags = TVHT_ONITEM;
	if((hitTarget = TreeView_HitTest(m_hwndTree, &tvht)) != NULL){
		if(TreeView_SelectItem(m_hwndTree, hitTarget)){
			itemInfo.mask = TVIF_HANDLE;
			itemInfo.hItem = hitTarget;
			TreeView_GetItem(m_hwndTree, &itemInfo);
			if(itemInfo.lParam >= 100){
				setContextMenuGroup(g_itemsTypes->getGroup(itemInfo.lParam));
				TrackPopupMenuEx(popupMenu, 0, screen_coords.x, screen_coords.y, h, NULL);
			}
		}
	}

	return TRUE;
}

LRESULT GUIWin::onImportOld(HWND h)
{
	OPENFILENAME opf;
	char fileItemsXml[512];
	char fileTibiaDat[512];
	long ret;
	memset(&opf, 0, sizeof(opf));
	fileItemsXml[0] = 0;
	fileTibiaDat[0] = 0;
	opf.lStructSize = sizeof(OPENFILENAME);
	opf.hwndOwner = h;
	opf.nFilterIndex = 1;
	opf.nMaxFile = 511;
	opf.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOLONGNAMES;

	opf.lpstrFile = fileTibiaDat;
	opf.lpstrFilter = "DAT files\0*.dat\0\0";
	ret = GetOpenFileName(&opf);
	if(ret == 0)
		return TRUE;

	opf.lpstrFile = fileItemsXml;
	opf.lpstrFilter = "XML files\0*.xml\0\0";
	ret = GetOpenFileName(&opf);
	if(ret == 0)
		return TRUE;
	
	curItem = NULL;
	curItemServerId = 0;

	g_itemsTypes->loadFromDat(fileTibiaDat);
	g_itemsTypes->loadFromXml(fileItemsXml);
	
	loadTreeItemTypes(h);

	return TRUE;
}

LRESULT GUIWin::onInitDialog(HWND h)
{

	m_hwndTree = GetDlgItem(h, IDC_EDITOR_TREE);
	createGroupsTree(m_hwndTree);

	SendMessage(GetDlgItem(h, IDC_SPINCID),UDM_SETRANGE,0,MAKELONG((short) SpriteType::maxClientId, (short)SpriteType::minClientId));

	//floor change
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "No change", FLOOR_NO_CHANGE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Down", FLOOR_DOWN);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up North", FLOOR_U_N);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up South", FLOOR_U_S);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up East", FLOOR_U_E);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up West", FLOOR_U_W);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up NE", FLOOR_U_NE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up NW", FLOOR_U_NW);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up SE", FLOOR_U_SE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_FLOOR), "Up SW", FLOOR_U_SW);

	//slot
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Default", SLOT_DEFAULT);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Head", SLOT_HEAD);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Body", SLOT_BODY);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Legs", SLOT_LEGS);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Backpack", SLOT_BACKPACK);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "weapon", SLOT_WEAPON);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Two hand", SLOT_2HAND);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Boots", SLOT_FEET);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Amulet", SLOT_AMULET);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Ring", SLOT_RING);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SLOT), "Hand", SLOT_HAND);

	
	//skills
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "None", WEAPON_NONE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Sword", WEAPON_SWORD);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Club", WEAPON_CLUB);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Axe", WEAPON_AXE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Shield", WEAPON_SHIELD);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Distance", WEAPON_DIST);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SKILL), "Magic", WEAPON_MAGIC);

	//amu
	createItemCombo(GetDlgItem(h, IDC_COMBO_AMU), "None", AMU_NONE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_AMU), "Bolt", AMU_BOLT);
	createItemCombo(GetDlgItem(h, IDC_COMBO_AMU), "Arrow", AMU_ARROW);

	//shoot
	SendMessage(GetDlgItem(h, IDC_COMBO_SHOOT), CB_SETDROPPEDWIDTH, 96, 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "None", DIST_NONE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Throwing Star", DIST_THROWINGSTAR);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Throwing Knife", DIST_THROWINGKNIFE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Small Stone", DIST_SMALLSTONE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Sudden Death", DIST_SUDDENDEATH);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Large Rock", DIST_LARGEROCK);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Snowball", DIST_SNOWBALL);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Spear", DIST_SPEAR);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Fire", DIST_FIRE);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Energy", DIST_ENERGY);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Bolt", DIST_BOLT);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Power Bolt", DIST_POWERBOLT);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Arrow", DIST_ARROW);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Poison Arrow", DIST_POISONARROW);
	createItemCombo(GetDlgItem(h, IDC_COMBO_SHOOT), "Burst Arrow", DIST_BURSTARROW);

	//editor types
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Grounds", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Building", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Hangable", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Interior", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Nature", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Accessories", 0);
	createItemCombo(GetDlgItem(h, IDC_COMBO_EDITOR), "Exterior", 0);


	//initialize menu entryes
	menuGroups[ITEM_GROUP_GROUND] = ID_MENUG_GROUND;
	menuGroups[ITEM_GROUP_CONTAINER] = ID_MENUG_CONTAINER;
	menuGroups[ITEM_GROUP_WEAPON] = ID_MENUG_WEAPON;
	menuGroups[ITEM_GROUP_AMMUNITION] = ID_MENUG_AMMUNITION;
	menuGroups[ITEM_GROUP_ARMOR] = ID_MENUG_ARMOR;
	menuGroups[ITEM_GROUP_RUNE] = ID_MENUG_RUNE;
	menuGroups[ITEM_GROUP_TELEPORT] = ID_MENUG_TELEPORT;
	menuGroups[ITEM_GROUP_MAGICFIELD] = ID_MENUG_MAGICFIELD;
	menuGroups[ITEM_GROUP_WRITEABLE] = ID_MENUG_WRITEABLE;
	menuGroups[ITEM_GROUP_KEY] = ID_MENUG_KEY;
	menuGroups[ITEM_GROUP_SPLASH] = ID_MENUG_SPLASH;
	menuGroups[ITEM_GROUP_FLUID] = ID_MENUG_FLUID;
	menuGroups[ITEM_GROUP_NONE] = ID_MENUG_NONE;
	popupMenu = GetSubMenu(LoadMenu(g_instance, MAKEINTRESOURCE(MENU_POPUP_ITEMS)), 0);

	updateControls(h);
	
	return TRUE;
}

void GUIWin::createItemCombo(HWND hcombo, const char* name, long value)
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

	rootItems[ITEM_GROUP_GROUND] = insertTreeItem(htree, "Ground", NULL, ITEM_GROUP_GROUND);
	rootItems[ITEM_GROUP_CONTAINER] = insertTreeItem(htree, "Container", NULL, ITEM_GROUP_CONTAINER);
	rootItems[ITEM_GROUP_WEAPON] = insertTreeItem(htree, "Weapon", NULL, ITEM_GROUP_WEAPON);
	rootItems[ITEM_GROUP_AMMUNITION] = insertTreeItem(htree, "Ammunition", NULL, ITEM_GROUP_AMMUNITION);
	rootItems[ITEM_GROUP_ARMOR] = insertTreeItem(htree, "Armor", NULL, ITEM_GROUP_ARMOR);
	rootItems[ITEM_GROUP_RUNE] = insertTreeItem(htree, "Rune", NULL, ITEM_GROUP_RUNE);
	rootItems[ITEM_GROUP_TELEPORT] = insertTreeItem(htree, "Teleport", NULL, ITEM_GROUP_TELEPORT);
	rootItems[ITEM_GROUP_MAGICFIELD] = insertTreeItem(htree, "Magic Field", NULL, ITEM_GROUP_MAGICFIELD);
	rootItems[ITEM_GROUP_WRITEABLE] = insertTreeItem(htree, "Writeable", NULL, ITEM_GROUP_WRITEABLE);
	rootItems[ITEM_GROUP_KEY] = insertTreeItem(htree, "Key", NULL, ITEM_GROUP_KEY);
	rootItems[ITEM_GROUP_SPLASH] = insertTreeItem(htree, "Splash", NULL, ITEM_GROUP_SPLASH);
	rootItems[ITEM_GROUP_FLUID] = insertTreeItem(htree, "Fluid Container", NULL, ITEM_GROUP_FLUID);
	rootItems[ITEM_GROUP_NONE] = insertTreeItem(htree, "Other", NULL, ITEM_GROUP_NONE);

	//insertTreeItem(htree, "Container 1", rootItems[ITEM_GROUP_NONE], 1988);
	//insertTreeItem(htree, "Container 2", rootItems[ITEM_GROUP_NONE],  1987);

}

HTREEITEM GUIWin::insertTreeItem(HWND h, const char* name, HTREEITEM parent, long entryID)
{
	TVINSERTSTRUCT itemstruct;
	itemstruct.hParent = parent;
	itemstruct.hInsertAfter = TVI_LAST;
	itemstruct.itemex.mask = TVIF_TEXT | TVIF_PARAM;
	itemstruct.itemex.pszText = (char*)name;
	itemstruct.itemex.lParam = entryID;
	itemstruct.itemex.cchTextMax = strlen(name);
	return (HTREEITEM)SendMessage(h, TVM_INSERTITEM, 0, (long)&itemstruct); 
}

HTREEITEM GUIWin::insertTreeItemType(HWND h, const ItemType *iType)
{
	char buffer[128];
	char *name;
	if(iType->name[0] != 0){
		name = (char*)&(iType->name[0]);
	}
	else{
		sprintf(buffer, "Item number %d", iType->id);
		name = buffer;
	}
	return insertTreeItem(h, name, rootItems[iType->group], iType->id);
}

LRESULT GUIWin::onSpinScroll(HWND h, HWND spin)
{
	long pos = SendMessage(spin,UDM_GETPOS, 0,0);
	return TRUE;
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

void GUIWin::changeGroup(HWND h, HTREEITEM htItem, HTREEITEM newParent)
{
	TVITEM itemInfo;
	char *buffer;
	long id;

	//change parent
	itemInfo.mask = TVIF_TEXT | TVIF_PARAM;
	itemInfo.hItem = htItem;
	buffer = new char[128];
	itemInfo.pszText = buffer;
	itemInfo.cchTextMax = 128;
	TreeView_GetItem(m_hwndTree, &itemInfo);
	id = itemInfo.lParam;
	insertTreeItem(m_hwndTree, itemInfo.pszText, newParent, itemInfo.lParam);
	TreeView_DeleteItem(m_hwndTree, htItem);
	
	delete buffer;
	//update type
	itemInfo.mask = TVIF_PARAM;
	itemInfo.hItem = newParent;
	TreeView_GetItem(m_hwndTree, &itemInfo);
	g_itemsTypes->setGroup(id, (itemgroup_t)itemInfo.lParam);

}

bool GUIWin::saveCurrentItem(HWND h)
{
	ItemType *iType;
	if(!curItemServerId)
		return true;

	if(curItemServerId >= 100 && !(iType = g_itemsTypes->getType(curItemServerId))){
		return false;
	}
	if(GetWindowTextLength(GetDlgItem(h, IDC_EDITNAME)) > 127 || GetWindowTextLength(GetDlgItem(h, IDC_EDITDESCR)) > 127){
		MessageBox(h, "Name or description too long.", NULL, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	//validate and save values to itemType[curItemServerId] map
	char buffer[128];
	int len = GetDlgItemText(h, IDC_EDITNAME, buffer, 127);
	if(len >= 0)
		memcpy(iType->name, buffer, len+1);

	len = GetDlgItemText(h, IDC_EDITDESCR, buffer, 127);
	if(len >= 0)
		memcpy(iType->descr, buffer, len+1);

	if(!getEditTextInt(h, IDC_EDITCID, iType->clientid)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_DECAYTO, iType->decayTo)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_DECAYTIME, iType->decayTime)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_ATK, iType->attack)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_DEF, iType->defence)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_ARM, iType->armor)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_MAXITEMS, iType->maxItems)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_SPEED, iType->speed)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_READONLYID, iType->readOnlyId)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_ROTATETO, iType->rotateTo)){
		return false;
	}
	if(!getEditTextDouble(h, IDC_EDIT_WEIGHT, iType->weight)){
		return false;
	}

	iType->blockSolid = getCheckButton(h, IDC_OPT_BLOCKING);
	iType->alwaysOnTop = getCheckButton(h, IDC_OPT_ATOP);
	iType->stackable = getCheckButton(h, IDC_OPT_STACKABLE);
	iType->useable = getCheckButton(h, IDC_OPT_USEABLE);
	iType->moveable = !getCheckButton(h, IDC_OPT_NO_MOVE);
	iType->pickupable = getCheckButton(h, IDC_OPT_PICKUP);
	iType->rotable = getCheckButton(h, IDC_OPT_ROTABLE);
	iType->blockProjectile = getCheckButton(h, IDC_OPT_BLOCKPROJECTILE);
	iType->readable = getCheckButton(h, IDC_OPT_READABLE);
	iType->blockPathFind = getCheckButton(h, IDC_OPT_BLOCKPATHFIND);
	iType->blockPickupable = getCheckButton(h, IDC_OPT_BLOCKPICKUP);

	iType->slot_position = (enum slots_t)getComboValue(h, IDC_COMBO_SLOT);
	iType->weaponType = (enum WeaponType)getComboValue(h, IDC_COMBO_SKILL);
	iType->amuType = (enum amu_t)getComboValue(h, IDC_COMBO_AMU);
	iType->shootType = (enum subfight_t)getComboValue(h, IDC_COMBO_SHOOT);

	int comboFloor = getComboValue(h, IDC_COMBO_FLOOR);
	if(comboFloor & FLOOR_DOWN){
		iType->floorChangeDown = true;
	}
	else{
		iType->floorChangeDown = false;
	}
	if(comboFloor & FLOOR_U_N){
		iType->floorChangeNorth = true;
	}
	else{
		iType->floorChangeNorth = false;
	}
	if(comboFloor & FLOOR_U_S){
		iType->floorChangeSouth = true;
	}
	else{
		iType->floorChangeSouth = false;
	}
	if(comboFloor & FLOOR_U_E){
		iType->floorChangeEast = true;
	}
	else{
		iType->floorChangeEast = false;
	}
	if(comboFloor & FLOOR_U_W){
		iType->floorChangeWest = true;
	}
	else{
		iType->floorChangeWest = false;
	}
	
	//change name in tree
	TVITEM itemInfo;
	itemInfo.mask = TVIF_HANDLE | TVIF_TEXT;
	itemInfo.hItem = curItem;
	if(iType->name[0] != 0){
		itemInfo.pszText = iType->name;
	}
	else{
		sprintf(buffer, "Item number %d", iType->id);
		itemInfo.pszText = buffer;
	}
	itemInfo.cchTextMax = strlen(iType->name);
	SendMessage(m_hwndTree, TVM_SETITEM, 0, (long)&itemInfo);

	return true;
}

void GUIWin::loadItem(HWND h)
{
	//load ItemType[curItemServerId] options in gui
	ItemType *iType;
	if(curItemServerId && (iType = g_itemsTypes->getType(curItemServerId))){
		
		SetDlgItemText(h, IDC_EDITNAME, iType->name);
		SetDlgItemText(h, IDC_EDITDESCR, iType->descr);

		setEditTextInt(h, IDC_SID, iType->id);
		setEditTextInt(h, IDC_EDITCID, iType->clientid);
		setEditTextInt(h, IDC_EDIT_DECAYTO, iType->decayTo);
		setEditTextInt(h, IDC_EDIT_DECAYTIME, iType->decayTime);
		setEditTextInt(h, IDC_EDIT_ATK, iType->attack);
		setEditTextInt(h, IDC_EDIT_DEF, iType->defence);
		setEditTextInt(h, IDC_EDIT_ARM, iType->armor);
		setEditTextInt(h, IDC_EDIT_MAXITEMS, iType->maxItems);
		setEditTextInt(h, IDC_EDIT_SPEED, iType->speed);
		setEditTextInt(h, IDC_EDIT_READONLYID, iType->readOnlyId);
		setEditTextInt(h, IDC_EDIT_ROTATETO, iType->rotateTo);
		setEditTextDouble(h, IDC_EDIT_WEIGHT, iType->weight);

		setCheckButton(h, IDC_OPT_BLOCKING, iType->blockSolid);
		setCheckButton(h, IDC_OPT_ATOP, iType->alwaysOnTop);
		setCheckButton(h, IDC_OPT_STACKABLE, iType->stackable);
		setCheckButton(h, IDC_OPT_USEABLE, iType->useable);
		setCheckButton(h, IDC_OPT_NO_MOVE, !iType->moveable);
		setCheckButton(h, IDC_OPT_PICKUP, iType->pickupable);
		setCheckButton(h, IDC_OPT_ROTABLE, iType->rotable);
		setCheckButton(h, IDC_OPT_BLOCKPROJECTILE, iType->blockProjectile);
		setCheckButton(h, IDC_OPT_READABLE, iType->readable);
		setCheckButton(h, IDC_OPT_BLOCKPATHFIND, iType->blockPathFind);
		setCheckButton(h, IDC_OPT_BLOCKPICKUP, iType->blockPickupable);

		setComboValue(h, IDC_COMBO_SLOT, iType->slot_position);
		setComboValue(h, IDC_COMBO_SKILL, iType->weaponType);
		setComboValue(h, IDC_COMBO_AMU, iType->amuType);
		setComboValue(h, IDC_COMBO_SHOOT, iType->shootType);

		if(iType->floorChangeDown)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_DOWN);
		else if(iType->floorChangeNorth && iType->floorChangeEast)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_NE);
		else if(iType->floorChangeSouth && iType->floorChangeEast)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_SE);
		else if(iType->floorChangeNorth && iType->floorChangeWest)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_NW);
		else if(iType->floorChangeSouth && iType->floorChangeWest)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_SW);
		else if(iType->floorChangeNorth)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_N);
		else if(iType->floorChangeSouth)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_S);
		else if(iType->floorChangeEast)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_E);
		else if(iType->floorChangeWest)
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_U_W);
		else
			setComboValue(h, IDC_COMBO_FLOOR, FLOOR_NO_CHANGE);

	}
	else{
		SetDlgItemText(h, IDC_EDITNAME, "");
		SetDlgItemText(h, IDC_EDITDESCR, "");

		setEditTextInt(h, IDC_SID, 0);
		setEditTextInt(h, IDC_EDITCID, 0);
		setEditTextInt(h, IDC_EDIT_DECAYTO, 0);
		setEditTextInt(h, IDC_EDIT_DECAYTIME, 0);
		setEditTextInt(h, IDC_EDIT_ATK, 0);
		setEditTextInt(h, IDC_EDIT_DEF, 0);
		setEditTextInt(h, IDC_EDIT_ARM, 0);
		setEditTextInt(h, IDC_EDIT_MAXITEMS, 0);
		setEditTextInt(h, IDC_EDIT_SPEED, 0);
		setEditTextInt(h, IDC_EDIT_READONLYID, 0);
		setEditTextInt(h, IDC_EDIT_ROTATETO, 0);
		setEditTextDouble(h, IDC_EDIT_WEIGHT, 0);

		setCheckButton(h, IDC_OPT_BLOCKING, false);
		setCheckButton(h, IDC_OPT_ATOP, false);
		setCheckButton(h, IDC_OPT_STACKABLE, false);
		setCheckButton(h, IDC_OPT_USEABLE, false);
		setCheckButton(h, IDC_OPT_NO_MOVE, false);
		setCheckButton(h, IDC_OPT_PICKUP, false);
		setCheckButton(h, IDC_OPT_ROTABLE, false);
		setCheckButton(h, IDC_OPT_BLOCKPROJECTILE, false);
		setCheckButton(h, IDC_OPT_READABLE, false);
		setCheckButton(h, IDC_OPT_BLOCKPATHFIND, false);
		setCheckButton(h, IDC_OPT_BLOCKPICKUP, false);

		setComboValue(h, IDC_COMBO_SLOT, SLOT_DEFAULT);
		setComboValue(h, IDC_COMBO_SKILL, WEAPON_NONE);
		setComboValue(h, IDC_COMBO_AMU, AMU_NONE);
		setComboValue(h, IDC_COMBO_SHOOT, DIST_NONE);

		setComboValue(h, IDC_COMBO_FLOOR, FLOOR_NO_CHANGE);
	}
}

void GUIWin::setComboValue(HWND h, int combo, int value)
{
	HWND hCombo = GetDlgItem(h, combo);
	int count = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
	for(count--;count >= 0; count--){
		int itemvalue = SendMessage(hCombo, CB_GETITEMDATA, count, 0);
		if(itemvalue == value){
			SendMessage(hCombo, CB_SETCURSEL, count, 0);
			return;
		}
	}
	SendMessage(hCombo, CB_SETCURSEL, -1, 0);
}

int GUIWin::getComboValue(HWND h, int combo)
{
	HWND hCombo = GetDlgItem(h, combo);
	int index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
	if(index != -1){
		return SendMessage(hCombo, CB_GETITEMDATA, index, 0);
	}
	return 0;
}

void GUIWin::setCheckButton(HWND h, int button, bool value)
{
	UINT check;
	if(value)
		check = BST_CHECKED;
	else
		check = BST_UNCHECKED;

	CheckDlgButton(h,button,check);

}

bool GUIWin::getCheckButton(HWND h, int button)
{
	UINT ret = IsDlgButtonChecked(h, button);
	if(ret == BST_UNCHECKED)
		return false;
	else
		return true;
}

void GUIWin::setEditTextInt(HWND h, int button, int value)
{
	char buffer[16];
	sprintf(buffer, "%d", value);
	SetDlgItemText(h, button , buffer);
}

void GUIWin::setEditTextDouble(HWND h, int button, double value)
{
	char buffer[16];
	sprintf(buffer, "%.2f", value);
	SetDlgItemText(h, button , buffer);
}

bool GUIWin::getEditTextInt(HWND h, int button, int &value)
{
	char buffer[16];
	int tmp;
	if(!GetDlgItemText(h, button, buffer, 15))
		return false;
	
	if(sscanf(buffer, "%d", &tmp) != 1)
		return false;

	value = tmp;
	return true;
}

bool GUIWin::getEditTextDouble(HWND h, int button, double &value)
{
	char buffer[16];
	double tmp;
	if(!GetDlgItemText(h, button, buffer, 15))
		return false;
	
	if(sscanf(buffer, "%lf", &tmp) != 1)
		return false;

	value = tmp;
	return true;
}

void GUIWin::updateControls(HWND h)
{
	//update controls depending on curItemServerId
	ItemType *iType;
	if(curItemServerId && (iType = g_itemsTypes->getType(curItemServerId))){
		unsigned long editbase = IDC_EDITNAME_FLAG | IDC_EDITDSECR_FLAG | IDC_EDITCID_FLAG |
						IDC_EDIT_DECAYTO_FLAG | IDC_EDIT_DECAYTIME_FLAG;
		setControlState(h, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		//TODO
		/*switch(iType->getGroup()){
		case ITEM_GROUP_GROUND:
			setControlState(h, editbase | ......, 0, 0, 0);
			break
		case ITEM_GROUP_CONTAINER:
			break
		case ITEM_GROUP_WEAPON:
			break
		case ITEM_GROUP_AMMUNITION:
			break
		case ITEM_GROUP_ARMOR:
			break
		case ITEM_GROUP_RUNE:
			break
		case ITEM_GROUP_TELEPORT:
			break
		case ITEM_GROUP_MAGICFIELD:
			break
		case ITEM_GROUP_WRITEABLE:
			break
		case ITEM_GROUP_KEY:
			break
		case ITEM_GROUP_SPLASH:
			break
		case ITEM_GROUP_FLUID:
			break
		case ITEM_GROUP_NONE:
			break
		default:
			break;
		}*/
	}
	else{
		setControlState(h, IDC_EDITCID_FLAG, 0, 0, 0);
		//setControlState(h, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	}
}

void GUIWin::setControlState(HWND h, unsigned long flagsEdit, unsigned long flagsOpt, unsigned long flagsCombo, unsigned long flagsButton)
{
	EnableWindow(GetDlgItem(h, IDC_EDIT_NAME),getFlagState(flagsEdit, IDC_EDITNAME_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_DESCR),getFlagState(flagsEdit, IDC_EDITDSECR_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_CID),getFlagState(flagsEdit, IDC_EDITCID_FLAG));

	EnableWindow(GetDlgItem(h, IDC_EDIT_DECAYTO),getFlagState(flagsEdit, IDC_EDIT_DECAYTO_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_DECAYTIME),getFlagState(flagsEdit, IDC_EDIT_DECAYTIME_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_ATK),getFlagState(flagsEdit, IDC_EDIT_ATK_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_DEF),getFlagState(flagsEdit, IDC_EDIT_DEF_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_ARM),getFlagState(flagsEdit, IDC_EDIT_ARM_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_MAXITEMS),getFlagState(flagsEdit, IDC_EDIT_MAXITEMS_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_SPEED),getFlagState(flagsEdit, IDC_EDIT_SPEED_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_READONLYID),getFlagState(flagsEdit, IDC_EDIT_READONLYID_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_ROTATETO),getFlagState(flagsEdit, IDC_EDIT_ROTATETO_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_WEIGHT),getFlagState(flagsEdit, IDC_EDIT_WEIGHT_FLAG));

	EnableWindow(GetDlgItem(h, IDC_OPT_BLOCKING),getFlagState(flagsOpt, IDC_OPT_BLOCKING_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_ATOP),getFlagState(flagsOpt, IDC_OPT_ATOP_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_STACKABLE),getFlagState(flagsOpt, IDC_OPT_STACKABLE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_USEABLE),getFlagState(flagsOpt, IDC_OPT_USEABLE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_NO_MOVE),getFlagState(flagsOpt, IDC_OPT_NO_MOVE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_PICKUP),getFlagState(flagsOpt, IDC_OPT_PICKUP_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_ROTABLE),getFlagState(flagsOpt, IDC_OPT_ROTABLE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_BLOCKPROJECTILE),getFlagState(flagsOpt, IDC_OPT_BLOCKPROJECTILE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_READABLE),getFlagState(flagsOpt, IDC_OPT_READABLE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_BLOCKPATHFIND),getFlagState(flagsOpt, IDC_OPT_BLOCKPATHFIND_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_BLOCKPICKUP),getFlagState(flagsOpt, IDC_OPT_BLOCKPICKUP_FLAG));

	EnableWindow(GetDlgItem(h, IDC_COMBO_SLOT),getFlagState(flagsCombo, IDC_COMBO_SLOT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_SKILL),getFlagState(flagsCombo, IDC_COMBO_SKILL_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_AMU),getFlagState(flagsCombo, IDC_COMBO_AMU_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_SHOOT),getFlagState(flagsCombo, IDC_COMBO_SHOOT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_FLOOR),getFlagState(flagsCombo, IDC_COMBO_FLOOR_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_EDITOR),getFlagState(flagsCombo, IDC_COMBO_EDITOR_FLAG));

	EnableWindow(GetDlgItem(h, IDC_SET_CLIENT_OPT),getFlagState(flagsButton, IDC_SET_CLIENT_OPT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_SAVE_ITEM),getFlagState(flagsButton, IDC_SAVE_ITEM_FLAG));
}

void GUIWin::loadTreeItemTypes(HWND h)
{
	//delete all items in the tree
	TreeView_DeleteAllItems(m_hwndTree);
	//create groups
	createGroupsTree(m_hwndTree);
	//load itemType map
	ItemMap::iterator it;
	for(it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		insertTreeItemType(m_hwndTree, it->second);
	}
}

void GUIWin::setContextMenuGroup(itemgroup_t group)
{
	for(int i = 0;i < ITEM_GROUP_LAST; i++){
		if(i != group){
			CheckMenuItem(popupMenu, menuGroups[i], MF_UNCHECKED);
		}
		else{
			CheckMenuItem(popupMenu, menuGroups[i], MF_CHECKED);
		}
	}
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
