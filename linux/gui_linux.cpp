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

long GUILinux::m_curItemServerId=100;
long GUILinux::m_curItemClientId=0;
/*
static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,         NULL,           0, "<Branch>" },
  { "/File/_New",     "<control>N", print_hello,    0, "<StockItem>", GTK_STOCK_NEW },
  { "/File/_Open",    "<control>O", print_hello,    0, "<StockItem>", GTK_STOCK_OPEN },
  { "/File/_Save",    "<control>S", print_hello,    0, "<StockItem>", GTK_STOCK_SAVE },
  { "/File/Save _As", NULL,         NULL,           0, "<Item>" },
  { "/File/sep1",     NULL,         NULL,           0, "<Separator>" },
  { "/File/_Quit",    "<CTRL>Q", gtk_main_quit, 0, "<StockItem>", GTK_STOCK_QUIT },
  { "/_Options",      NULL,         NULL,           0, "<Branch>" },
  { "/Options/tear",  NULL,         NULL,           0, "<Tearoff>" },
  { "/Options/Check", NULL,         print_toggle,   1, "<CheckItem>" },
  { "/Options/sep",   NULL,         NULL,           0, "<Separator>" },
  { "/Options/Rad1",  NULL,         print_selected, 1, "<RadioItem>" },
  { "/Options/Rad2",  NULL,         print_selected, 2, "/Options/Rad1" },
  { "/Options/Rad3",  NULL,         print_selected, 3, "/Options/Rad1" },
  { "/_Help",         NULL,         NULL,           0, "<LastBranch>" },
  { "/_Help/About",   NULL,         NULL,           0, "<Item>" },
};
*/
static GtkItemFactoryEntry menu_items[] = {
{ "/_File",         	NULL,		NULL,				0, "<Branch>" },
{ "/File/_New",		"<control>N",	GUILinux::onNew,		0, "<StockItem>", GTK_STOCK_NEW },
{ "/File/_Load otb",	"<control>L",	GUILinux::onLoadOtb,	0, "<StockItem>", GTK_STOCK_OPEN },
{ "/File/_Import old",	"<control>I",	NULL,				0, "<Item>" },
{ "/File/sep1",		NULL,		NULL,				0, "<Separator>" },
{ "/File/_Save as",		"<control>S",	NULL,				0, "<StockItem>", GTK_STOCK_SAVE },
{ "/File/sep2",		NULL,		NULL,				0, "<Separator>" },
{ "/File/_Quit",		"<CTRL>Q",	GUILinux::onQuit,		0, "<StockItem>", GTK_STOCK_QUIT },
{ "/_Tools",			NULL,		NULL,				0, "<Branch>" },
{ "/Tools/Verify items",	NULL,		NULL,				0, "<Item>" },
{ "/Tools/Go to item",	NULL,		NULL,				0, "<Item>" },
{ "/Tools/sep1",		NULL,		NULL,				0, "<Separator>" },
{ "/Tools/Create missing client items",	NULL,NULL,			0, "<Item>" },
{ "/Tools/sep2",		NULL,		NULL,				0, "<Separator>" },
{ "/Tools/Autofind client id",	NULL,NULL,				0, "<Item>" },
{ "/Tools/Show not found images",	NULL,NULL,				0, "<Item>" },
{ "/Tools/Show all",	NULL,		NULL,				0, "<Item>" },
{ "/Tools/sep3",		NULL,		NULL,				0, "<Separator>" },
{ "/Tools/Import xml names",	NULL,	GUILinux::onImportXmlNames,0, "<Item>" },
{ "/Tools/Export xml names",	NULL,	GUILinux::onExportXmlNames,0, "<Item>" },
{ "/_Help",			NULL,		NULL,				0, "<LastBranch>" },
{ "/_Help/About",		NULL,		NULL,				0, "<Item>" },
};

static gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

enum
{
	TREE_COL_ID,
	TREE_COL_NAME,
	TREE_COL_TOTAL
};

void getImageHash(unsigned short cid, void*output)
{
}


GUILinux::GUILinux()
{
	m_mainWindow = NULL;
	
	gtk_init(&g_argc, &g_argv);
}

GtkTreeModel *GUILinux::createTreeModel()
{
	GtkTreeStore *model;
	GtkTreeIter top;

	model = gtk_tree_store_new (TREE_COL_TOTAL, G_TYPE_INT, G_TYPE_STRING);

	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Ground",
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Container",
					-1);		
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Weapon",
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Ammunition",
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Armor",
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Rune",
					-1);		
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Teleport",
					-1);	
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Magic Field",	
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Writeable",
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Key",
					-1);
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Splash",
					-1);		
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Fluid Container",
					-1);	
	gtk_tree_store_append (model, &top, NULL);
	gtk_tree_store_set (model, &top,
					TREE_COL_ID, 0,
					TREE_COL_NAME, "Other",	
					-1);	
	
	return GTK_TREE_MODEL (model);
}

GtkWidget *GUILinux::createViewAndModel()
{
	GtkWidget *view;
	GtkTreeModel *model;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;

	view = gtk_tree_view_new ();

	col = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (col, "Item groups");

	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute  (col, renderer, "text",
								TREE_COL_NAME);

	model = createTreeModel();

	gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

	g_object_unref (model);

	return view;
}


void GUILinux::initGUI()
{
	GtkWidget *menubar, *mainVBox, *mainHBox;
	GtkWidget *view, *scrolledWindow, *fixed, *label, *frame, *vbox;
	GtkAdjustment *adj;
	
	// create main window
	m_mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(m_mainWindow), "OT Item Editor");
	gtk_widget_set_size_request(GTK_WIDGET(m_mainWindow), 640, 480);
	
	// create the main vertical box
	mainVBox = gtk_vbox_new(FALSE, 1);
	gtk_container_set_border_width(GTK_CONTAINER (mainVBox), 1);
	gtk_container_add(GTK_CONTAINER(m_mainWindow), mainVBox);
	
	// create the menubar
	menubar = createMenuBar();
	gtk_box_pack_start(GTK_BOX(mainVBox), menubar, FALSE, TRUE, 0);
	
	// create the main horizontal box
	mainHBox = gtk_hbox_new(FALSE, 1);
	gtk_container_set_border_width(GTK_CONTAINER (mainHBox), 1);
	gtk_box_pack_end(GTK_BOX(mainVBox), mainHBox, TRUE, TRUE, 0);
	
	// create the scrolled window
	scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledWindow),
							GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	
	gtk_box_pack_start (GTK_BOX(mainHBox), scrolledWindow, 
					TRUE, TRUE, 0);
	
	// create the tree
	view = createViewAndModel();
	gtk_scrolled_window_add_with_viewport (
				GTK_SCROLLED_WINDOW (scrolledWindow), view);
	
	// create the frame
	frame = gtk_frame_new("Settings");
	gtk_box_pack_start(GTK_BOX(mainHBox), frame, 
					FALSE, TRUE, 0);
	// create a new vbox
	vbox = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	// create the first fixed panel
	fixed = gtk_fixed_new();
	gtk_box_pack_start(GTK_BOX(vbox), fixed, 
					FALSE, TRUE, 0);
	
	// create the labels, checkboxes, text entries, etc.
	label = gtk_label_new("Name:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 150, 10);
	
	m_entryName = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(m_entryName), 40);
	gtk_fixed_put(GTK_FIXED(fixed), m_entryName, 150, 25);
	
	label = gtk_label_new("Description:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 150, 50);
	
	m_entryDescription = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(m_entryDescription), 40);
	gtk_fixed_put(GTK_FIXED(fixed), m_entryDescription, 150, 65);
	
	label = gtk_label_new("sid:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 20, 85);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (m_curItemServerId, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinServerId = gtk_spin_button_new (adj, 0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (m_spinServerId), TRUE);
	g_signal_connect (G_OBJECT (adj), "value_changed",
					G_CALLBACK (onSpinServerIdChange),
					(gpointer) m_spinServerId);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinServerId, 50, 85);
	
	label = gtk_label_new("cid:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 20, 110);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (m_curItemClientId, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinClientId = gtk_spin_button_new (adj, 0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (m_spinClientId), TRUE);
	g_signal_connect (G_OBJECT (adj), "value_changed",
					G_CALLBACK (onSpinClientIdChange),
					(gpointer) m_spinClientId);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinClientId, 50, 110);
	
	// second frame
	frame = gtk_frame_new("Options");
	gtk_box_pack_start(GTK_BOX(vbox), frame, 
					TRUE, TRUE, 0);
	// second fixed panel
	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(frame), fixed);
	
	// check buttons
	m_checkBlocking = gtk_check_button_new_with_label("*Blocking");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkBlocking, 0, 0);
	
	m_checkAlwaysTop = gtk_check_button_new_with_label("*Always Top");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkAlwaysTop, 0, 20);
	
	m_checkStackable = gtk_check_button_new_with_label("*Stackable");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkStackable, 0, 40);
	
	m_checkUseable = gtk_check_button_new_with_label("*Useable");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkUseable, 0, 60);
	
	m_checkNotMoveable = gtk_check_button_new_with_label("*Not Moveable");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkNotMoveable, 0, 80);
	
	m_checkReadable = gtk_check_button_new_with_label("Readable");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkReadable, 0, 100);
	
	m_checkRotable = gtk_check_button_new_with_label("*Rotable");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkRotable, 0, 120);
	
	m_checkPickupable = gtk_check_button_new_with_label("*Pickupable");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkPickupable, 0, 140);
	
	m_checkBlockProjectile = gtk_check_button_new_with_label("Block Projectile");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkBlockProjectile, 0, 160);
	
	m_checkBlockPathFind = gtk_check_button_new_with_label("Block Pathfind");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkBlockPathFind, 0, 180);
	
	m_checkHasHeight = gtk_check_button_new_with_label("HasHeight");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkHasHeight, 0, 200);
	
	m_checkCanNotDecay = gtk_check_button_new_with_label("CanNotDecay");
	gtk_fixed_put(GTK_FIXED(fixed), m_checkCanNotDecay, 0, 220);
	
	// second column of options frame
	label = gtk_label_new("Weight:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 0);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 0.01,
										1.0, 0.0);
	m_spinWeight = gtk_spin_button_new (adj, 0, 2);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinWeight, 200, 0);
	
	label = gtk_label_new("DecayTo:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 20);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinDecayTo = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinDecayTo, 200, 20);
	
	label = gtk_label_new("DecayTime:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 40);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinDecayTime = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinDecayTime, 200, 40);
	
	label = gtk_label_new("FloorChange:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 70);
	
	m_comboFloorChange = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "No change");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Down");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up North");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up South");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up East");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up West");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up NE");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up NW");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up SE");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboFloorChange), "Up SW");
	gtk_fixed_put(GTK_FIXED(fixed), m_comboFloorChange, 200, 60);
	
	label = gtk_label_new("Slot:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 100);
	
	m_comboSlot = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Default");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Head");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Body");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Legs");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Backpack");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Weapon");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Two Hand");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Boots");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Amulet");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Ring");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSlot), "Hand");
	gtk_fixed_put(GTK_FIXED(fixed), m_comboSlot, 200, 90);
	
	label = gtk_label_new("ReadOnlyID:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 120);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinReadOnlyId = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinReadOnlyId, 200, 120);
	
	label = gtk_label_new("RotateTo:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 140);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinRotateTo = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinRotateTo, 200, 140);
	
	label = gtk_label_new("*Speed:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 120, 160);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinSpeed = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinSpeed, 200, 160);
	
	// third column of options frame
	label = gtk_label_new("MaxItems:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 0);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinMaxItems = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinMaxItems, 380, 0);
	
	label = gtk_label_new("Skill:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 30);
	
	m_comboSkill = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "None");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "Sword");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "Club");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "Axe");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "Shield");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "Distance");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboSkill), "Magic");
	gtk_fixed_put(GTK_FIXED(fixed), m_comboSkill, 380, 20);
	
	label = gtk_label_new("AmuType:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 60);
	
	m_comboAmuType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboAmuType), "None");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboAmuType), "Bolt");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboAmuType), "Arrow");
	gtk_fixed_put(GTK_FIXED(fixed), m_comboAmuType, 380, 50);
	
	label = gtk_label_new("ShootType:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 90);
	
	m_comboShootType = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "None");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Bolt");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Arrow");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Fire");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Energy");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Poison Arrow");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Burst Arrow");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Throwing Star");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Throwing Knife");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Small Stone");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Sudden Death");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Large Rock");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Snowball");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Power Bolt");
	gtk_combo_box_append_text(GTK_COMBO_BOX(m_comboShootType), "Spear");
	gtk_fixed_put(GTK_FIXED(fixed), m_comboShootType, 380, 80);
	
	label = gtk_label_new("Attack:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 110);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinAttack = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinAttack, 380, 110);
	
	label = gtk_label_new("Defence:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 130);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinDefence = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinDefence, 380, 130);
	
	label = gtk_label_new("Armor:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 300, 150);
	
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 20000.0, 1.0,
										10.0, 0.0);
	m_spinArmor = gtk_spin_button_new (adj, 0, 0);
	gtk_fixed_put(GTK_FIXED(fixed), m_spinArmor, 380, 150);
	
	
	
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
	/*
	switch(type)
	{
	case MESSAGE_TYPE_FATAL_ERROR:
	case MESSAGE_TYPE_ERROR:
	case MESSAGE_TYPE_INFO:
	case MESSAGE_TYPE_NO_ICON:
	default:
		break;
	}
	*/
	dialog = gtk_message_dialog_new(GTK_WINDOW(m_mainWindow),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			text);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

GtkWidget* GUILinux::createMenuBar()
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	
	/* Make an accelerator group (shortcut keys) */
	accel_group = gtk_accel_group_new ();
	
	/* Make an ItemFactory (that makes a menubar) */
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
								accel_group);
	
	/* This function generates the menu items. Pass the item factory,
		the number of items in the array, the array itself, and any
		callback data for the the menu items. */
	gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
	
	/* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (m_mainWindow), accel_group);
	
	/* Finally, return the actual menu bar created by the item factory. */
	return gtk_item_factory_get_widget (item_factory, "<main>");
}

void GUILinux::loadItem()
{
	ItemType *iType;
	if(m_curItemServerId && (iType = g_itemsTypes->getType(m_curItemServerId))){
		
		gtk_entry_set_text(GTK_ENTRY(m_entryName), iType->name);
		gtk_entry_set_text(GTK_ENTRY(m_entryDescription), iType->descr);
		
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


//
// EVENTS
//
void GUILinux::onNew()
{
	//autoFindPerformed = false;
	g_itemsTypes->clear();
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
		g_gui->loadItem();
		g_free(filename);
	}
	gtk_widget_destroy(pFileSelection);
}

void GUILinux::onQuit()
{
	gtk_main_quit();
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

void GUILinux::onSpinClientIdChange(GtkWidget *widget, GtkSpinButton *spin)
{
	g_gui->m_curItemClientId = gtk_spin_button_get_value_as_int (spin);
}

void GUILinux::onSpinServerIdChange(GtkWidget *widget, GtkSpinButton *spin)
{
	g_gui->m_curItemServerId = gtk_spin_button_get_value_as_int (spin);
	g_gui->loadItem();
}

void GUILinux::onDestroyEvent(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

gboolean GUILinux::onDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// TODO: Dialog asking to close or not
	
	return FALSE;
}
