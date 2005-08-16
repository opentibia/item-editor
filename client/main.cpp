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
#include "definitions.hpp"

#include "item_type.hpp"
#include "gui.hpp"

GUI *g_gui = NULL;
Items *g_items = NULL;

#if defined WIN32 || defined __WINDOWS__
//windows 
#include "windows.h"
HINSTANCE g_instance;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
#else
//linux 
#include "wx/wx.h"
class OtItemEditorApp: public wxApp { 
    virtual bool OnInit(); 
    virtual int OnExit(); 
};
IMPLEMENT_APP(OtItemEditorApp)

bool OtItemEditorApp::OnInit()
#endif
{
#ifdef _WINDOWS
	g_instance = hInstance;
#endif
	
	//get GUI
	g_gui = GUI::getGUI();
	ASSERT( g_gui != NULL )
	
	//load sprites/items
	g_items = new Items();
	g_items->loadFromDat("tibia.dat");
	g_items->loadFromSpr("tibia.spr");

	if(g_gui)
	{
		//init gui application
		g_gui->initGUI();
	}

	/*!
	TODO: Check the returning value in Windows.
	Btw, for the wxWidgets I need to put the unloading functions in another place :P.
	*/
#if defined WIN32 || defined __WINDOWS__
	//unloading sprites/items
	g_items->unloadDat();
	g_items->unloadSpr();
	
	return 0;
#else
	return true;
#endif
}

#ifndef WIN32
int OtItemEditorApp::OnExit() 
{ 
	//unloading sprites/items
	g_items->unloadDat();
	g_items->unloadSpr();
	
	return 0;
}
#endif
