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

#ifdef __OLDINCLUDE__
extern "C" WINGDIAPI BOOL  WINAPI TransparentBlt(IN HDC,IN int,IN int,IN int,IN int,IN HDC,IN int,IN int,IN int,IN int,IN UINT);
#endif

GUIDraw* GUIWin::drawEngine = NULL;
long GUIWin::curItemClientId = 0;

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
				//return onTreeCustomDraw(h, (NMTVCUSTOMDRAW*)lParam);
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
	case WM_COMMAND:
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
	case WM_SIZE:
		//return onMainSize(h,wParam,lParam);
		return TRUE;
		break;
	}
	return FALSE;
}


bool GUIWin::onInitDialog(HWND h)
{
	createItemsTree(GetDlgItem(h, IDC_EDITOR_TREE));

	SendMessage(GetDlgItem(h, IDC_SPINCID),UDM_SETRANGE,0,MAKELONG((short) SpriteType::maxClientId, (short)SpriteType::minClientId));

	char *a = "test 1";
	SendMessage(GetDlgItem(h, IDC_COMBO_FLOOR),CB_ADDSTRING, 0, (long)a);
	char *b = "test 2";
	SendMessage(GetDlgItem(h, IDC_COMBO_FLOOR),CB_ADDSTRING, 0, (long)b);
	SendMessage(GetDlgItem(h, IDC_COMBO_FLOOR),CB_ADDSTRING, 0, (long)b);

	//EnableWindow(GetDlgItem(h, IDC_SPINCID),false);
	//EnableWindow(GetDlgItem(h, IDC_EDITCID),false);
	return TRUE;
}

void GUIWin::createItemsTree(HWND htree)
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

bool GUIWin::onSpinScroll(HWND h, HWND spin)
{
	long pos = SendMessage(spin,UDM_GETPOS, 0,0);
	curItemClientId = pos;
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
	return true;
}

/*
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
		*//*
		return TRUE;
	}
	return FALSE;
}
*/

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
