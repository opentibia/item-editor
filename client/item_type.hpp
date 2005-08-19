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
	ITEM_GROUP_SPLASH,
	ITEM_GROUP_FLUID,
	ITEM_GROUP_LAST
};

enum slots_t {
	SLOT_DEFAULT,
	SLOT_HEAD,
	SLOT_BODY,
	SLOT_LEGS,
	SLOT_BACKPACK,
	SLOT_WEAPON,
	SLOT_2HAND,
	SLOT_FEET,
	SLOT_AMULET,
	SLOT_RING,
	SLOT_HAND,
};

enum WeaponType {
	WEAPON_NONE,
	WEAPON_SWORD,
	WEAPON_CLUB,
	WEAPON_AXE,
	WEAPON_SHIELD,
	WEAPON_DIST,
	WEAPON_MAGIC,
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
	unsigned short clientid;
	bool groundtile;
	bool blocking;
	bool alwaysOnTop;
	bool container;
	bool stackable;
	bool useable;
	bool notMoveable;
	bool pickupable;
	bool fluid;
	bool rotable;
	bool isammo;

	unsigned short rotateTo;
	
	//xml data
	char name[128];
	char descr[128];
	double weight;
	unsigned short	decayTo;
	unsigned short	decayTime;
	bool blockingProjectile;
	bool floorchange;
	slots_t slot_position;
	bool floorChangeNorth;
	bool floorChangeSouth;
	bool floorChangeEast;
	bool floorChangeWest;

	//ground
	int speed;

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
	bool write1time;
	
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
