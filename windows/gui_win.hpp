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

#ifndef __OTITEMEDITOR_GUI_WIN_H__
#define __OTITEMEDITOR_GUI_WIN_H__

#include "windows.h"

#ifdef __OLDINCLUDE__
typedef DWORD* DWORD_PTR;
typedef UINT* UINT_PTR;
typedef ULONG* ULONG_PTR;
typedef INT* INT_PTR;
#endif

#include "commctrl.h"
#include "../client/gui.hpp"
#include "../client/item_type.hpp"
#include "../client/item_sprite.hpp"

class GUIDraw;

class GUIWin: public GUI {
public:
	GUIWin();
	virtual ~GUIWin(){};
	virtual void initGUI();
	virtual void messageBox(const char* text, MesageBoxType_t type);
	virtual bool loadSpriteInternal(const unsigned char *, const unsigned long, InternalSprite * );
	virtual void loadSpriteInternalTransparent(unsigned long color,  InternalSprite *sprite);
	virtual void unloadSpriteInternal(InternalSprite);

protected:

	enum floorchange_t{
		FLOOR_NO_CHANGE = 1,
		FLOOR_DOWN = 2,
		FLOOR_U_N = 4,
		FLOOR_U_S = 8,
		FLOOR_U_E = 16,
		FLOOR_U_W = 32,
		FLOOR_U_NE = FLOOR_U_N | FLOOR_U_E,
		FLOOR_U_NW = FLOOR_U_N | FLOOR_U_W,
		FLOOR_U_SE = FLOOR_U_S | FLOOR_U_E,
		FLOOR_U_SW = FLOOR_U_S | FLOOR_U_W,
	};

	enum controlEdit_t{
		IDC_EDITNAME_FLAG = 1,
		IDC_EDITDSECR_FLAG = 2,
		IDC_EDITCID_FLAG = 4,
		IDC_EDIT_DECAYTO_FLAG = 8,
		IDC_EDIT_DECAYTIME_FLAG = 16,
		IDC_EDIT_ATK_FLAG = 32,
		IDC_EDIT_DEF_FLAG = 64,
		IDC_EDIT_ARM_FLAG = 128,
		IDC_EDIT_MAXITEMS_FLAG = 256,
		IDC_EDIT_SPEED_FLAG = 512,
		IDC_EDIT_READONLYID_FLAG = 1024,
		IDC_EDIT_ROTATETO_FLAG = 2048,
		IDC_EDIT_WEIGHT_FLAG = 4096,
	};

	enum controlOpt_t{
		IDC_OPT_BLOCKING_FLAG = 1,
		IDC_OPT_ATOP_FLAG = 2,
		IDC_OPT_STACKABLE_FLAG = 4,
		IDC_OPT_USEABLE_FLAG = 8,
		IDC_OPT_NO_MOVE_FLAG = 16,
		IDC_OPT_PICKUP_FLAG = 32,
		IDC_OPT_ROTABLE_FLAG = 64,
		IDC_OPT_BLOCKPROJECTILE_FLAG = 128,
		IDC_OPT_READABLE_FLAG = 256,
		IDC_OPT_BLOCKPATHFIND_FLAG = 512,
		IDC_OPT_HASHEIGHT_FLAG = 1024,
	};

	enum controlCombo_t{
		IDC_COMBO_SLOT_FLAG = 1,
		IDC_COMBO_SKILL_FLAG = 2,
		IDC_COMBO_AMU_FLAG = 4,
		IDC_COMBO_SHOOT_FLAG = 8,
		IDC_COMBO_FLOOR_FLAG = 16,
		IDC_COMBO_EDITOR_FLAG = 32,
	};

	enum controlButton_t{
		IDC_SET_CLIENT_OPT_FLAG = 1,
		IDC_SAVE_ITEM_FLAG = 2,
	};

	static LRESULT CALLBACK DlgProcMain(HWND h, UINT Msg,WPARAM wParam, LPARAM lParam);

	static LRESULT onInitDialog(HWND h);
	static LRESULT onTreeSelChange(HWND h, const NMTREEVIEW* nmTree);
	static LRESULT onSpinScroll(HWND h, HWND spin);
	static LRESULT onDragBegin(HWND h, const NMTREEVIEW* nmTree);
	static LRESULT onDragMove(HWND h, LPARAM lParam);
	static LRESULT onDragEnd(HWND h);
	static LRESULT onClientIdChange(HWND h, HWND hEdit);
	static LRESULT onImportOld(HWND h);
	static LRESULT onSaveOtb(HWND h);
	static LRESULT onLoadOtb(HWND h);
	static LRESULT onAutoFindImages(HWND h);
	static LRESULT onCreateMissing(HWND h);
	static LRESULT onGotoItem(HWND h);
	static LRESULT onContextMenu(HWND h, unsigned long lParam);
	static LRESULT onContextMenuClick(HWND h, unsigned long newgroup);

	static void setControlState(HWND h, unsigned long flagsEdit, unsigned long flagsOpt, 
			unsigned long flagsCombo, unsigned long flagsButton);

	static void getItemTypeName(const ItemType *iType,char *name);

	//edit
	static void setEditTextInt(HWND h, int button, int value);
	static void setEditTextDouble(HWND h, int button, double value);
	static bool getEditTextInt(HWND h, int button, int &value);
	static bool getEditTextDouble(HWND h, int button, double &value);
	//option
	static void setCheckButton(HWND h, int button, bool value);
	static bool getCheckButton(HWND h, int button);
	//combo
	static void createItemCombo(HWND hcombo, const char* name, long value);
	static void setComboValue(HWND h, int combo, int value);
	static int getComboValue(HWND h, int button);
	//treeview
	static void createGroupsTree(HWND htree);
	static HTREEITEM insertTreeItem(HWND h, const char* name, HTREEITEM parent, long entryID);
	static HTREEITEM insertTreeItemType(HWND h, const ItemType *iType);
	//context menu
	static void setContextMenuGroup(itemgroup_t group);

	static void invalidateSprite(HWND h);

	static bool saveCurrentItem(HWND h);
	static void loadItem(HWND h);
	static void updateControls(HWND h);
	static void loadTreeItemTypes(HWND h, bool notFound = false);
	static void changeGroup(HWND h, HTREEITEM htItem, HTREEITEM newParent);

	static long curItemClientId;
	static long curItemServerId;
	static GUIDraw* drawEngine;
	static HWND m_hwndTree;

	static unsigned long menuGroups[ITEM_GROUP_LAST];
	static HMENU popupMenu;
	static HTREEITEM rootItems[ITEM_GROUP_LAST];
	static HTREEITEM curItem;

	//dragging variables
	static bool m_dragging;
	static HTREEITEM m_dragItem;

	static bool autoFindPerformed;

private:
};

//////////////////////////////////////////////////////////////////////////////////////

typedef std::map<unsigned long, HBITMAP> BitmapMap;

class GUIDraw{
public:
	GUIDraw();
	~GUIDraw();
	bool drawSprite(HDC desthdc, long x, long y, long maxx, long maxy, unsigned long itemid, bool drawFrame = false);
	HBITMAP getBitmap(const InternalSprite sprite);
	void releaseBitmaps();

private:
	
	//gdi objects
	static HDC m_auxHDC;
	static HBITMAP m_oldauxBMP;
	static HBITMAP m_auxBMP;

	static BitmapMap m_bitmaps;

	friend GUIWin;
};

#endif
