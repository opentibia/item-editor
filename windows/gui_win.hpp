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
	virtual bool loadSpriteInternal(const unsigned char *, const unsigned long, InternalSprite * );
	virtual void loadSpriteInternalTransparent(unsigned long color,  InternalSprite *sprite);
	virtual void unloadSpriteInternal(InternalSprite);

protected:
	static LRESULT CALLBACK DlgProcMain(HWND h, UINT Msg,WPARAM wParam, LPARAM lParam);
	
	//static bool drawSprite(HDC desthdc, long x, long y, long maxx, long maxy, unsigned long itemid, unsigned long count, bool drawFrame /*= false*/, bool animation /*=false*/);
	//static bool drawItem(HDC desthdc, long x, long y, long maxx, long maxy, ItemBase* item, bool drawFrame /*=false*/,bool animation /*=false*/);

	//treeview
	static void createEditorTree(HWND htree);
	static HTREEITEM insterTreeItem(HWND h, char* name, HTREEITEM parent, long size, long entryID);
	static bool onTreeCustomDraw(HWND h, NMTVCUSTOMDRAW* lParam);

	static GUIDraw* drawEngine;

private:
};


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
