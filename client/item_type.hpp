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

#ifndef __OTITEMEDITOR_ITEM_TYPE_H__
#define __OTITEMEDITOR_ITEM_TYPE_H__

#include "expat.h"
#include <map>

enum itemgroup_t{
	ITEM_GROUP_NONE,
	ITEM_GROUP_GROUND,
	ITEM_GROUP_CONTAINER,
	ITEM_GROUP_WEAPON,
	ITEM_GROUP_AMMUNITION,
	ITEM_GROUP_ARMOR,
	ITEM_GROUP_RUNE,
	ITEM_GROUP_TELEPORT,
	ITEM_GROUP_MAGICFIELD,
	ITEM_GROUP_WRITEABLE,
	ITEM_GROUP_KEY,
	ITEM_GROUP_SPLASH
};

enum slots_t {
	SLOT_HEAD=1,
	SLOT_NECKLACE=2,
	SLOT_BACKPACK=3,
	SLOT_ARMOR=4,
	SLOT_HAND=5,
	SLOT_LEGS=6,
	SLOT_FEET=7,
	SLOT_RING=8,
	SLOT_AMMO=9
};

enum WeaponType {
  WEAPON_NONE   = 0,
	WEAPON_SWORD  = 1,
	WEAPON_CLUB   = 2,
	WEAPON_AXE    = 3,
	WEAPON_DIST   = 4,
	WEAPON_MAGIC  = 5,
	WEAPON_AMO    = 6,
	WEAPON_SHIELD = 7
};

enum subfight_t {
	DIST_NONE          = 0,
	DIST_BOLT          = 1,
  DIST_ARROW         = 2,
  DIST_FIRE          = 3,
  DIST_ENERGY        = 4,
  DIST_POISONARROW   = 5,
  DIST_BURSTARROW    = 6,
  DIST_THROWINGSTAR  = 7,
  DIST_THROWINGKNIFE = 8,
  DIST_SMALLSTONE    = 9,
  DIST_SUDDENDEATH   = 10,
  DIST_LARGEROCK     = 11,
  DIST_SNOWBALL      = 12,
  DIST_POWERBOLT     = 13,
  DIST_SPEAR         = 14,
  DIST_POISONFIELD   = 15
};

enum amu_t{
	AMU_NONE = 0,
	AMU_BOLT = 1,
	AMU_ARROW = 2
};

enum magicfield_t {
	MAGIC_FIELD_FIRE,
	MAGIC_FIELD_POISON,
	MAGIC_FIELD_ENERGY,	
};

struct xmlProp {
	char *name;
	char *value;
};


class ItemType
{
public:
	ItemType();
	~ItemType(){}
	
	itemgroup_t group;
	unsigned short id;
	bool groundtile;
	bool blocking;
	bool alwaysOnTop;
	bool container;
	bool stackable;
	bool useable;
	bool notMoveable;
	bool pickupable;
	bool fluidcontainer;
	bool creature;

	//xml data
	char name[128];
	char descr[128];
	double weight;
  unsigned short	decayTo;
  unsigned short	decayTime;
	bool blockingProjectile;
	bool floorchange;
	int slot_position;

	bool floorChangeNorth;
	bool floorChangeSouth;
	bool floorChangeEast;
	bool floorChangeWest;

	//container
	int maxItems;

	//weapon
	amu_t amuType;
	WeaponType weaponType;
  subfight_t shootType;
  int attack;
  int	defence;

	//armor
	int armor;
	
	//rune
	int runeMagLevel;

	//teleport
	bool isteleport;

	//magicfield
	bool ismagicfield;
	int magicfieldtype;

	//writeable
	unsigned short readonlyId;

	//key
	bool iskey;

	//splash
	bool issplash;	
};


class ItemsTypes {
public:
	bool loadFromXml(const char *filename);

	bool setGroup(int id, itemgroup_t newgroup);
	ItemType* getItem(int id);

private:
	static int loadstatus;
	static void XMLCALL xmlstartNode(void *userData, const char *name, const char **atts);
	static void XMLCALL xmlendNode(void *userData, const char *name);

	typedef std::map<unsigned short, ItemType*> ItemMap;
	ItemMap item;
};

#endif 
