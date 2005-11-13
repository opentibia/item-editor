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

#ifndef __OTITEMEDITOR_GUI_LINUX_H__
#define __OTITEMEDITOR_GUI_LINUX_H__

#include "gtk/gtk.h"

#include "../client/gui.hpp"
#include "../client/item_type.hpp"
#include "../client/item_sprite.hpp"


class GUILinux: public GUI {
public:
	GUILinux();
	virtual ~GUILinux(){};
	virtual void initGUI();
	virtual void messageBox(const char* text, MesageBoxType_t type);
	/*
	virtual bool loadSpriteInternal(const unsigned char *, const unsigned long, InternalSprite * );
	virtual void loadSpriteInternalTransparent(unsigned long color,  InternalSprite *sprite);
	virtual void unloadSpriteInternal(InternalSprite);
	*/
	
	static void onNew();
	static void onLoadOtb();
	static void onQuit();
	static void onImportXmlNames();
	static void onExportXmlNames();
	
private:
	
	GtkWidget* createMenuBar();
	GtkTreeModel *createTreeModel();
	GtkWidget *createViewAndModel();
	void loadItem();
	// Events
	static void onDestroyEvent(GtkWidget *widget, gpointer data);
	static gboolean onDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data);
	static void onSpinClientIdChange(GtkWidget *widget, GtkSpinButton *spin);
	static void onSpinServerIdChange(GtkWidget *widget, GtkSpinButton *spin);
	
	//vars
	static long m_curItemServerId;
	static long m_curItemClientId;
	
	GtkWidget *m_mainWindow;
	GtkWidget *m_entryName;
	GtkWidget *m_entryDescription;
	GtkWidget *m_spinClientId;
	GtkWidget *m_spinServerId;
	GtkWidget *m_checkBlocking;
	GtkWidget *m_checkAlwaysTop;
	GtkWidget *m_checkStackable;
	GtkWidget *m_checkUseable;
	GtkWidget *m_checkNotMoveable;
	GtkWidget *m_checkReadable;
	GtkWidget *m_checkRotable;
	GtkWidget *m_checkPickupable;
	GtkWidget *m_checkBlockProjectile;
	GtkWidget *m_checkBlockPathFind;
	GtkWidget *m_checkHasHeight;
	GtkWidget *m_checkCanNotDecay;
	GtkWidget *m_spinWeight;
	GtkWidget *m_spinDecayTo;
	GtkWidget *m_spinDecayTime;
	GtkWidget *m_comboFloorChange;
	GtkWidget *m_comboSlot;
	GtkWidget *m_spinReadOnlyId;
	GtkWidget *m_spinRotateTo;
	GtkWidget *m_spinSpeed;
	GtkWidget *m_spinMaxItems;
	GtkWidget *m_comboSkill;
	GtkWidget *m_comboAmuType;
	GtkWidget *m_comboShootType;
	GtkWidget *m_spinAttack;
	GtkWidget *m_spinDefence;
	GtkWidget *m_spinArmor;
};

#endif
