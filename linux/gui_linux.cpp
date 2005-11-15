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
extern ItemsSprites *g_itemsSprites;
extern ItemsTypes *g_itemsTypes;
extern GUILinux *g_gui;

static GladeXML *g_xmlInterface;
long GUILinux::m_curItemServerId=100;
long GUILinux::m_curItemClientId=0;

/////////////////////////////////////////////
/// BEGIN --- GLADE CALLBACKS
////////////////////////////////////////////
void on_menu_new_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_gui->onNew();
}

void on_menu_load_otb_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_gui->onLoadOtb();
}

void on_menu_save_otb_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_gui->onSaveOtb();
}

void on_import_xml_names_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_gui->onImportXmlNames();
}

void on_export_xml_names_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_gui->onExportXmlNames();
}

void on_spinServerId_value_changed (GtkSpinButton *spinbutton, GtkScrollType scroll, gpointer user_data)
{
	g_gui->onSpinServerIdChange();
}

void on_spinClientId_value_changed (GtkSpinButton *spinbutton, GtkScrollType scroll, gpointer user_data)
{
	g_gui->onSpinClientIdChange();
}

void on_buttonSaveItem_clicked (GtkButton *button, gpointer user_data)
{
	g_gui->saveCurrentItem();
}

/////////////////////////////////////////////
/// END --- GLADE CALLBACKS
////////////////////////////////////////////



//TODO: move this function to a common source for windows and linux
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
				InternalSprite sprite = g_itemsSprites->getSpriteInternalFormat(cid,frameindex);
				if(sprite)
					MD5Update(&m_md5, (const unsigned char*)sprite, 32*32*4);
			}
		}
	}

	MD5Final(&m_md5);

	memcpy(output, m_md5.digest, 16);
}

GUILinux::GUILinux()
{
	gtk_init(&g_argc, &g_argv);
	glade_init();
	
	g_xmlInterface = glade_xml_new("gui-otie.glade", NULL, NULL);
}

void GUILinux::initGUI()
{
	// set some widget pointers
	m_windowMain = glade_xml_get_widget(g_xmlInterface, "windowMain");
	m_imageSprite = glade_xml_get_widget(g_xmlInterface, "imageSprite");
	
	GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,34,34);
	for(int x=0; x < 33; ++x)
	{
		putPixel(pixbuf, x, 0, 0xFF, 0x00, 0x00, 0xFF);
		putPixel(pixbuf, x, 33, 0xFF, 0x00, 0x00, 0xFF);
	}
	for(int y=1; y < 33; ++y)
	{
		putPixel(pixbuf, 0, y, 0xFF, 0x00, 0x00, 0xFF);
		putPixel(pixbuf, 33, y, 0xFF, 0x00, 0x00, 0xFF);
	}
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_imageSprite), pixbuf);
	
	m_entryName = glade_xml_get_widget(g_xmlInterface, "entryName");
	m_entryDescription = glade_xml_get_widget(g_xmlInterface, "entryDescription");
	m_spinServerId = glade_xml_get_widget(g_xmlInterface, "spinServerId");
	m_spinClientId = glade_xml_get_widget(g_xmlInterface, "spinClientId");
	m_labelInfo = glade_xml_get_widget(g_xmlInterface, "labelInfo");
	
	m_checkBlocking = glade_xml_get_widget(g_xmlInterface, "checkBlocking");
	m_checkAlwaysTop = glade_xml_get_widget(g_xmlInterface, "checkAlwaysTop");
	m_checkStackable = glade_xml_get_widget(g_xmlInterface, "checkStackable");
	m_checkUseable = glade_xml_get_widget(g_xmlInterface, "checkUseable");
	m_checkNotMoveable = glade_xml_get_widget(g_xmlInterface, "checkNotMoveable");
	m_checkReadable = glade_xml_get_widget(g_xmlInterface, "checkReadable");
	m_checkRotable = glade_xml_get_widget(g_xmlInterface, "checkRotable");
	m_checkPickupable = glade_xml_get_widget(g_xmlInterface, "checkPickupable");
	m_checkBlockProjectile = glade_xml_get_widget(g_xmlInterface, "checkBlockProjectile");
	m_checkBlockPathFind = glade_xml_get_widget(g_xmlInterface, "checkBlockPathFind");
	m_checkHasHeight = glade_xml_get_widget(g_xmlInterface, "checkHasHeight");
	m_checkCanNotDecay = glade_xml_get_widget(g_xmlInterface, "checkCanNotDecay");
	
	m_spinWeight = glade_xml_get_widget(g_xmlInterface, "spinWeight");
	m_spinDecayTo = glade_xml_get_widget(g_xmlInterface, "spinDecayTo");
	m_spinDecayTime = glade_xml_get_widget(g_xmlInterface, "spinDecayTime");
	m_comboFloorChange =  glade_xml_get_widget(g_xmlInterface, "comboFloorChange");
	m_comboSlot =  glade_xml_get_widget(g_xmlInterface, "comboSlot");
	m_spinReadOnlyId = glade_xml_get_widget(g_xmlInterface, "spinReadOnlyId");
	m_spinRotateTo = glade_xml_get_widget(g_xmlInterface, "spinRotateTo");
	m_spinSpeed = glade_xml_get_widget(g_xmlInterface, "spinSpeed");
	
	m_spinMaxItems = glade_xml_get_widget(g_xmlInterface, "spinMaxItems");
	m_comboSkill =  glade_xml_get_widget(g_xmlInterface, "comboSkill");
	m_comboAmuType =  glade_xml_get_widget(g_xmlInterface, "comboAmuType");
	m_comboShootType =  glade_xml_get_widget(g_xmlInterface, "comboShootType");
	m_spinAttack = glade_xml_get_widget(g_xmlInterface, "spinAttack");
	m_spinDefence = glade_xml_get_widget(g_xmlInterface, "spinDefence");
	m_spinArmor = glade_xml_get_widget(g_xmlInterface, "spinArmor");
	
	// set some signals of the widgets
	glade_xml_signal_autoconnect(g_xmlInterface);
	
	// gtk main loop
	gtk_main();
}

void GUILinux::messageBox(const char* text, MesageBoxType_t type)
{
	GtkWidget *dialog;
	GtkMessageType mtype;
	
	switch(type)
	{
	case MESSAGE_TYPE_FATAL_ERROR:
	case MESSAGE_TYPE_ERROR:
		mtype = GTK_MESSAGE_ERROR;
		break;
	case MESSAGE_TYPE_INFO:
	case MESSAGE_TYPE_NO_ICON:
	default:
		mtype = GTK_MESSAGE_INFO;
		break;
	}
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(m_windowMain),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			mtype,
			GTK_BUTTONS_CLOSE,
			text);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

bool GUILinux::loadSpriteInternal(const unsigned char *dump, const unsigned long size, InternalSprite *sprite)
{
	return false;
}

void GUILinux::loadItem()
{
	ItemType *iType;
	if(m_curItemServerId && (iType = g_itemsTypes->getType(m_curItemServerId)))
	{
		gtk_entry_set_text(GTK_ENTRY(m_entryName), iType->name);
		gtk_entry_set_text(GTK_ENTRY(m_entryDescription), iType->descr);
		gtk_label_set_text(GTK_LABEL(g_gui->m_labelInfo), "");
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinClientId), iType->clientid);
		
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkBlocking), iType->blockSolid);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkAlwaysTop), iType->alwaysOnTop);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkStackable), iType->stackable);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkUseable), iType->useable);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkNotMoveable), !iType->moveable);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkReadable), iType->readable);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkRotable), iType->rotable);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkPickupable), iType->pickupable);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkBlockProjectile), iType->blockProjectile);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkBlockPathFind), iType->blockPathFind);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkHasHeight), iType->hasHeight);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkCanNotDecay), iType->canNotDecay);
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinWeight), iType->weight);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinDecayTo), iType->decayTo);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinDecayTime), iType->decayTime);
		if(iType->floorChangeDown)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 1);
		else if(iType->floorChangeNorth && iType->floorChangeEast)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 6);
		else if(iType->floorChangeSouth && iType->floorChangeEast)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 8);
		else if(iType->floorChangeNorth && iType->floorChangeWest)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 7);
		else if(iType->floorChangeSouth && iType->floorChangeWest)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 9);
		else if(iType->floorChangeNorth)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 2);
		else if(iType->floorChangeSouth)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 3);
		else if(iType->floorChangeEast)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 4);
		else if(iType->floorChangeWest)
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 5);
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 0);
		
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboSlot), iType->slot_position);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinReadOnlyId), iType->readOnlyId);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinRotateTo), iType->rotateTo);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinSpeed), iType->speed);
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinMaxItems), iType->maxItems);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboSkill), iType->weaponType);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboAmuType), iType->amuType);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboShootType), iType->shootType);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinAttack), iType->attack);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinDefence), iType->defence);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinArmor), iType->armor);
	}
	else{
		gtk_entry_set_text(GTK_ENTRY(m_entryName), "");
		gtk_entry_set_text(GTK_ENTRY(m_entryDescription), "");
		gtk_label_set_text(GTK_LABEL(g_gui->m_labelInfo), "** Current Item Not Defined **");
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinClientId), 0);
		
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkBlocking), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkAlwaysTop), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkStackable), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkUseable), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkNotMoveable), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkReadable), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkRotable), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkPickupable), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkBlockProjectile), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkBlockPathFind), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkHasHeight), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkCanNotDecay), FALSE);
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinWeight), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinDecayTo), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinDecayTime), 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboFloorChange), 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboSlot), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinReadOnlyId), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinRotateTo), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinSpeed), 0);
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinMaxItems), 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboSkill), 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboAmuType), 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboShootType), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinAttack), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinDefence), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinArmor), 0);
	}
}

void GUILinux::drawCurrentSprite()
{
	GdkPixbuf *pixbuf;
	char dump[4000];
	unsigned short size;
	unsigned short x, y, npix, sprite_id;
	int state;
	int i,j;
	int posx=1, posy=1;
	
	sprite_id = g_itemsSprites->getItemSpriteID(m_curItemClientId, 0);
	if(sprite_id <= 1)
	{
		onSpriteImageNotFound();
		return;
	}
	
	size = g_itemsSprites->getSpriteDump(sprite_id, dump);
	if(size < 1)
	{
		onSpriteImageNotFound();
		return;
	}
	
	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(m_imageSprite));
	
	state=0;
	x=0;
	y=0;
	for(i=0; i < size;)
	{
		memcpy(&npix, dump+i, 2); // number of pixels (transparent or defined)
		i += 2;
		//std::cout << "<< " << (int)npix << " / " << (int)i<<"\n";
		switch(state)
		{
		default: // state 0, drawing transparent pixels
			for(j=0; j < npix; ++j)
			{
				putPixel(pixbuf, posx+x, posy+y, 0x00, 0x00, 0x00, 0x00);
				if(x < 31)
					x++;
				else
				{
					x=0;
					y++;
				}
			}
			state=1;
			break;
		case 1: // state 1, drawing defined pixels
			for(j=0; j < npix; ++j)
			{
				putPixel(pixbuf, posx+x, posy+y, dump[i], dump[i+1], dump[i+2], 0xFF);
				//std::cout << "(" << (int)dump[i] << "|" << (int)dump[i+1] << "|" << (int)dump[i+2] << "\n"; 
				i += 3;
				if(x < 31)
				{
					x++;
				}
				else
				{
					x=0;
					y++;
				}
			}
			state=0;
			break;
		}
	}
	while(x < 32 && y < 32)
	{
		putPixel(pixbuf, posx+x, posy+y, 0x00, 0x00, 0x00, 0x00);
		if(x < 31)
			x++;
		else
		{
			x=0;
			y++;
		}
	}
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_imageSprite), pixbuf);
}

void GUILinux::onSpriteImageNotFound()
{
	GdkPixbuf *pixbuf;
	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(m_imageSprite));
	
	for(int x=1; x < 33; ++x)
		for(int y=1; y < 33; ++y)
			putPixel(pixbuf, x, y, 0x00, 0x00, 0x00, 0x00);
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_imageSprite), pixbuf);
}

void GUILinux::putPixel( GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha)
{
	int width, height, rowstride, n_channels;
	guchar *pixels, *p;
	
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	
	g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
	g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
	g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
	g_assert (n_channels == 4);
	
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	
	g_assert (x >= 0 && x < width);
	g_assert (y >= 0 && y < height);
	
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	
	p = pixels + y * rowstride + x * n_channels;
	p[0] = red;
	p[1] = green;
	p[2] = blue;
	p[3] = alpha;
}

//
// EVENTS
//
void GUILinux::onNew()
{
	//autoFindPerformed = false;
	g_itemsTypes->clear();
	g_gui->loadItem();
	//loadTreeItemTypes(h);
}

void GUILinux::onLoadOtb()
{
	GtkWidget *pFileSelection;
	GtkFileFilter *filter;
	
	
	pFileSelection = gtk_file_chooser_dialog_new("Load items.otb",
									NULL,
									GTK_FILE_CHOOSER_ACTION_OPEN,
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									NULL);
	
	gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.otb");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pFileSelection), filter);
	
	if(gtk_dialog_run(GTK_DIALOG(pFileSelection)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pFileSelection));
		
		g_itemsTypes->clear();
		g_itemsTypes->loadOtb(filename);
		
		//autoFindPerformed = false;
		//loadTreeItemTypes(h);
		loadItem();
		g_free(filename);
	}
	gtk_widget_destroy(pFileSelection);
}

void GUILinux::onSaveOtb()
{
	GtkWidget *pFileSelection;
	
	pFileSelection = gtk_file_chooser_dialog_new("Save items.otb",
									NULL,
									GTK_FILE_CHOOSER_ACTION_SAVE,
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
									NULL);
	
	gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);
	if(gtk_dialog_run(GTK_DIALOG(pFileSelection)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pFileSelection));
		
		g_itemsTypes->saveOtb(filename);
		
		g_free(filename);
	}
	gtk_widget_destroy(pFileSelection);
}

void GUILinux::onImportXmlNames()
{
	GtkWidget *pFileSelection;
	GtkFileFilter *filter;
	
	
	pFileSelection = gtk_file_chooser_dialog_new("Import XML names",
									NULL,
									GTK_FILE_CHOOSER_ACTION_OPEN,
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									NULL);
	
	gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pFileSelection), filter);
	
	if(gtk_dialog_run(GTK_DIALOG(pFileSelection)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pFileSelection));
		
		g_itemsTypes->importFromXml(filename);
		//loadTreeItemTypes(h);
		g_free(filename);
	}
	gtk_widget_destroy(pFileSelection);
}

void GUILinux::onExportXmlNames()
{
	GtkWidget *pFileSelection;
	
	pFileSelection = gtk_file_chooser_dialog_new("Export XML names",
									NULL,
									GTK_FILE_CHOOSER_ACTION_SAVE,
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
									NULL);
	
	gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);
	if(gtk_dialog_run(GTK_DIALOG(pFileSelection)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pFileSelection));
		
		g_itemsTypes->exportToXml(filename);
		
		g_free(filename);
	}
	gtk_widget_destroy(pFileSelection);
}

void GUILinux::onSpinClientIdChange()
{
	m_curItemClientId = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(m_spinClientId));
	drawCurrentSprite();
}

void GUILinux::onSpinServerIdChange()
{
	m_curItemServerId = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(m_spinServerId));
	loadItem();
}

void GUILinux::saveCurrentItem()
{
	ItemType *iType=NULL;
	
	if(!g_gui->m_curItemServerId)
	{
		g_gui->messageBox("Failed to save the current item.\n(sid == 0)", MESSAGE_TYPE_ERROR);
		return;
	}

	if(g_gui->m_curItemServerId >= 100 && !(iType = g_itemsTypes->getType(g_gui->m_curItemServerId)))
	{
		g_gui->messageBox("Failed to save the current item.\n(sid < 100 or sid don't exist)", MESSAGE_TYPE_ERROR);
		return;
	}

	if(!g_gui->m_curItemClientId)
	{
		g_gui->messageBox("Failed to save the current item.\n(cid == 0)", MESSAGE_TYPE_ERROR);
		return;
	}
	
	strcpy(iType->name, gtk_entry_get_text(GTK_ENTRY(g_gui->m_entryName)));
	strcpy(iType->descr, gtk_entry_get_text(GTK_ENTRY(g_gui->m_entryDescription)));
	
	iType->clientid = g_gui->m_curItemClientId;
	
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
		iType->miniMapColor = sType->miniMapColor;
		iType->subParam07 = sType->subParam07;
		iType->subParam08 = sType->subParam08;
	}
	
	iType->blockSolid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkBlocking));
	iType->alwaysOnTop = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkAlwaysTop));
	iType->stackable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkStackable));
	iType->useable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkUseable));
	iType->moveable = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkNotMoveable));
	iType->pickupable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkPickupable));
	iType->rotable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkRotable));
	iType->blockProjectile = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkBlockProjectile));
	iType->readable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkReadable));
	iType->blockPathFind = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkBlockPathFind));
	iType->hasHeight = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkHasHeight));
	iType->canNotDecay = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_gui->m_checkCanNotDecay));

	iType->weight = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(g_gui->m_spinWeight));
	iType->decayTo = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinDecayTo));
	iType->decayTime = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinDecayTime));
	iType->attack = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinAttack));
	iType->defence = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinDefence));
	iType->armor = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinArmor));
	iType->maxItems = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinMaxItems));
	iType->speed = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinSpeed));
	iType->readOnlyId = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinReadOnlyId));
	iType->rotateTo = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g_gui->m_spinRotateTo));

	iType->slot_position = (slots_t)gtk_combo_box_get_active(GTK_COMBO_BOX(g_gui->m_comboSlot));
	iType->weaponType = (WeaponType)gtk_combo_box_get_active(GTK_COMBO_BOX(g_gui->m_comboSkill));
	iType->amuType = (amu_t)gtk_combo_box_get_active(GTK_COMBO_BOX(g_gui->m_comboAmuType));
	iType->shootType = (subfight_t)gtk_combo_box_get_active(GTK_COMBO_BOX(g_gui->m_comboShootType));
	
	iType->floorChangeDown = false;
	iType->floorChangeNorth = false;
	iType->floorChangeSouth = false;
	iType->floorChangeEast = false;
	iType->floorChangeWest = false;
	switch(gtk_combo_box_get_active(GTK_COMBO_BOX(g_gui->m_comboFloorChange)))
	{
	case 1: // Down
		iType->floorChangeDown = true;
		break;
	case 2: // Up North
		iType->floorChangeNorth = true;
		break;
	case 3: // Up South
		iType->floorChangeSouth = true;
		break;
	case 4: // Up East
		iType->floorChangeEast = true;
		break;
	case 5: // Up West
		iType->floorChangeWest = true;
		break;
	case 6: // Up NE
		iType->floorChangeNorth = true;
		iType->floorChangeEast = true;
		break;
	case 7: // Up NW
		iType->floorChangeNorth = true;
		iType->floorChangeWest = true;
		break;
	case 8: // Up SE
		iType->floorChangeSouth = true;
		iType->floorChangeEast = true;
		break;
	case 9: // Up SW
		iType->floorChangeSouth = true;
		iType->floorChangeWest = true;
		break;
	case 0: // No Change
	default:
		break;
	}
	/*
	//change name in tree
	TVITEM itemInfo;
	char name[160];
	itemInfo.mask = TVIF_HANDLE | TVIF_TEXT;
	getItemTypeName(iType, name);

	itemInfo.hItem = curItem;
	itemInfo.pszText = name;
	itemInfo.cchTextMax = strlen(name);
	SendMessage(m_hwndTree, TVM_SETITEM, 0, (long)&itemInfo);
	*/
}

