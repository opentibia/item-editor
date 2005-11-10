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

#include "gui_linux.hpp"
#include "../client/md5.h"

extern int g_argc;
extern char **g_argv;

void getImageHash(unsigned short cid, void*output)
{
}


GUILinux::GUILinux()
{
	m_mainWindow = NULL;
	
	gtk_init(&g_argc, &g_argv);
}

void GUILinux::initGUI()
{
	// create main window
	m_mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	
	
	// Connect signals
	g_signal_connect(G_OBJECT(m_mainWindow), "destroy", G_CALLBACK(onDestroyEvent), NULL);
	g_signal_connect(G_OBJECT(m_mainWindow), "delete_event", G_CALLBACK(onDeleteEvent), NULL);
	
	// show all windows
	gtk_widget_show_all(m_mainWindow);
	// gtk main loop
	gtk_main();
}

void GUILinux::messageBox(const char* text, MesageBoxType_t type)
{
	GtkWidget *dialog;
	
	switch(type)
	{
	case MESSAGE_TYPE_FATAL_ERROR:
	case MESSAGE_TYPE_ERROR:
	case MESSAGE_TYPE_INFO:
	case MESSAGE_TYPE_NO_ICON:
	default:
		break;
	}
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(m_mainWindow),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			text);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


//
// EVENTS
//

void GUILinux::onDestroyEvent(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

gboolean GUILinux::onDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// TODO: Dialog asking to close or not
	
	return FALSE;
}
