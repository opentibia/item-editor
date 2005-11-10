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
#include "item_sprite.hpp"
#include "gui.hpp"

GUI *g_gui = NULL;
ItemsSprites *g_itemsSprites = NULL;
ItemsTypes *g_itemsTypes = NULL;

#if defined WIN32 || defined __WINDOWS__
//windows 
#include "windows.h"
HINSTANCE g_instance;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
#else
//linux 
#include "gtk/gtk.h"
int g_argc;
char **g_argv;
int main(int argc, char *argv[])
#endif
{
#ifdef _WINDOWS
	g_instance = hInstance;
#else
	g_argc = argc;
	g_argv = argv;
#endif
	
	//get GUI
	g_gui = GUI::getGUI();
	ASSERT( g_gui != NULL )
	
	g_itemsTypes = new ItemsTypes();

	//load sprites/items
	g_itemsSprites = new ItemsSprites();
	if(!g_itemsSprites->loadFromDat("tibia.dat"))
		g_gui->messageBox("Error while loading client's tibia.dat.", MESSAGE_TYPE_FATAL_ERROR);

	if(!g_itemsSprites->loadFromSpr("tibia.spr"))
		g_gui->messageBox("Error while loading client's tibia.spr.", MESSAGE_TYPE_FATAL_ERROR);

	#ifdef __SPRITE_SEARCH__
	g_itemsSprites->loadHash();
	#endif

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
	g_itemsSprites->unloadDat();
	g_itemsSprites->unloadSpr();
	
	return 0;
#else
	return true;
#endif
}
