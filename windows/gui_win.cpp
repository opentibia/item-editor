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
			return onDragEnd(h);
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
			ItemType *iType;
			if(iType = g_itemsTypes->getItem(curItemServerId)){
				//TreeView_SelectDropTarget(m_hwndTree, rootItems[iType->getGroup()]);
			}
		}
	} 
	return TRUE;
}

bool GUIWin::onDragEnd(HWND h)
{
	TVITEM itemInfo;
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
		curItem = insterTreeItem(m_hwndTree, itemInfo.pszText, hitTarget, itemInfo.lParam);
		curItemServerId = itemInfo.lParam;
		TreeView_DeleteItem(m_hwndTree, m_dragItem);
		TreeView_SortChildren(m_hwndTree, hitTarget, 0);
		delete buffer;
		//update type
		itemInfo.mask = TVIF_PARAM;
		itemInfo.hItem = hitTarget;
		TreeView_GetItem(m_hwndTree, &itemInfo);
		g_itemsTypes->setGroup(id, (itemgroup_t)itemInfo.lParam);
		
		updateControls(h);
	}
	m_dragging = false;
	m_dragItem = NULL;
	ReleaseCapture();
	return TRUE;
}

bool GUIWin::onTreeSelChange(HWND h, NMTREEVIEW* nmTree)
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
	return TRUE;
}


bool GUIWin::onInitDialog(HWND h)
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

	updateControls(h);
	
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
	rootItems[ITEM_GROUP_FLUID] = insterTreeItem(htree, "FLuid Container", NULL, ITEM_GROUP_FLUID);
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

bool GUIWin::saveCurrentItem(HWND h)
{
	////CHANGE THIS///////////////////////////
	return true;
	//////////////////////////////////////////
	ItemType *iType;
	if(!curItemServerId)
		return true;

	if(curItemServerId && !(iType = g_itemsTypes->getItem(curItemServerId))){
		return false;
	}

	//validate and save values to itemType[curItemServerId] map
	char buffer[128];
	int len = GetDlgItemText(h, IDC_EDITNAME, buffer, 127);
	if(len >= 0)
		memcpy(iType->name, buffer, len);

	len = GetDlgItemText(h, IDC_EDITDESCR, buffer, 127);
	if(len >= 0)
		memcpy(iType->descr, buffer, len);

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
	if(!getEditTextInt(h, IDC_EDIT_READONLYID, iType->readonlyId)){
		return false;
	}
	if(!getEditTextInt(h, IDC_EDIT_ROTATETO, iType->rotateTo)){
		return false;
	}
	if(!getEditTextDouble(h, IDC_EDIT_WEIGHT, iType->weight)){
		return false;
	}

	iType->blocking = getCheckButton(h, IDC_OPT_BLOCKING);
	iType->alwaysOnTop = getCheckButton(h, IDC_OPT_ATOP);
	iType->stackable = getCheckButton(h, IDC_OPT_STACKABLE);
	iType->useable = getCheckButton(h, IDC_OPT_USEABLE);
	iType->notMoveable = getCheckButton(h, IDC_OPT_NO_MOVE);
	iType->pickupable = getCheckButton(h, IDC_OPT_PICKUP);
	iType->rotable= getCheckButton(h, IDC_OPT_ROTABLE);
	iType->blockingProjectile = getCheckButton(h, IDC_OPT_BLOCKPROJECTILE);
	iType->write1time = getCheckButton(h, IDC_OPT_WRITE1TIME);


	iType->slot_position = (enum slots_t)getComboValue(h, IDC_COMBO_SLOT);
	iType->weaponType = (enum WeaponType)getComboValue(h, IDC_COMBO_SKILL);
	iType->amuType = (enum amu_t)getComboValue(h, IDC_COMBO_AMU);
	iType->shootType = (enum subfight_t)getComboValue(h, IDC_COMBO_SHOOT);

	int comboFloor = getComboValue(h, IDC_COMBO_FLOOR);
	if(comboFloor & FLOOR_DOWN){
		iType->floorchange = true;
	}
	else{
		iType->floorchange = false;
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
	itemInfo.pszText = iType->name;
	itemInfo.cchTextMax = strlen(iType->name);
	SendMessage(m_hwndTree, TVM_SETITEM, 0, (long)&itemInfo);

	return true;
}

void GUIWin::loadItem(HWND h)
{
	//load ItemType[curItemServerId] options in gui
	ItemType *iType;
	if(curItemServerId && (iType = g_itemsTypes->getItem(curItemServerId))){
		
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
		setEditTextInt(h, IDC_EDIT_READONLYID, iType->readonlyId);
		setEditTextInt(h, IDC_EDIT_ROTATETO, iType->rotateTo);
		setEditTextDouble(h, IDC_EDIT_WEIGHT, iType->weight);

		setCheckButton(h, IDC_OPT_BLOCKING, iType->blocking);
		setCheckButton(h, IDC_OPT_ATOP, iType->alwaysOnTop);
		setCheckButton(h, IDC_OPT_STACKABLE, iType->stackable);
		setCheckButton(h, IDC_OPT_USEABLE, iType->useable);
		setCheckButton(h, IDC_OPT_NO_MOVE, iType->notMoveable);
		setCheckButton(h, IDC_OPT_PICKUP, iType->pickupable);
		setCheckButton(h, IDC_OPT_ROTABLE, iType->rotable);
		setCheckButton(h, IDC_OPT_BLOCKPROJECTILE, iType->blockingProjectile);
		setCheckButton(h, IDC_OPT_WRITE1TIME, iType->write1time);

		setComboValue(h, IDC_COMBO_SLOT, iType->slot_position);
		setComboValue(h, IDC_COMBO_SKILL, iType->weaponType);
		setComboValue(h, IDC_COMBO_AMU, iType->amuType);
		setComboValue(h, IDC_COMBO_SHOOT, iType->shootType);

		if(iType->floorchange)
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
		setCheckButton(h, IDC_OPT_WRITE1TIME, false);

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
	if(curItemServerId && (iType = g_itemsTypes->getItem(curItemServerId))){
		unsigned long editbase = IDC_EDITNAME_FLAG | IDC_EDITDSECR_FLAG | IDC_EDITCID_FLAG |
						IDC_EDIT_DECAYTO_FLAG | IDC_EDIT_DECAYTIME_FLAG;
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
	}
}

void GUIWin::setControlState(HWND h, unsigned long flagsEdit, unsigned long flagsOpt, unsigned long flagsCombo, unsigned long flagsButton)
{

	EnableWindow(GetDlgItem(h, IDC_EDITNAME),getFlagState(flagsEdit, IDC_EDITNAME_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDITDESCR),getFlagState(flagsEdit, IDC_EDITDSECR_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDITCID),getFlagState(flagsEdit, IDC_EDITCID_FLAG));

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
	EnableWindow(GetDlgItem(h, IDC_OPT_WRITE1TIME),getFlagState(flagsOpt, IDC_OPT_WRITE1TIME_FLAG));

	EnableWindow(GetDlgItem(h, IDC_COMBO_SLOT),getFlagState(flagsCombo, IDC_COMBO_SLOT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_SKILL),getFlagState(flagsCombo, IDC_COMBO_SKILL_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_AMU),getFlagState(flagsCombo, IDC_COMBO_AMU_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_SHOOT),getFlagState(flagsCombo, IDC_COMBO_SHOOT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_COMBO_FLOOR),getFlagState(flagsCombo, IDC_COMBO_FLOOR_FLAG));

	EnableWindow(GetDlgItem(h, IDC_SET_CLIENT_OPT),getFlagState(flagsButton, IDC_SET_CLIENT_OPT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_SAVE_ITEM),getFlagState(flagsButton, IDC_SAVE_ITEM_FLAG));
}

void GUIWin::loadTreeItemTypes(HWND h)
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
