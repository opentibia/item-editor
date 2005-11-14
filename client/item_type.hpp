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
#include "item_loader.hpp"
#include <map>
#include <list>

enum itemgroup_t{
	ITEM_GROUP_NONE = 0,
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

/////////OTB specific//////////////
enum rootattrib_t{
	ROOT_ATTR_VERSION = 0x01
};

enum itemattrib_t{
	ITEM_ATTR_FIRST = 0x10,
	ITEM_ATTR_SERVERID = ITEM_ATTR_FIRST,
	ITEM_ATTR_CLIENTID,
	ITEM_ATTR_NAME,
	ITEM_ATTR_DESCR,
	ITEM_ATTR_SPEED,
	ITEM_ATTR_SLOT,
	ITEM_ATTR_MAXITEMS,
	ITEM_ATTR_WEIGHT,
	ITEM_ATTR_WEAPON,
	ITEM_ATTR_AMU,
	ITEM_ATTR_ARMOR,
	ITEM_ATTR_MAGLEVEL,
	ITEM_ATTR_MAGFIELDTYPE,
	ITEM_ATTR_WRITEABLE,
	ITEM_ATTR_ROTATETO,
	ITEM_ATTR_DECAY,
	ITEM_ATTR_SPRITEHASH,
	ITEM_ATTR_MINIMAPCOLOR,
	ITEM_ATTR_07,
	ITEM_ATTR_08,
	ITEM_ATTR_LIGHT,

	//1-byte aligned
	ITEM_ATTR_DECAY2,
	ITEM_ATTR_WEAPON2,
	ITEM_ATTR_AMU2,
	ITEM_ATTR_ARMOR2,
	ITEM_ATTR_WRITEABLE2,
	ITEM_ATTR_LIGHT2,

	ITEM_ATTR_LAST
};

enum itemflags_t {
 FLAG_BLOCK_SOLID = 1,
 FLAG_BLOCK_PROJECTILE = 2, 
 FLAG_BLOCK_PATHFIND = 4, 
 FLAG_HAS_HEIGHT = 8,
 FLAG_USEABLE = 16,
 FLAG_PICKUPABLE = 32,
 FLAG_MOVEABLE = 64,
 FLAG_STACKABLE = 128,
 FLAG_FLOORCHANGEDOWN = 256,
 FLAG_FLOORCHANGENORTH = 512,
 FLAG_FLOORCHANGEEAST = 1024,
 FLAG_FLOORCHANGESOUTH = 2048,
 FLAG_FLOORCHANGEWEST = 4096,
 FLAG_ALWAYSONTOP = 8192,
 FLAG_READABLE = 16384,
 FLAG_ROTABLE = 32768,
 FLAG_HANGABLE = 65536,
 FLAG_VERTICAL = 131072,
 FLAG_HORIZONTAL = 262144,
 FLAG_CANNOTDECAY = 524288
};

struct decayBlock{
	unsigned short decayTo;
	unsigned short decayTime;
};

struct weaponBlock {
	unsigned char weaponType;
	unsigned char amuType;
	unsigned char shootType;
	unsigned char attack;
	unsigned char defence;
};

struct amuBlock {
	unsigned char amuType;
	unsigned char shootType;
	unsigned char attack;
};

struct armorBlock {
	unsigned short armor;
	double weight;
	unsigned short slot_position;
};

struct writeableBlock {
	unsigned short readOnlyId;
};

struct lightBlock {
	unsigned short lightLevel;
	unsigned short lightColor;
};


//1-byte aligned structs
#pragma pack(1)

struct VERSIONINFO{
	unsigned long dwMajorVersion;
	unsigned long dwMinorVersion;
	unsigned long dwBuildNumber;
	char CSDVersion[128];
};

struct decayBlock2{
	unsigned short decayTo;
	unsigned short decayTime;
};

struct weaponBlock2 {
	unsigned char weaponType;
	unsigned char amuType;
	unsigned char shootType;
	unsigned char attack;
	unsigned char defence;
};

struct amuBlock2 {
	unsigned char amuType;
	unsigned char shootType;
	unsigned char attack;
};

struct armorBlock2 {
	unsigned short armor;
	double weight;
	unsigned short slot_position;
};

struct writeableBlock2 {
	unsigned short readOnlyId;
};

struct lightBlock2 {
	unsigned short lightLevel;
	unsigned short lightColor;
};

#pragma pack()
//////////////////////////

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

class SpriteType;

class ItemType
{
public:
	ItemType();
	ItemType::ItemType(unsigned short id, const SpriteType *stype);
	~ItemType(){}

	static long minClientId;
	static long maxClientId;
	static long minServerId;
	static long maxServerId;

	static long dwMajorVersion;
	static long dwMinorVersion;
	static long dwBuildNumber;
	
	itemgroup_t group;
	int id;
	int clientid;
	bool foundNewImage;

	bool blockSolid;
	bool hasHeight;
	bool blockProjectile;
	bool blockPathFind;

	bool alwaysOnTop;
	bool stackable;
	bool useable;
	bool moveable;
	bool pickupable;
	bool rotable;
	bool readable;

	bool canNotDecay;

	int rotateTo;
	
	char name[128];
	char descr[128];
	char sprHash[16];
	double weight;
	int decayTo;
	int decayTime;
	slots_t slot_position;
	bool floorChangeDown;
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
	
	//magicfield
	int magicfieldtype;
	
	//writeable
	int readOnlyId;
	
	unsigned short miniMapColor;
	unsigned short subParam07;
	unsigned short subParam08;

	int lightLevel;
	int lightColor;

	bool isVertical;
	bool isHorizontal;

	bool isHangable;

	//
	//bool canWalkThrough;
	//bool ladderUp;
	//bool canSeeThrough;
	//bool wallObject;
	//bool hasAnimation;
	//bool hasParameter14;
	//unsigned short heightdisplacement;
	//bool makeLight;

	bool compareOptions(const SpriteType *stype);
};

typedef std::map<unsigned short, ItemType*> ItemMap;

class ItemsTypes {
public:
	ItemsTypes();
	~ItemsTypes();
	void clear();

	bool loadFromDat(const char *filename);
	bool loadFromXml(const char *filename);

	bool importFromXml(const char *filename);
	bool exportToXml(const char *filename);

	int saveOtb(const char *filename);
	int loadOtb(const char *filename);

	ItemMap::iterator getTypes() {return item.begin();};
	ItemMap::iterator getEnd() {return item.end();};

	bool setGroup(int id, itemgroup_t newgroup);
	itemgroup_t getGroup(int id);

	ItemType* getType(int id);
	ItemType* getTypeBySrpite(int sprite);
	bool addType(int id, ItemType* type);
	
private:
	bool datLoaded;
	static int loadstatus;
	static void XMLCALL xmlstartNode(void *userData, const char *name, const char **atts);
	static void XMLCALL xmlendNode(void *userData, const char *name);
	static void XMLCALL xmlstartNodeImport(void *userData, const char *name, const char **atts);
	
	ItemMap item;
};

#endif 
