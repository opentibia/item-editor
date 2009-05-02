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

#ifdef __GNUC__
#define _WIN32_IE	0x0500
#define  WINVER  0x0500
#define _WIN32_WINNT 0x0501
#define max(a, b)       ((a) >? (b))
#define min(a, b)       ((a) <? (b))
#endif

#ifdef STRICT
#undef STRICT
#endif

#include "gui_win.hpp"
#include "../client/md5.h"

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

void getImageHash(unsigned short cid, void*output)
{
	MD5_CTX m_md5;
	unsigned long spriteBase;
	unsigned long spriteSize;
	const SpriteType *it = g_itemsSprites->getSprite(cid);
	if(!it)
		return;

	spriteSize = it->width*it->height*it->blendframes;
	spriteBase = 0;

	//hash sprite
	MD5Init(&m_md5, 0);
	for(long frame = 0; frame < it->blendframes; frame++) {
		for(long cy = 0; cy < it->height; cy++) {
			for(long cx = 0; cx < it->width; cx++) {
				unsigned long frameindex = spriteBase + cx + cy*it->width + frame*it->width*it->height;
				InternalSprite sprite = g_itemsSprites->getSpriteInternalFormat((unsigned short)cid, (unsigned short)frameindex);
				if(sprite)
					MD5Update(&m_md5, (const unsigned char*)sprite, 32*32*4);
			}
		}
	}

	MD5Final(&m_md5);

	memcpy(output, m_md5.digest, 16);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// class GUIWin
/////////////////////////////////////////////////////////////////////////////////////////////

GUIDraw* GUIWin::drawEngine = NULL;
long GUIWin::curItemClientIdSprite = 0;
long GUIWin::curItemClientCountSprite = 0;
long GUIWin::curItemClientId = 0;
HTREEITEM GUIWin::curItem = NULL;
long GUIWin::curItemServerId = 0;
bool GUIWin::m_dragging = false;
HWND GUIWin::m_hwndTree = 0;
HTREEITEM GUIWin::m_dragItem = NULL;
HTREEITEM GUIWin::rootItems[ITEM_GROUP_LAST] = {NULL};
unsigned long GUIWin::menuGroups[ITEM_GROUP_LAST] = {NULL};
HMENU GUIWin::popupMenu = 0;
bool GUIWin::autoFindPerformed = false;

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
	MessageBoxA(NULL, text, NULL, mtype);
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
		drawEngine->drawSprite(tmp, 34, 34, rect.right, rect.bottom, curItemClientIdSprite, curItemClientCountSprite);
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
		case ID_FILE_SAVEAS:
			return onSaveOtb(h);
			break;
		case ID_FILE_LOADOTB:
			return onLoadOtb(h);
			break;
		case ID_FILE_NEWIT:
			autoFindPerformed = false;
			g_itemsTypes->clear();
			loadTreeItemTypes(h);
			return TRUE;
			break;
		case ID_TOOLS_AUTOFIND:
			return onAutoFindImages(h);
			break;
		case ID_TOOLS_VERIFYITEMS:
			return onVerify(h);
			break;
		case ID_TOOLS_GOTOITEM:
			return onGotoItem(h);
			break;
		case ID_TOOLS_CREATEMISSING:
			return onCreateMissing(h);
			break;
		case ID_TOOLS_SHOWNOTFOUN:
			loadTreeItemTypes(h, true);
			break;
		case ID_TOOLS_SHOWALL:
			loadTreeItemTypes(h);
			break;
		case ID_TOOLS_EXPORTXML:
			g_itemsTypes->exportToXml("itemsExport.xml");
			break;
		case ID_TOOLS_GETXMLNAMES:
			g_itemsTypes->importFromXml("items.xml");
			loadTreeItemTypes(h);
			break;
		case ID_TOOLS_ADDITEM:
			return onAddItem(h);
			break;
		case ID_TOOLS_COPYITEM:
			return onCopyItem(h);
			break;
		case ID_TOOLS_RELOADATTR:
			return onReloadAttributes(h);
			break;
		case ID_HELP_ABOUT:
			MessageBoxA(h, OTIE_VERSION_STRING, "OTItemEditor", MB_OK | MB_ICONINFORMATION) ;
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
	long id = (long)nmTree->itemNew.lParam;
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
			TreeView_SelectDropTarget(m_hwndTree,  rootItems[g_itemsTypes->getGroup((long)itemInfo.lParam)]);
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
		id = (long)itemInfo.lParam;

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
		curItemServerId = (long)nmTree->itemNew.lParam;
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
	long len = (long)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
	tmp = new char[len+1];
	SendMessage(hEdit, WM_GETTEXT , len + 1, (LPARAM)tmp);
	tmp[len] = 0;
	if(sscanf(tmp, "%d", &new_id) == 1){
		if(curItemServerId >= 20000){
			curItemClientId = new_id;
			curItemClientIdSprite = 2874;
			if(curItemClientId > 255){
				curItemClientCountSprite = 0;
			}
			else{
				curItemClientCountSprite = (curItemServerId - 20000) % 8;
			}
		}
		else if(new_id <= SpriteType::maxClientId && new_id >= SpriteType::minClientId){
			curItemClientId = new_id;
			curItemClientIdSprite = new_id;
			curItemClientCountSprite = 0;
		}
		else{
			curItemClientId = 0;
			curItemClientIdSprite = 0;
			curItemClientCountSprite = 0;
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
				setContextMenuGroup(g_itemsTypes->getGroup((long)itemInfo.lParam));
				TrackPopupMenuEx(popupMenu, 0, screen_coords.x, screen_coords.y, h, NULL);
			}
		}
	}

	return TRUE;
}


LRESULT GUIWin::onSaveOtb(HWND h)
{
	if(!saveCurrentItem(h)){
		return TRUE;
	}

	OPENFILENAME opf;
	char fileItemsOtb[512];
	long ret;
	memset(&opf, 0, sizeof(opf));
	fileItemsOtb[0] = 0;
	opf.lStructSize = sizeof(OPENFILENAME);
	opf.hwndOwner = h;
	opf.nFilterIndex = 1;
	opf.nMaxFile = 511;
	opf.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOLONGNAMES;

	opf.lpstrFile = fileItemsOtb;
	opf.lpstrFilter = "OTB files\0*.otb\0\0";
	ret = GetSaveFileName(&opf);
	if(ret == 0)
		return TRUE;

	g_itemsTypes->saveOtb(fileItemsOtb);
	return TRUE;
}

LRESULT GUIWin::onLoadOtb(HWND h)
{

	OPENFILENAME opf;
	char fileItemsOtb[512];
	long ret;
	memset(&opf, 0, sizeof(opf));
	fileItemsOtb[0] = 0;
	opf.lStructSize = sizeof(OPENFILENAME);
	opf.hwndOwner = h;
	opf.nFilterIndex = 1;
	opf.nMaxFile = 511;
	opf.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOLONGNAMES;

	opf.lpstrFile = fileItemsOtb;
	opf.lpstrFilter = "OTB files\0*.otb\0\0";
	ret = GetOpenFileName(&opf);
	if(ret == 0)
		return TRUE;

	g_itemsTypes->clear();
	g_itemsTypes->loadOtb(fileItemsOtb);

	autoFindPerformed = false;
	loadTreeItemTypes(h);
	return TRUE;
}

LRESULT GUIWin::onAutoFindImages(HWND h)
{
	char hash[16];
	int n = 0;

	if(!saveCurrentItem(h)){
		return TRUE;
	}

	// Maybe the IDs haven't moved at all (this prevents IDs moving around when CiP adds new duplicate items)
	for(ItemMap::iterator it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		memcpy(hash, g_itemsSprites->getSprite(it->second->clientid)->sprHash, 16);
		if(memcmp(hash, it->second->sprHash, 16) == 0){
			if(it->second->compareOptions(g_itemsSprites->getSprite(it->second->clientid))){
				it->second->foundNewImage = true;
				n++;
				continue;
			}
		}
	}

	int i = 0;
	for(i = SpriteType::minClientId; i <= SpriteType::maxClientId ; i++){
		//#ifdef __SPRITE_SEARCH__
		memcpy(hash, g_itemsSprites->getSprite(i)->sprHash, 16);
		/*
		#else
		getImageHash(i, hash);
		#endif
		*/

		for(ItemMap::iterator it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
			if(it->second->foundNewImage == false && memcmp(hash, it->second->sprHash, 16) == 0){
				if(it->second->compareOptions(g_itemsSprites->getSprite(i))){
					it->second->foundNewImage = true;
					it->second->clientid = i;
					n++;
					break;
				}
			}
		}
	}

	char str[64];
	sprintf(str , "Found %d of %d.", n, SpriteType::maxClientId - SpriteType::minClientId + 1);
	MessageBoxA(h, str, NULL, MB_OK | MB_ICONINFORMATION);

	/*
	int spriteCount = std::distance(g_itemsTypes->getTypes(), g_itemsTypes->getEnd());
	int itemCount = std::distance(g_itemsTypes->getTypes(), g_itemsTypes->getEnd());
	int depricatedCount = 0;

	for(ItemMap::iterator it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		if(it->second->group == ITEM_GROUP_DEPRECATED){
			++depricatedCount;
		}
	}
	sprintf(str , "itemCount: %d  spriteCount: %d, depricatedCount: %d", itemCount, spriteCount, depricatedCount);
	MessageBoxA(h, str, NULL, MB_OK | MB_ICONINFORMATION);
	*/

	autoFindPerformed = true;
	loadTreeItemTypes(h);

	return TRUE;
}

LRESULT GUIWin::onReloadAttributes(HWND h)
{
	if(!saveCurrentItem(h)){
		return TRUE;
	}

	//look through all items and verify if their attributes are correct
	//if they aren't, make them correct.
	int n = 0;
	for(ItemMap::iterator it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		if(it->second->group == ITEM_GROUP_DEPRECATED)
			continue;

		if(!it->second->compareOptions(g_itemsSprites->getSprite(it->second->clientid))){
			it->second->reloadOptions(g_itemsSprites->getSprite(it->second->clientid));
			n++;
		}
	}

	char str[64];
	sprintf(str , "Reloaded %d of %d.", n, SpriteType::maxClientId - SpriteType::minClientId + 1);
	MessageBoxA(h, str, NULL, MB_OK | MB_ICONINFORMATION);

	return TRUE;
}

LRESULT GUIWin::onVerify(HWND h)
{
	//char hash[16];
	int n;
	if(!saveCurrentItem(h)){
		return TRUE;
	}
	n = 0;
	ItemMap::iterator it;
	for(it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		if(it->first < 20000){
			if(it->second->compareOptions(g_itemsSprites->getSprite(it->second->clientid))){
				n++;
			}
			else{
				int u = it->first;
			}
		}
	}

	char str[64];
	sprintf(str , "Found %d of %d.", n, SpriteType::maxClientId - SpriteType::minClientId + 1);
	MessageBoxA(h, str, NULL, MB_OK | MB_ICONINFORMATION);

	return TRUE;
}

LRESULT GUIWin::onCreateMissing(HWND h)
{
	if(!saveCurrentItem(h)){
		return TRUE;
	}

	if(ItemType::minServerId == 0 || ItemType::maxServerId == 0){
		MessageBoxA(h, "Wrong items set.", NULL, MB_OK | MB_ICONERROR);
		return TRUE;
	}

	char *usedSprites;
	usedSprites = new char[SpriteType::maxClientId + 1];
	memset(usedSprites, 0, SpriteType::maxClientId + 1);
	ItemMap::iterator it;
	for(it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		if(it->second->clientid >= SpriteType::minClientId && it->second->clientid <= SpriteType::maxClientId){
			usedSprites[it->second->clientid]++;
		}
	}

	ItemType *new_it;
	long counter = 0;
	int n = ItemType::maxServerId + 1;
	bool found;
	//FILE *f;
	//f = fopen("noused.txt","w");
	for(int i = SpriteType::minClientId; i <= SpriteType::maxClientId; i++){
		found = false;


		if(usedSprites[i] == 0){
			//fprintf(f, "%d\n", i);
			/*for(int j = SpriteType::minClientId; j <= SpriteType::maxClientId; j++){
				if(usedSprites[j] != 0 && memcmp(g_itemsSprites->getSprite(i)->sprHash,g_itemsSprites->getSprite(j)->sprHash,16) == 0){
					if(g_itemsSprites->getSprite(i)->compareOptions(g_itemsSprites->getSprite(j))){
						found = true;
						break;
					}
				}
			}*/
			//if(!found){
				new_it = new ItemType(n, g_itemsSprites->getSprite(i));
				g_itemsTypes->addType(n, new_it);
				n++;
				counter++;
				usedSprites[i]++;
			//}
		}
		else if(usedSprites[i] > 1){
			n = n;
		}
	}
	//fclose(f);
	char str[64];
	sprintf(str , "Created %d items.", counter);
	MessageBoxA(h, str, NULL, MB_OK | MB_ICONINFORMATION);

	delete usedSprites;
	loadTreeItemTypes(h);
	return TRUE;
}

LRESULT GUIWin::onAddItem(HWND h)
{
	if(!saveCurrentItem(h)){
		return TRUE;
	}

	if(ItemType::minServerId == 0 || ItemType::maxServerId == 0){
		MessageBoxA(h, "Wrong items set.", NULL, MB_OK | MB_ICONERROR);
		return TRUE;
	}

	int n;
	if(!getEditTextInt(h, IDC_EDITCID, n))
		return TRUE;

	ItemType *new_it;
	if(n < 20000) {
		n = ItemType::maxServerId + 1;
	}

	if(g_itemsTypes->getType(n) != NULL){
		MessageBoxA(h, "Used item id.", NULL, MB_OK | MB_ICONERROR);
		return TRUE;
	}


	new_it = new ItemType();
	new_it->id = n;
	new_it->clientid = 0;
	g_itemsTypes->addType(n, new_it);


	loadTreeItemTypes(h);
	return TRUE;
}

LRESULT GUIWin::onCopyItem(HWND h)
{
	if(!saveCurrentItem(h)){
		return TRUE;
	}

	if(ItemType::minServerId == 0 || ItemType::maxServerId == 0){
		MessageBoxA(h, "Wrong items set.", NULL, MB_OK | MB_ICONERROR);
		return TRUE;
	}

	int n;
	if(!getEditTextInt(h, IDC_SID, n))
		return TRUE;

	int newn = ItemType::maxServerId + 1;

	ItemType *new_it;
	new_it = new ItemType(*g_itemsTypes->getType(n));
	new_it->id = newn;
	g_itemsTypes->addType(newn, new_it);


	loadTreeItemTypes(h);
	return TRUE;
}


LRESULT GUIWin::onGotoItem(HWND h)
{
	return TRUE;
}

LRESULT GUIWin::onInitDialog(HWND h)
{
	//set window title
	SetWindowText(h, OTIE_VERSION_STRING);

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

	//initialize menu entryes
	menuGroups[ITEM_GROUP_GROUND] = ID_MENUG_GROUND;
	menuGroups[ITEM_GROUP_CONTAINER] = ID_MENUG_CONTAINER;
	//menuGroups[ITEM_GROUP_CHARGES] = ID_MENUG_CHARGES;
	//menuGroups[ITEM_GROUP_TELEPORT] = ID_MENUG_TELEPORT;
	//menuGroups[ITEM_GROUP_MAGICFIELD] = ID_MENUG_MAGICFIELD;
	menuGroups[ITEM_GROUP_SPLASH] = ID_MENUG_SPLASH;
	menuGroups[ITEM_GROUP_FLUID] = ID_MENUG_FLUID;
	menuGroups[ITEM_GROUP_DOOR] = ID_MENUG_DOOR;
	menuGroups[ITEM_GROUP_DEPRECATED] = ID_MENUG_DEPRECATED;
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
	//rootItems[ITEM_GROUP_CHARGES] = insertTreeItem(htree, "Charges", NULL, ITEM_GROUP_CHARGES);
	//rootItems[ITEM_GROUP_TELEPORT] = insertTreeItem(htree, "Teleport", NULL, ITEM_GROUP_TELEPORT);
	//rootItems[ITEM_GROUP_MAGICFIELD] = insertTreeItem(htree, "Magic Field", NULL, ITEM_GROUP_MAGICFIELD);
	//rootItems[ITEM_GROUP_DOOR] = insertTreeItem(htree, "Door", NULL, ITEM_GROUP_DOOR);
	rootItems[ITEM_GROUP_SPLASH] = insertTreeItem(htree, "Splash", NULL, ITEM_GROUP_SPLASH);
	rootItems[ITEM_GROUP_FLUID] = insertTreeItem(htree, "Fluid Container", NULL, ITEM_GROUP_FLUID);
	rootItems[ITEM_GROUP_NONE] = insertTreeItem(htree, "Other", NULL, ITEM_GROUP_NONE);
	rootItems[ITEM_GROUP_DEPRECATED] = insertTreeItem(htree, "Deprecated", NULL, ITEM_GROUP_DEPRECATED);
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
	char name[160];
	getItemTypeName(iType, name);

	HTREEITEM parent;

	switch(iType->group){
	case ITEM_GROUP_GROUND:
	case ITEM_GROUP_CONTAINER:
	//case ITEM_GROUP_CHARGES:
	//case ITEM_GROUP_TELEPORT:
	//case ITEM_GROUP_MAGICFIELD:
	//case ITEM_GROUP_DOOR:
	case ITEM_GROUP_SPLASH:
	case ITEM_GROUP_FLUID:
	case ITEM_GROUP_DEPRECATED:
	case ITEM_GROUP_NONE:
		parent = rootItems[iType->group];
		break;
	default:
		parent = rootItems[ITEM_GROUP_NONE];
		break;
	}

	return insertTreeItem(h, name, parent, iType->id);
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

	if(!getEditTextInt(h, IDC_EDITCID, iType->clientid)){
		return false;
	}
	//update spr hash
	if(iType->clientid >= SpriteType::minClientId && iType->clientid <= SpriteType::maxClientId){
		getImageHash(iType->clientid, iType->sprHash);
	}
	else{
		memset(iType->sprHash, 0, 16);
	}
	//update spr params
	SpriteType *sType;
	sType = g_itemsSprites->getSprite(iType->clientid);
	if(sType){
		iType->speed = sType->speed;
		iType->miniMapColor = sType->miniMapColor;
		iType->subParam07 = sType->subParam07;
		iType->subParam08 = sType->subParam08;
		iType->lightLevel = sType->lightLevel;
		iType->lightColor = sType->lightColor;
		iType->lightColor = sType->lightColor;
		iType->lightColor = sType->lightColor;
		iType->isHangable = sType->isHangable;
		iType->isHorizontal = sType->isHorizontal;
		iType->isVertical = sType->isVertical;
	}


	if(!getEditTextInt(h, IDC_EDIT_SPEED, iType->speed)){
		return false;
	}

	int a;
	if(!getEditTextInt(h, IDC_EDIT_TOPORDER, a)){
		return false;
	}
	iType->alwaysOnTopOrder = a;

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
	iType->hasHeight = getCheckButton(h, IDC_OPT_HASHEIGHT);
	iType->allowDistRead = getCheckButton(h, IDC_OPT_DISTREAD);
	iType->isHangable = getCheckButton(h, IDC_OPT_HANGABLE);
	iType->clientCharges = getCheckButton(h, IDC_OPT_CHARGES);

	//change name in tree
	TVITEM itemInfo;
	char name[160];
	itemInfo.mask = TVIF_HANDLE | TVIF_TEXT;
	getItemTypeName(iType, name);

	itemInfo.hItem = curItem;
	itemInfo.pszText = name;
	itemInfo.cchTextMax = strlen(name);
	SendMessage(m_hwndTree, TVM_SETITEM, 0, (long)&itemInfo);

	return true;
}

void GUIWin::loadItem(HWND h)
{
	//load ItemType[curItemServerId] options in gui
	ItemType *iType;
	if(curItemServerId && (iType = g_itemsTypes->getType(curItemServerId))){

		setEditTextInt(h, IDC_SID, iType->id);

		if(iType->group != ITEM_GROUP_DEPRECATED){
			setEditTextInt(h, IDC_EDITCID, iType->clientid);
		}
		else{
			setEditTextInt(h, IDC_EDITCID, 0);
		}
		//
		SpriteType *sType;
		sType = g_itemsSprites->getSprite(iType->clientid);
		if(sType){
			if(memcmp(iType->sprHash, sType->sprHash, 16) != 0){
				SetDlgItemText(h, IDC_DIFF_SPRITE, "[x]");
			}

			if(iType->blockSolid != sType->blockSolid){
				SetDlgItemText(h, IDC_DIFF_BLOCKING, "[x]");
			}
			if(iType->blockProjectile != sType->blockProjectile){
				SetDlgItemText(h, IDC_DIFF_BLOCKPROJECTILE, "[x]");
			}
			if(iType->blockPathFind != sType->blockPathFind){
				SetDlgItemText(h, IDC_DIFF_BLOCKPATHFIND, "[x]");
			}
			if(iType->moveable != sType->moveable){
				SetDlgItemText(h, IDC_DIFF_NO_MOVE, "[x]");
			}
			if(iType->alwaysOnTop != sType->alwaysOnTop){
				SetDlgItemText(h, IDC_DIFF_ATOP, "[x]");
			}
			if(iType->hasHeight != sType->hasHeight){
				SetDlgItemText(h, IDC_DIFF_HASHEIGHT, "[x]");
			}
			if(iType->pickupable != sType->pickupable){
				SetDlgItemText(h, IDC_DIFF_PICKUP, "[x]");
			}
			if(iType->isHangable != sType->isHangable){
				SetDlgItemText(h, IDC_DIFF_HANGABLE, "[x]");
			}
			if(iType->useable != sType->useable){
				SetDlgItemText(h, IDC_DIFF_USEABLE, "[x]");
			}
			if(iType->rotable != sType->rotable){
				SetDlgItemText(h, IDC_DIFF_ROTABLE, "[x]");
			}
			if(iType->readable != sType->readable){
				SetDlgItemText(h, IDC_DIFF_READABLE, "[x]");
			}
			/*
			if(iType->allowDistRead != sType->allowDistRead){
				SetDlgItemText(h, IDC_DIFF_DISTREAD, "[x]");
			}
			*/
			if(iType->isVertical != sType->isVertical){
				SetDlgItemText(h, IDC_DIFF_VERTICAL, "[x]");
			}
			if(iType->isHorizontal != sType->isHorizontal){
				SetDlgItemText(h, IDC_DIFF_HORIZONTAL, "[x]");
			}
			if(iType->clientCharges != sType->clientCharges){
				SetDlgItemText(h, IDC_DIFF_CHARGES, "[x]");
			}
			if(iType->stackable != sType->stackable){
				SetDlgItemText(h, IDC_DIFF_STACKABLE, "[x]");
			}
			if(iType->speed != sType->speed){
				SetDlgItemText(h, IDC_DIFF_SPEED, "[x]");
			}
			if(iType->alwaysOnTopOrder != sType->alwaysOnTopOrder){
				SetDlgItemText(h, IDC_DIFF_TOPORDER, "[x]");
			}
			if(iType->subParam07 != sType->subParam07){
				SetDlgItemText(h, IDC_DIFF_PARAM7, "[x]");
			}
			if(iType->subParam08 != sType->subParam08){
				SetDlgItemText(h, IDC_DIFF_PARAM8, "[x]");
			}
			if(iType->lightLevel != sType->lightLevel){
				SetDlgItemText(h, IDC_DIFF_LIGHT_LEVEL, "[x]");
			}
			if(iType->lightColor != sType->lightColor){
				SetDlgItemText(h, IDC_DIFF_LIGHT_COLOR, "[x]");
			}
			if(iType->miniMapColor != sType->miniMapColor){
				SetDlgItemText(h, IDC_DIFF_MINIMAP_COLOR, "[x]");
			}
		}
		//
		setEditTextInt(h, IDC_EDIT_SPEED, iType->speed);
		setEditTextInt(h, IDC_EDIT_TOPORDER, iType->alwaysOnTopOrder);
		setEditTextInt(h, IDC_EDIT_LIGHT_LEVEL, iType->lightLevel);
		setEditTextInt(h, IDC_EDIT_LIGHT_COLOR, iType->lightColor);
		setEditTextInt(h, IDC_EDIT_MINIMAP_COLOR, iType->miniMapColor);
		setEditTextInt(h, IDC_EDIT_PARAM7, iType->subParam07);
		setEditTextInt(h, IDC_EDIT_PARAM8, iType->subParam08);

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
		setCheckButton(h, IDC_OPT_HASHEIGHT, iType->hasHeight);
		setCheckButton(h, IDC_OPT_DISTREAD, iType->allowDistRead);
		setCheckButton(h, IDC_OPT_VERTICAL, iType->isVertical);
		setCheckButton(h, IDC_OPT_HORIZONTAL, iType->isHorizontal);
		setCheckButton(h, IDC_OPT_CHARGES, iType->clientCharges);
		setCheckButton(h, IDC_OPT_HANGABLE, iType->isHangable);
	}
	else{

		setEditTextInt(h, IDC_SID, 0);
		setEditTextInt(h, IDC_EDITCID, 0);
		setEditTextInt(h, IDC_EDIT_SPEED, 0);
		setEditTextInt(h, IDC_EDIT_TOPORDER, 0);
		setEditTextInt(h, IDC_EDIT_LIGHT_LEVEL, 0);
		setEditTextInt(h, IDC_EDIT_LIGHT_COLOR, 0);
		setEditTextInt(h, IDC_EDIT_MINIMAP_COLOR, 0);
		setEditTextInt(h, IDC_EDIT_PARAM7, 0);
		setEditTextInt(h, IDC_EDIT_PARAM8, 0);

		setCheckButton(h, IDC_OPT_BLOCKING, false);
		setCheckButton(h, IDC_OPT_BLOCKPROJECTILE, false);
		setCheckButton(h, IDC_OPT_BLOCKPATHFIND, false);
		setCheckButton(h, IDC_OPT_NO_MOVE, false);
		setCheckButton(h, IDC_OPT_ATOP, false);
		setCheckButton(h, IDC_OPT_HASHEIGHT, false);
		setCheckButton(h, IDC_OPT_HANGABLE, false);
		setCheckButton(h, IDC_OPT_PICKUP, false);
		setCheckButton(h, IDC_OPT_USEABLE, false);
		setCheckButton(h, IDC_OPT_ROTABLE, false);
		setCheckButton(h, IDC_OPT_READABLE, false);
		setCheckButton(h, IDC_OPT_DISTREAD, false);
		setCheckButton(h, IDC_OPT_VERTICAL, false);
		setCheckButton(h, IDC_OPT_HORIZONTAL, false);
		setCheckButton(h, IDC_OPT_CHARGES, false);
		setCheckButton(h, IDC_OPT_STACKABLE, false);

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
		setControlState(h, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	}
	else{
		setControlState(h, IDC_EDITCID_FLAG, 0, 0, 0);
		//setControlState(h, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	}
}

void GUIWin::setControlState(HWND h, unsigned long flagsEdit, unsigned long flagsOpt, unsigned long flagsCombo, unsigned long flagsButton)
{
	EnableWindow(GetDlgItem(h, IDC_EDIT_CID),getFlagState(flagsEdit, IDC_EDITCID_FLAG));

	EnableWindow(GetDlgItem(h, IDC_EDIT_SPEED),getFlagState(flagsEdit, IDC_EDIT_SPEED_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_LIGHT_LEVEL),getFlagState(flagsEdit, IDC_EDIT_LIGHT_LEVEL_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_LIGHT_COLOR),getFlagState(flagsEdit, IDC_EDIT_LIGHT_LEVEL_FLAG));
	EnableWindow(GetDlgItem(h, IDC_EDIT_TOPORDER),getFlagState(flagsEdit, IDC_EDIT_TOPORDER_FLAG ));
	EnableWindow(GetDlgItem(h, IDC_EDIT_MINIMAP_COLOR),getFlagState(flagsEdit, IDC_EDIT_MINIMAP_COLOR_FLAG ));

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
	EnableWindow(GetDlgItem(h, IDC_OPT_HASHEIGHT),getFlagState(flagsOpt, IDC_OPT_HASHEIGHT_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_DISTREAD),getFlagState(flagsOpt, IDC_OPT_DISTREAD_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_HANGABLE),getFlagState(flagsOpt, IDC_OPT_HANGABLE_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_VERTICAL),getFlagState(flagsOpt, IDC_OPT_VERTICAL_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_HORIZONTAL),getFlagState(flagsOpt, IDC_OPT_HORIZONTAL_FLAG));
	EnableWindow(GetDlgItem(h, IDC_OPT_CHARGES),getFlagState(flagsOpt, IDC_OPT_CHARGES_FLAG));


	EnableWindow(GetDlgItem(h, IDC_COMBO_FLOOR),getFlagState(flagsCombo, IDC_COMBO_FLOOR_FLAG));
}

void GUIWin::loadTreeItemTypes(HWND h, bool notFound)
{
	curItem = NULL;
	curItemServerId = 0;
	for(int i = 0;i < ITEM_GROUP_LAST;i++){
		TreeView_Expand(m_hwndTree, rootItems[i], TVE_COLLAPSE);
	}
	//delete all items in the tree
	TreeView_DeleteAllItems(m_hwndTree);
	//create groups
	createGroupsTree(m_hwndTree);
	//load itemType map
	ItemMap::iterator it;
	for(it = g_itemsTypes->getTypes(); it != g_itemsTypes->getEnd(); it++){
		if(notFound){
			if(it->second->foundNewImage == true)
				continue;
		}
		insertTreeItemType(m_hwndTree, it->second);
	}
	loadItem(h);
	updateControls(h);
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

void GUIWin::getItemTypeName(const ItemType *iType, char* name)
{
	char *tmp;
	if(autoFindPerformed && iType->foundNewImage){
		name[0] = '*';
		tmp = name + 1;
	}
	else{
		tmp = name;
	}
	if(iType->name[0] != 0){
		strcpy(tmp, iType->name.c_str());
	}
	else{
		sprintf(tmp, "Item number %d", iType->id);
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

bool GUIDraw::drawSprite(HDC desthdc, long x, long y, long maxx, long maxy, unsigned long itemid, long count, bool drawFrame /*= false*/)
{

	const SpriteType *it = g_itemsSprites->getSprite(itemid);
	if(!it)
		return false;

	unsigned long spriteBase;
	unsigned long spriteSize;

	spriteSize = it->width*it->height*it->blendframes;
	spriteBase = 0;

	//draw sprite
	for(long frame = 0; frame < it->blendframes; frame++) {
		for(long cy = 0; cy < it->height; cy++) {
			for(long cx = 0; cx < it->width; cx++) {
				unsigned long frameindex = spriteBase + cx + cy*it->width + frame*it->width*it->height;
				InternalSprite sprite;
				if(count == 0){
					sprite = g_itemsSprites->getSpriteInternalFormat((unsigned short)itemid, (unsigned short)frameindex);
				}
				else{
					sprite = g_itemsSprites->getSpriteInternalFormat((unsigned short)itemid, (unsigned short)count);
				}
				HBITMAP itembmp = getBitmap(sprite);
				if(sprite){
					if(x - cx*32 >= 0 && y - cy*32 >= 0){
						long x_draw = max(x - cx*32,0);
						long y_draw = max(y - cy*32,0);
						long w_draw = min(32 ,maxx - x + cx*32);
						long h_draw = min(32 ,maxy - y + cy*32);
						SelectObject(m_auxHDC,itembmp);
						TransparentBlt(desthdc,x_draw,y_draw,w_draw, h_draw, m_auxHDC,0,0,w_draw,h_draw,0x111111);
						//BitBlt(desthdc,x_draw,y_draw,w_draw, h_draw, m_auxHDC,0,0,SRCCOPY);
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
		m_bitmaps.erase(it++);
	}

}
