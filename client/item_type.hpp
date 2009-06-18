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

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
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
	ITEM_GROUP_CHARGES,
	ITEM_GROUP_TELEPORT,
	ITEM_GROUP_MAGICFIELD,
	ITEM_GROUP_WRITEABLE,
	ITEM_GROUP_KEY,
	ITEM_GROUP_SPLASH,
	ITEM_GROUP_FLUID,
	ITEM_GROUP_DOOR,
	ITEM_GROUP_DEPRECATED,
	ITEM_GROUP_LAST
};

/////////OTB specific//////////////

enum clientVersion_t{
	CLIENT_VERSION_750 = 1,
	CLIENT_VERSION_755 = 2,
	CLIENT_VERSION_760 = 3,
	CLIENT_VERSION_780 = 4,
	CLIENT_VERSION_790 = 5,
	CLIENT_VERSION_792 = 6,
	CLIENT_VERSION_800 = 7,
	CLIENT_VERSION_810 = 8,
	CLIENT_VERSION_811 = 9,
	CLIENT_VERSION_820 = 10,
	CLIENT_VERSION_830 = 11,
	CLIENT_VERSION_840 = 12,
	CLIENT_VERSION_841 = 13,
	CLIENT_VERSION_842 = 14,
};

enum rootattrib_t{
	ROOT_ATTR_VERSION = 0x01
};

enum itemattrib_t{
	ITEM_ATTR_FIRST = 0x10,
	ITEM_ATTR_SERVERID = ITEM_ATTR_FIRST,
	ITEM_ATTR_CLIENTID,
	ITEM_ATTR_NAME,				/*deprecated*/
	ITEM_ATTR_DESCR,			/*deprecated*/
	ITEM_ATTR_SPEED,
	ITEM_ATTR_SLOT,				/*deprecated*/
	ITEM_ATTR_MAXITEMS,			/*deprecated*/
	ITEM_ATTR_WEIGHT,			/*deprecated*/
	ITEM_ATTR_WEAPON,			/*deprecated*/
	ITEM_ATTR_AMU,				/*deprecated*/
	ITEM_ATTR_ARMOR,			/*deprecated*/
	ITEM_ATTR_MAGLEVEL,			/*deprecated*/
	ITEM_ATTR_MAGFIELDTYPE,		/*deprecated*/
	ITEM_ATTR_WRITEABLE,		/*deprecated*/
	ITEM_ATTR_ROTATETO,			/*deprecated*/
	ITEM_ATTR_DECAY,			/*deprecated*/
	ITEM_ATTR_SPRITEHASH,
	ITEM_ATTR_MINIMAPCOLOR,
	ITEM_ATTR_07,
	ITEM_ATTR_08,
	ITEM_ATTR_LIGHT,

	//1-byte aligned
	ITEM_ATTR_DECAY2,			/*deprecated*/
	ITEM_ATTR_WEAPON2,			/*deprecated*/
	ITEM_ATTR_AMU2,				/*deprecated*/
	ITEM_ATTR_ARMOR2,			/*deprecated*/
	ITEM_ATTR_WRITEABLE2,		/*deprecated*/
	ITEM_ATTR_LIGHT2,
	ITEM_ATTR_TOPORDER,
	ITEM_ATTR_WRITEABLE3,		/*deprecated*/

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
 FLAG_CANNOTDECAY = 524288,
 FLAG_ALLOWDISTREAD = 1048576,
 FLAG_CORPSE = 2097152,			/*deprecated*/
 FLAG_CLIENTCHARGES = 4194304,
 FLAG_LOOKTHROUGH = 8388608
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

struct weaponBlock2{
	unsigned char weaponType;
	unsigned char amuType;
	unsigned char shootType;
	unsigned char attack;
	unsigned char defence;
};

struct amuBlock2{
	unsigned char amuType;
	unsigned char shootType;
	unsigned char attack;
};

struct armorBlock2{
	unsigned short armor;
	double weight;
	unsigned short slot_position;
};

struct writeableBlock2{
	unsigned short readOnlyId;
};

struct writeableBlock3{
	unsigned short readOnlyId;
	unsigned short maxTextLen;
};

struct lightBlock2{
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
	WEAPON_WAND,
	WEAPON_AMMO
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
	DIST_POISONFIELD   = 15,
	DIST_INFERNALBOLT   = 16
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
	ItemType(unsigned short id, const SpriteType *stype);
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
	unsigned char alwaysOnTopOrder;
	bool stackable;
	bool useable;
	bool moveable;
	bool pickupable;
	bool rotable;
	bool readable;
	bool allowDistRead;
	bool clientCharges;
	bool lookThrough;
	int groundSpeed;
	std::string name;
	char sprHash[16];

	unsigned short miniMapColor;
	unsigned short subParam07;
	unsigned short subParam08;

	int lightLevel;
	int lightColor;

	bool isVertical;
	bool isHorizontal;

	bool isHangable;
	bool compareOptions(const SpriteType *stype);
	void reloadOptions(const SpriteType *stype);
};

typedef std::map<unsigned short, ItemType*> ItemMap;

class ItemsTypes {
public:
	ItemsTypes();
	~ItemsTypes();
	void clear();

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
	
	ItemMap item;
};

#endif 
