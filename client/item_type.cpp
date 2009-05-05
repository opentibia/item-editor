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

extern ItemsTypes *g_itemsTypes;
extern ItemsSprites *g_itemsSprites;
extern void getImageHash(unsigned short cid, void* output);

const char* readXmlProp(const char* propName, const xmlProp *props);


long ItemType::minClientId = 0;
long ItemType::maxClientId = 0;
long ItemType::minServerId = 0;
long ItemType::maxServerId = 0;
long ItemType::dwMajorVersion = 0;
long ItemType::dwMinorVersion = 0;
long ItemType::dwBuildNumber = 0;

ItemType::ItemType()
{
	name = "";
	group     = ITEM_GROUP_NONE;
	id		  = 100;
	clientid  = 100;
	foundNewImage = false;

	blockSolid	= false;
	blockProjectile = false;
	hasHeight = false;
	blockPathFind = false;

	alwaysOnTop	= false;
	alwaysOnTopOrder = 0;
	stackable	= false;
	useable		= false;
	moveable	= true;
	pickupable	= false;
	rotable = false;
	readable = false;
	allowDistRead = false;
	clientCharges = false;
	
	//rotateTo = 0;

	isVertical = false;
	isHorizontal = false;

	isHangable = false;

	miniMapColor = 0;
	subParam07 = 0;
	subParam08 = 0;

	memset(sprHash, 0, sizeof(sprHash));

	//weight = 0.00;
	//decayTo = 0;
	//decayTime = 0;
	//slot_position = SLOT_HAND;

	lightLevel = 0;
	lightColor = 0;

	//ground
	speed = 0;

	//container
	//maxItems = 8;
	
	//weapon
	//amuType = AMU_NONE;
	//weaponType = WEAPON_NONE;
	//shootType = DIST_NONE;
	//attack = 0;
	//defence = 0;
	
	//armor
	//armor = 0;
	
	//rune
	//runeMagLevel = -1;
	
	//writeable
	//readOnlyId = 0;
	//maxTextLen = 0;
}

ItemType::ItemType(unsigned short _id, const SpriteType *stype)
{
	group = stype->group;
	id = _id;
	clientid = stype->id;
	foundNewImage = false;

	blockSolid = stype->blockSolid;
	hasHeight = stype->hasHeight;
	blockProjectile = stype->blockProjectile;
	blockPathFind = stype->blockPathFind;

	alwaysOnTop = stype->alwaysOnTop;
	alwaysOnTopOrder = stype->alwaysOnTopOrder;
	stackable = stype->stackable ;
	useable = stype->useable;
	moveable = stype->moveable;
	pickupable = stype->pickupable;
	rotable = stype->rotable;
	readable = stype->readable;
	clientCharges = stype->clientCharges;
	speed = stype->speed;

	allowDistRead = false;

	miniMapColor = stype->miniMapColor;
	subParam07 = stype->subParam07;
	subParam08 = stype->subParam08;

	lightLevel = stype->lightLevel;
	lightColor = stype->lightColor;

	isVertical = stype->isVertical;
	isHorizontal = stype->isHorizontal;

	isHangable = stype->isHangable;

	//#ifdef __SPRITE_SEARCH__
	memcpy(sprHash, stype->sprHash, 16);
	/*
	#else
	getImageHash(stype->id, sprHash);
	#endif
	*/

	/*
	rotateTo = 0;
	weight = 0.00;
	decayTo = 0;
	decayTime = 0;
	slot_position = SLOT_HAND;

	floorChangeDown = false;
	floorChangeNorth = false;
	floorChangeSouth = false;
	floorChangeEast = false;
	floorChangeWest = false;

	//container
	maxItems = 8;
	
	//weapon
	amuType = AMU_NONE;
	weaponType = WEAPON_NONE;
	shootType = DIST_NONE;
	attack = 0;
	defence = 0;
	
	//armor
	armor = 0;
	
	//rune
	runeMagLevel = -1;
	
	
	//writeable
	readOnlyId = 0;
	maxTextLen = 0;
	*/
}

bool ItemType::compareOptions(const SpriteType *stype)
{
	if(!stype)
		return false;

	switch(stype->group){
	case ITEM_GROUP_GROUND:
	case ITEM_GROUP_CONTAINER:
	case ITEM_GROUP_SPLASH:
	case ITEM_GROUP_FLUID:
	//case ITEM_GROUP_WRITEABLE:
	case ITEM_GROUP_CHARGES:
		if(group != stype->group)
			return false;
	}
	
	if(blockSolid != stype->blockSolid)
		return false;
	
	if(blockPathFind != stype->blockPathFind)
		return false;

	if(alwaysOnTop != stype->alwaysOnTop){
		return false;
	}

	if(alwaysOnTopOrder != stype->alwaysOnTopOrder)
		return false;

	if(stackable != stype->stackable)
		return false;

	if(useable != stype->useable)
		return false;

	if(moveable != stype->moveable)
		return false;

	if(pickupable != stype->pickupable)
		return false;

	if(rotable != stype->rotable)
		return false;

	if(speed != stype->speed)
		return false;
/*
	if(miniMapColor != stype->miniMapColor)
		return false;
	
	if(subParam07 != stype->subParam07)
		return false;

	if(subParam08 != stype->subParam08)
		return false;
*/
	if(lightLevel != stype->lightLevel)
		return false;

	if(lightColor != stype->lightColor)
		return false;

	if(isVertical != stype->isVertical)
		return false;

	if(isHorizontal != stype->isHorizontal)
		return false;

	if(isHangable != stype->isHangable)
		return false;

	if(readable != stype->readable)
		return false;

	if(clientCharges != stype->clientCharges)
		return false;

	return true;
}

void ItemType::reloadOptions(const SpriteType *stype)
{
	if(!stype)
		return;

	group = stype->group;
	clientid = stype->id;
	foundNewImage = false;

	blockSolid = stype->blockSolid;
	hasHeight = stype->hasHeight;
	blockProjectile = stype->blockProjectile;
	blockPathFind = stype->blockPathFind;

	alwaysOnTop = stype->alwaysOnTop;
	alwaysOnTopOrder = stype->alwaysOnTopOrder;
	stackable = stype->stackable ;
	useable = stype->useable;
	moveable = stype->moveable;
	pickupable = stype->pickupable;
	rotable = stype->rotable;
	readable = stype->readable;
	clientCharges = stype->clientCharges;
	speed = stype->speed;

	allowDistRead = false;

	miniMapColor = stype->miniMapColor;
	subParam07 = stype->subParam07;
	subParam08 = stype->subParam08;

	lightLevel = stype->lightLevel;
	lightColor = stype->lightColor;

	isVertical = stype->isVertical;
	isHorizontal = stype->isHorizontal;

	isHangable = stype->isHangable;

	//#ifdef __SPRITE_SEARCH__
	memcpy(sprHash, stype->sprHash, 16);
	/*
	#else
	getImageHash(stype->id, sprHash);
	#endif
	*/
}



ItemsTypes::ItemsTypes()
{
	datLoaded = false;
}

ItemsTypes::~ItemsTypes()
{
	clear();
}

void ItemsTypes::clear()
{
	ItemMap::iterator it;
	for(it = item.begin(); it != item.end(); ++it) {
		delete it->second;
	}

	item.clear();
}

void saveAttribute(FILE* f, const std::string& key, const std::string& value)
{
	fprintf(f, "\t<attribute key=\"%s\" value=\"%s\"/>\n", key.c_str(), value.c_str());
}

void saveAttribute(FILE* f, const std::string& key, int value)
{
	fprintf(f, "\t<attribute key=\"%s\" value=\"%d\"/>\n", key.c_str(), value);
}

bool ItemsTypes::exportToXml(const char *filename)
{
	FILE *f;
	f = fopen(filename, "w");
	if(!f)
		return false;

	fprintf(f, "<?xml version=\"1.0\"?>\n<items>\n");

	for(ItemMap::iterator it = item.begin(); it != item.end(); it++){
		fprintf(f, "<item id=\"%d\" name=\"%s\">\n", it->second->id, it->second->name.c_str());

		if(it->second->group == ITEM_GROUP_KEY){
			saveAttribute(f, "type", "key");
		}
		else if(it->second->group == ITEM_GROUP_MAGICFIELD){
			saveAttribute(f, "type", "magicfield");
		}

		/*
		if(it->second->descr.size() > 0){
			saveAttribute(f, "description", it->second->descr);
		}

		if(it->second->pickupable){
			saveAttribute(f, "weight", int(it->second->weight * 100));
		}

		if(it->second->armor != 0){
			saveAttribute(f, "armor", it->second->armor);
		}

		if(it->second->defence != 0){
			saveAttribute(f, "defense", it->second->defence);
		}

		if(it->second->attack != 0){
			saveAttribute(f, "attack", it->second->attack);
		}

		if(it->second->rotateTo != 0){
			saveAttribute(f, "rotateTo", it->second->rotateTo);
		}

		if(it->second->group == ITEM_GROUP_CONTAINER){
			saveAttribute(f, "containerSize", it->second->maxItems);
		}

		if(it->second->group == ITEM_GROUP_WRITEABLE){
			saveAttribute(f, "maxTextLen", it->second->maxTextLen);

			if(it->second->readOnlyId != 0){
				saveAttribute(f, "writeOnceItemId", it->second->readOnlyId);
			}
		}

		if(it->second->group == ITEM_GROUP_AMMUNITION){
			saveAttribute(f, "weaponType", "ammunition");
		}
		if(it->second->weaponType != WEAPON_NONE){
			switch(it->second->weaponType){
				case WEAPON_AXE: saveAttribute(f, "weaponType", "axe"); break;
				case WEAPON_CLUB: saveAttribute(f, "weaponType", "club"); break;
				case WEAPON_SWORD: saveAttribute(f, "weaponType", "sword"); break;
				case WEAPON_DIST: saveAttribute(f, "weaponType", "distance"); break;
				case WEAPON_WAND: saveAttribute(f, "weaponType", "wand"); break;
				case WEAPON_SHIELD: saveAttribute(f, "weaponType", "shield"); break;
			}
		}

		if(it->second->slot_position != SLOT_HAND){
			switch(it->second->slot_position){
				case SLOT_HEAD: saveAttribute(f, "slotType", "head"); break;
				case SLOT_BODY: saveAttribute(f, "slotType", "body"); break;
				case SLOT_LEGS: saveAttribute(f, "slotType", "legs"); break;
				case SLOT_FEET: saveAttribute(f, "slotType", "feet"); break;
				case SLOT_BACKPACK: saveAttribute(f, "slotType", "backpack"); break;
				case SLOT_2HAND: saveAttribute(f, "slotType", "two-handed"); break;
				case SLOT_AMULET: saveAttribute(f, "slotType", "necklace"); break;
				case SLOT_RING: saveAttribute(f, "slotType", "ring"); break;
			}
		}

		if(it->second->amuType != AMU_NONE){
			if(it->second->weaponType == WEAPON_NONE){
				saveAttribute(f, "weaponType", "ammunition");
			}

			switch(it->second->amuType){
				case AMU_ARROW: saveAttribute(f, "ammoType", "arrow"); break;
				case AMU_BOLT: saveAttribute(f, "ammoType", "bolt"); break;
			}
		}

		if(it->second->shootType != DIST_NONE){
			switch(it->second->shootType){
				case DIST_SPEAR: saveAttribute(f, "shootType", "spear"); break;
				case DIST_BOLT: saveAttribute(f, "shootType", "bolt"); break;
				case DIST_ARROW: saveAttribute(f, "shootType", "arrow"); break;
				case DIST_FIRE: saveAttribute(f, "shootType", "fire"); break;
				case DIST_ENERGY: saveAttribute(f, "shootType", "energy"); break;
				case DIST_POISONARROW: saveAttribute(f, "shootType", "poisonarrow"); break;
				case DIST_BURSTARROW: saveAttribute(f, "shootType", "burstarrow"); break;
				case DIST_THROWINGSTAR: saveAttribute(f, "shootType", "throwingstar"); break;
				case DIST_THROWINGKNIFE: saveAttribute(f, "shootType", "throwingknife"); break;
				case DIST_SMALLSTONE: saveAttribute(f, "shootType", "smallstone"); break;
				case DIST_SUDDENDEATH: saveAttribute(f, "shootType", "suddendeath"); break;
				case DIST_LARGEROCK: saveAttribute(f, "shootType", "largerock"); break;
				case DIST_SNOWBALL: saveAttribute(f, "shootType", "snowball"); break;
				case DIST_POWERBOLT: saveAttribute(f, "shootType", "powerbolt"); break;
				case DIST_POISONFIELD: saveAttribute(f, "shootType", "poison"); break;
				case DIST_INFERNALBOLT: saveAttribute(f, "shootType", "infernalbolt"); break;
			}
		}			
		if(it->second->decayTime > 0){
			saveAttribute(f, "decayTo", it->second->decayTo);
			saveAttribute(f, "duration", it->second->decayTime);
		}
		*/

		//stopduration
		//transformEquipTo
		//transformDeEquipTo
		//showduration
		//charges
		//showcharges
		//invisible
		//speed
		//healthGain
		//healthTicks
		//manaGain
		//manaTicks
		//manaShield
		//skillSword
		//skillAxe
		//skillClub
		//skillDist
		//skillFish
		//skillShield
		//skillFist
		//absorbPercentAll
		//absorbPercentEnergy
		//absorbPercentFire
		//absorbPercentPoison
		//absorbPercentLifeDrain
		//absorbPercentManaDrain
		//absorbPercentPhysical
		//suppressDrunk
		//field

		fprintf(f, "</item>\n");
	}

	fprintf(f, "</items>\n");
	fclose(f);
	return true;
}


bool ItemsTypes::setGroup(int id, itemgroup_t newgroup)
{
	ItemType* it = getType(id);
	if(it) {
		it->group = newgroup;
		return true;
	}

	return false;
}

itemgroup_t ItemsTypes::getGroup(int id)
{
	ItemMap::iterator it = item.find(id);

	if ((it != item.end()) && (it->second != NULL))
		return it->second->group;

	return ITEM_GROUP_NONE;
}

bool ItemsTypes::addType(int id, ItemType* type)
{
	if(!getType(id)) {
		item[id] = type;

		if(id < 20000){
			if(type->id > ItemType::maxServerId || ItemType::maxServerId == 0)
				ItemType::maxServerId = type->id;

			if(type->id != 0 && (type->id < ItemType::minServerId || ItemType::minServerId == 0))
				ItemType::minServerId = type->id;
		}
		return true;
	}

	return false;
}

ItemType* ItemsTypes::getType(int id)
{
	ItemMap::iterator it = item.find(id);
	if ((it != item.end()) && (it->second != NULL))
		return it->second;
	   
	return NULL;
}


ItemType* ItemsTypes::getTypeBySrpite(int sprite)
{
	ItemMap::iterator it;
	for(it = getTypes(); it != getEnd(); it++){
		if(it->second->clientid == sprite){
			return it->second;
		}
	}
	   
	return NULL;
}


int ItemsTypes::loadOtb(const char *filename)
{
	ItemLoader f;
	if(!f.openFile(filename, false, true)) {
		return f.getError();
	}
	
	unsigned long type,len;
	const unsigned char* data;

	NODE node = f.getChildNode((const NODE)NULL, type);
	data = f.getProps(node, len);
	//4 byte flags
	//attributes (optional)
	//0x01 = version data
	if(len > 4){
		const unsigned char* p = &data[4];
		//attribute
		attribute_t attrib = *p; p+= sizeof(attribute_t);
		if(p >= data + len) {
			return ERROR_INVALID_FORMAT;
		}

		datasize_t datalen = 0;
		//size of data
		memcpy(&datalen, p, sizeof(datasize_t)); p+= sizeof(datalen);
		if(p >= data + len) {
			return ERROR_INVALID_FORMAT;
		}
	
		VERSIONINFO vi;
		memcpy(&vi, p, sizeof(VERSIONINFO)); p+= sizeof(VERSIONINFO);
		ItemType::dwMajorVersion = vi.dwMajorVersion;
		ItemType::dwMinorVersion = vi.dwMinorVersion;
		ItemType::dwBuildNumber = vi.dwBuildNumber;
	}
	
	node = f.getChildNode(node, type);

	while(node != NO_NODE) {
		data = f.getProps(node, len);
		if(data == NULL && f.getError() != ERROR_NONE)
			return f.getError();
		
		flags_t flags;
		if(data != NULL) {
			const unsigned char* p = &data[0];
			ItemType* sType = new ItemType();
			bool loadedFlags = false;

			while(p < data + len) {
				sType->group = (itemgroup_t)type;

				switch(type) {
					case ITEM_GROUP_NONE:
					case ITEM_GROUP_GROUND:
					case ITEM_GROUP_CONTAINER:
					case ITEM_GROUP_WEAPON:
					case ITEM_GROUP_AMMUNITION:
					case ITEM_GROUP_ARMOR:
					case ITEM_GROUP_CHARGES:
					case ITEM_GROUP_TELEPORT:
					case ITEM_GROUP_MAGICFIELD:
					case ITEM_GROUP_WRITEABLE:
					case ITEM_GROUP_KEY:
					case ITEM_GROUP_SPLASH:
					case ITEM_GROUP_FLUID:
					case ITEM_GROUP_DOOR:
					case ITEM_GROUP_DEPRECATED:
					{
						if(ItemType::dwMajorVersion == 2){
							if(type == ITEM_GROUP_CHARGES){
								sType->clientCharges = true;
							}
						}

						if(!loadedFlags) {
							//read 4 byte flags
							memcpy((void*)&flags, p, sizeof(flags_t)); p+= sizeof(flags_t);

							sType->blockSolid = ((flags & FLAG_BLOCK_SOLID) == FLAG_BLOCK_SOLID);
							sType->blockProjectile = ((flags & FLAG_BLOCK_PROJECTILE) == FLAG_BLOCK_PROJECTILE);
							sType->blockPathFind = ((flags & FLAG_BLOCK_PATHFIND) == FLAG_BLOCK_PATHFIND);
							sType->hasHeight = ((flags & FLAG_HAS_HEIGHT) == FLAG_HAS_HEIGHT);
							sType->useable = ((flags & FLAG_USEABLE) == FLAG_USEABLE);
							sType->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
							sType->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
							sType->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
							/*
							sType->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
							sType->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
							sType->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
							sType->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
							sType->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
							*/
							sType->alwaysOnTop = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
							sType->readable = ((flags & FLAG_READABLE) == FLAG_READABLE);
							sType->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
							sType->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
							sType->isVertical = ((flags & FLAG_VERTICAL) == FLAG_VERTICAL);
							sType->isHorizontal = ((flags & FLAG_HORIZONTAL) == FLAG_HORIZONTAL);
							sType->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
							sType->clientCharges = ((flags & FLAG_CLIENTCHARGES) == FLAG_CLIENTCHARGES);

							if(p >= data + len) //no attributes
								break;
							loadedFlags = true;
						}

						//attribute
						attribute_t attrib = *p; p+= sizeof(attribute_t);
						if(p >= data + len) {
							delete sType;
							return ERROR_INVALID_FORMAT;
						}

						datasize_t datalen = 0;
						//size of data
						memcpy(&datalen, p, sizeof(datasize_t)); p+= sizeof(datalen);
						if(p >= data + len) {
							delete sType;
							return ERROR_INVALID_FORMAT;
						}

						switch(attrib) {
							case ITEM_ATTR_SERVERID:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->id, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_CLIENTID:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->clientid, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_NAME:
							{
								if(datalen >= sizeof(sType->name))
									return ERROR_INVALID_FORMAT;

								sType->name = std::string((char*)p, datalen);
								break;
							}
							case ITEM_ATTR_DESCR:
							{
								/*
								if(datalen >= sizeof(sType->descr))
									return ERROR_INVALID_FORMAT;

								sType->desc = std::string((char*)p, datalen);
								*/
								break;
							}
							case ITEM_ATTR_SPEED:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->speed, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_SLOT:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								//memcpy(&sType->slot_position, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_MAXITEMS:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								//memcpy(&sType->maxItems, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_WEIGHT:
							{
								if(datalen != sizeof(double))
									return ERROR_INVALID_FORMAT;

								//memcpy(&sType->weight, p, sizeof(double));
								break;
							}
							case ITEM_ATTR_WEAPON:
							{
								if(datalen != sizeof(weaponBlock))
									return ERROR_INVALID_FORMAT;

								weaponBlock wb;
								memcpy(&wb, p, sizeof(weaponBlock));
								/*
								sType->weaponType = (WeaponType)wb.weaponType;
								sType->shootType = (subfight_t)wb.shootType;
								sType->amuType = (amu_t)wb.amuType;
								sType->attack = wb.attack;
								sType->defence = wb.defence;
								*/
								break;
							}
							case ITEM_ATTR_AMU:
							{
								if(datalen != sizeof(amuBlock))
									return ERROR_INVALID_FORMAT;

								amuBlock ab;
								memcpy(&ab, p, sizeof(amuBlock));
								/*
								sType->shootType = (subfight_t)ab.shootType;
								sType->amuType = (amu_t)ab.amuType;
								sType->attack = ab.attack;
								*/
								break;
							}
							case ITEM_ATTR_ARMOR:
							{
								if(datalen != sizeof(armorBlock))
									return ERROR_INVALID_FORMAT;

								armorBlock ab;
								memcpy(&ab, p, sizeof(armorBlock));
								/*
								sType->armor = ab.armor;
								sType->weight = ab.weight;
								sType->slot_position = (slots_t)ab.slot_position;
								*/

								break;
							}
							case ITEM_ATTR_MAGLEVEL:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;
								
								//memcpy(&sType->runeMagLevel, p, sizeof(unsigned short));
								
								break;
							}
							case ITEM_ATTR_MAGFIELDTYPE:
							{
								if(datalen != sizeof(unsigned char))
									return ERROR_INVALID_FORMAT;
								
								break;
							}

							case ITEM_ATTR_WRITEABLE:
							{
								if(datalen != sizeof(writeableBlock))
									return ERROR_INVALID_FORMAT;

								struct writeableBlock wb;
								memcpy(&wb, p, sizeof(writeableBlock));

								//sType->readOnlyId = wb.readOnlyId;

								break;
							}
							case ITEM_ATTR_ROTATETO:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								//memcpy(&sType->rotateTo, p, sizeof(unsigned short));

								break;
							}
							case ITEM_ATTR_DECAY:
							{
								if(datalen != sizeof(decayBlock))
									return ERROR_INVALID_FORMAT;

								decayBlock db;
								memcpy(&db, p, sizeof(decayBlock));
								/*
								sType->decayTime = db.decayTime;
								sType->decayTo = db.decayTo;
								*/
								break;
							}

							case ITEM_ATTR_SPRITEHASH:
							{
								if(datalen != sizeof(sType->sprHash))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->sprHash, p, sizeof(sType->sprHash));
								
								break;
							}

							case ITEM_ATTR_MINIMAPCOLOR:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->miniMapColor, p, sizeof(unsigned short));
								break;
							}

							case ITEM_ATTR_07:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->subParam07, p, sizeof(unsigned short));

								break;
							}

							case ITEM_ATTR_08:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->subParam08, p, sizeof(unsigned short));

								break;
							}

							case ITEM_ATTR_LIGHT:
							{
								if(datalen != sizeof(lightBlock))
									return ERROR_INVALID_FORMAT;

								lightBlock lb;
								memcpy(&lb, p, sizeof(lightBlock));
								sType->lightLevel = lb.lightLevel;
								sType->lightColor = lb.lightColor;
								break;
							}

							case ITEM_ATTR_DECAY2:
							{
								if(datalen != sizeof(decayBlock2))
									return ERROR_INVALID_FORMAT;

								decayBlock2 db2;
								memcpy(&db2, p, sizeof(decayBlock2));
								/*
								sType->decayTime = db2.decayTime;
								sType->decayTo = db2.decayTo;
								*/
								break;
							}

							case ITEM_ATTR_WEAPON2:
							{
								if(datalen != sizeof(weaponBlock2))
									return ERROR_INVALID_FORMAT;

								weaponBlock2 wb2;
								memcpy(&wb2, p, sizeof(weaponBlock2));
								/*
								sType->weaponType = (WeaponType)wb2.weaponType;
								sType->shootType = (subfight_t)wb2.shootType;
								sType->amuType = (amu_t)wb2.amuType;
								sType->attack = wb2.attack;
								sType->defence = wb2.defence;
								*/
								break;
							}

							case ITEM_ATTR_AMU2:
							{
								if(datalen != sizeof(amuBlock2))
									return ERROR_INVALID_FORMAT;

								amuBlock2 ab2;
								memcpy(&ab2, p, sizeof(amuBlock2));
								/*
								sType->shootType = (subfight_t)ab2.shootType;
								sType->amuType = (amu_t)ab2.amuType;
								sType->attack = ab2.attack;
								*/
								break;
							}

							case ITEM_ATTR_ARMOR2:
							{
								if(datalen != sizeof(armorBlock2))
									return ERROR_INVALID_FORMAT;

								armorBlock2 ab2;
								memcpy(&ab2, p, sizeof(armorBlock2));
								/*
								sType->armor = ab2.armor;
								sType->weight = ab2.weight;
								sType->slot_position = (slots_t)ab2.slot_position;
								*/

								break;
							}

							case ITEM_ATTR_WRITEABLE2:
							{
								if(datalen != sizeof(writeableBlock2))
									return ERROR_INVALID_FORMAT;

								struct writeableBlock2 wb2;
								memcpy(&wb2, p, sizeof(writeableBlock2));
								//sType->readOnlyId = wb2.readOnlyId;
								break;
							}

							case ITEM_ATTR_WRITEABLE3:
							{
								if(datalen != sizeof(writeableBlock3))
									return ERROR_INVALID_FORMAT;

								struct writeableBlock3 wb3;
								memcpy(&wb3, p, sizeof(writeableBlock3));
								//sType->readOnlyId = wb3.readOnlyId;
								//sType->maxTextLen = wb3.maxTextLen;

								break;
							}

							case ITEM_ATTR_LIGHT2:
							{
								if(datalen != sizeof(lightBlock2))
									return ERROR_INVALID_FORMAT;

								lightBlock2 lb2;
								memcpy(&lb2, p, sizeof(lightBlock2));
								sType->lightLevel = lb2.lightLevel;
								sType->lightColor = lb2.lightColor;
								break;
							}
							
							case ITEM_ATTR_TOPORDER:
							{
								if(datalen != sizeof(unsigned char))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->alwaysOnTopOrder, p, sizeof(unsigned char));
								break;
							}

							default:
								delete sType;
								return ERROR_INVALID_FORMAT;
						}
						
						p+= datalen;
						break;
					}

					default:
						return ERROR_INVALID_FORMAT;
						break;
				}
			}

			addType(sType->id, sType);
		}

		node = f.getNextNode(node, type);
	}
	
	return ERROR_NONE;
}

int ItemsTypes::saveOtb(const char *filename)
{
	ItemLoader *f = new ItemLoader();
	if(!f->openFile(filename, true)) {
		return f->getError();
	}

	f->startNode(0);
	f->setFlags(0);
	
	VERSIONINFO vi;
	memset(&vi, '\0', sizeof(VERSIONINFO));

	//vi.dwMajorVersion = 2; //version
	vi.dwMajorVersion = 3; //version
	char clientString[32];
	const graphicsVersion* spritesVersion;
	if(spritesVersion = g_itemsSprites->getVersion()){
		vi.dwMinorVersion = spritesVersion->otb;
		sprintf(clientString, "%d.%d", spritesVersion->client/100, spritesVersion->client % 100);
	}
	else{
		vi.dwMinorVersion = 0xFFFFFFFF;
		strcpy(clientString,"Unk");
	}
	vi.dwBuildNumber = ItemType::dwBuildNumber + 1; //build
	char str_version[128];
	sprintf(str_version, "OTB %d.%d.%d-%s", vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber, clientString);
	strcpy(vi.CSDVersion, str_version);

	f->setProps(ROOT_ATTR_VERSION, &vi, sizeof(VERSIONINFO));

	ItemMap::iterator it;

	for(it = item.begin(); it != item.end(); it++){
		if(it->second->id >= 20000)
			continue;

		/*
		if(it->second->alwaysOnTopOrder > 0){
			SpriteType* sType = g_itemsSprites->getSprite(it->second->clientid);
			if(it->second->alwaysOnTopOrder != sType->alwaysOnTopOrder){
				it->second->alwaysOnTop = sType->alwaysOnTop;
				it->second->alwaysOnTopOrder = sType->alwaysOnTopOrder;
				it->second->blockProjectile = sType->blockProjectile;
			}
		}

		if(it->second->group != ITEM_GROUP_DEPRECATED){
			SpriteType* sType = g_itemsSprites->getSprite(it->second->clientid);
			if(!it->second->compareOptions(sType)){
				it->second->compareOptions(sType);
			}
		}
		*/

		flags_t flags = 0;
		std::list<itemattrib_t> saveAttr;
		
		saveAttr.push_back(ITEM_ATTR_SERVERID);
		saveAttr.push_back(ITEM_ATTR_CLIENTID);
		
//#ifdef __SPRITE_SEARCH__
		if(it->second->id < 20000){
			saveAttr.push_back(ITEM_ATTR_SPRITEHASH);
			getImageHash(it->second->clientid, it->second->sprHash);
			SpriteType* sprite = g_itemsSprites->getSprite(it->second->clientid);
			if(sprite){
				if(sprite->miniMapColor){
					saveAttr.push_back(ITEM_ATTR_MINIMAPCOLOR);
					it->second->miniMapColor = sprite->miniMapColor;
				}

				if(sprite->subParam07 != 0){
					saveAttr.push_back(ITEM_ATTR_07);
					it->second->subParam07 = g_itemsSprites->getSprite(it->second->clientid)->subParam07;
				}

				if(sprite->subParam08 != 0){
					saveAttr.push_back(ITEM_ATTR_08);
					it->second->subParam08 = g_itemsSprites->getSprite(it->second->clientid)->subParam08;
				}

				if(sprite->lightLevel != 0 || 
					sprite->lightColor != 0 || 
					it->second->lightLevel != 0 ||
					it->second->lightColor != 0){
					it->second->lightLevel = sprite->lightLevel;
					it->second->lightColor = sprite->lightColor;
					saveAttr.push_back(ITEM_ATTR_LIGHT2);
				}

				if(it->second->group != ITEM_GROUP_DEPRECATED){
					switch(sprite->group){
						case ITEM_GROUP_GROUND:
						case ITEM_GROUP_CONTAINER:
						case ITEM_GROUP_SPLASH:
						case ITEM_GROUP_FLUID:
						case ITEM_GROUP_WRITEABLE:
							it->second->group = sprite->group;
							break;

						default:
							switch(it->second->group){
								case ITEM_GROUP_GROUND:
								case ITEM_GROUP_CONTAINER:
								case ITEM_GROUP_SPLASH:
								case ITEM_GROUP_FLUID:
									it->second->group = ITEM_GROUP_NONE;
									break;
							}
							break;
					}
				}
			}
		}

//#endif
		if(it->second->lightLevel != 0 || it->second->lightColor != 0) {
			saveAttr.push_back(ITEM_ATTR_LIGHT2);
		}

		switch(it->second->group){
		case ITEM_GROUP_GROUND:
		case ITEM_GROUP_CONTAINER:
		//case ITEM_GROUP_CHARGES:
		//case ITEM_GROUP_TELEPORT:
		//case ITEM_GROUP_MAGICFIELD:
		//case ITEM_GROUP_DOOR:
		case ITEM_GROUP_SPLASH:
		case ITEM_GROUP_FLUID:
		case ITEM_GROUP_DEPRECATED:
		//case ITEM_GROUP_NONE:
			f->startNode(it->second->group);
			break;
		default:
			f->startNode(ITEM_GROUP_NONE);
			break;
		}

		switch(it->second->group) {
			case ITEM_GROUP_GROUND:
			{				
				saveAttr.push_back(ITEM_ATTR_SPEED);
				break;
			}
		}

		if(it->second->blockSolid)
			flags |= FLAG_BLOCK_SOLID;
		
		if(it->second->blockProjectile)
			flags |= FLAG_BLOCK_PROJECTILE;
		
		if(it->second->blockPathFind)
			flags |= FLAG_BLOCK_PATHFIND;
		
		if(it->second->hasHeight)
			flags |= FLAG_HAS_HEIGHT;

		if(it->second->useable)
			flags |= FLAG_USEABLE;
		
		if(it->second->pickupable)
			flags |= FLAG_PICKUPABLE;
		
		if(it->second->moveable)
			flags |= FLAG_MOVEABLE;
		
		if(it->second->stackable)
			flags |= FLAG_STACKABLE;
		
		/*
		if(it->second->floorChangeDown)
			flags |= FLAG_FLOORCHANGEDOWN;
		
		if(it->second->floorChangeNorth)
			flags |= FLAG_FLOORCHANGENORTH;
		
		if(it->second->floorChangeEast)
			flags |= FLAG_FLOORCHANGEEAST;
		
		if(it->second->floorChangeSouth)
			flags |= FLAG_FLOORCHANGESOUTH;
		
		if(it->second->floorChangeWest)
			flags |= FLAG_FLOORCHANGEWEST;
		*/

		if(it->second->alwaysOnTop){
			flags |= FLAG_ALWAYSONTOP;

			saveAttr.push_back(ITEM_ATTR_TOPORDER);
		}
		
		if(it->second->readable)
			flags |= FLAG_READABLE;

		if(it->second->rotable)
			flags |= FLAG_ROTABLE;
		
		if(it->second->isHangable)
			flags |= FLAG_HANGABLE;

		if(it->second->isVertical)
			flags |= FLAG_VERTICAL;

		if(it->second->isHorizontal)
			flags |= FLAG_HORIZONTAL;

		if(it->second->allowDistRead)
			flags |= FLAG_ALLOWDISTREAD;

		if(it->second->clientCharges)
			flags |= FLAG_CLIENTCHARGES;


		if(it->second->group == ITEM_GROUP_DEPRECATED){
			flags = 0;
			saveAttr.clear();
			saveAttr.push_back(ITEM_ATTR_SERVERID);
		}

		f->setFlags(flags);
		
		std::list<itemattrib_t>::iterator attIt;
		for(attIt = saveAttr.begin(); attIt != saveAttr.end(); ++attIt) {
			switch(*attIt) {
				case ITEM_ATTR_SERVERID:
				{
					f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_CLIENTID:
				{
					f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_SPEED:
				{
					f->setProps(ITEM_ATTR_SPEED, &it->second->speed, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_SPRITEHASH:
				{
					f->setProps(ITEM_ATTR_SPRITEHASH, &it->second->sprHash, sizeof(it->second->sprHash));
					break;
				}
				case ITEM_ATTR_MINIMAPCOLOR:
				{
					f->setProps(ITEM_ATTR_MINIMAPCOLOR, &it->second->miniMapColor, sizeof(it->second->miniMapColor));
					break;
				}
				case ITEM_ATTR_07:
				{
					f->setProps(ITEM_ATTR_07, &it->second->subParam07, sizeof(it->second->subParam07));
					break;
				}
				case ITEM_ATTR_08:
				{
					f->setProps(ITEM_ATTR_08, &it->second->subParam08, sizeof(it->second->subParam08));
					break;
				}
				case ITEM_ATTR_LIGHT2:
				{
					struct lightBlock2 lb2;
					lb2.lightLevel = it->second->lightLevel;
					lb2.lightColor = it->second->lightColor;
					f->setProps(ITEM_ATTR_LIGHT2, &lb2, sizeof(lb2));
					break;
				}
				case ITEM_ATTR_TOPORDER:
				{
					f->setProps(ITEM_ATTR_TOPORDER, &it->second->alwaysOnTopOrder, sizeof(it->second->alwaysOnTopOrder));
					break;
				}
				default:
				{
					break;
				}
			}
		}

		f->endNode();
	}

	f->endNode();

	delete f;
	f = NULL;
	return 1;
}

int ItemsTypes::loadstatus;
bool ItemsTypes::importFromXml(const char *filename)
{
	xmlDocPtr doc = xmlParseFile(filename);
	if(!doc){
		return false;
	}
	
	xmlNodePtr root = xmlDocGetRootElement(doc);

	if(xmlStrcmp(root->name,(const xmlChar*)"items") != 0){
		xmlFreeDoc(doc);
		return false;
	}

	char* nodeValue;
	for(xmlNodePtr itemNode = root->children; itemNode != NULL; itemNode = itemNode->next){
		int id;
		std::string name;

		nodeValue = (char*)xmlGetProp(itemNode, (xmlChar*)"id");
		if(nodeValue){
			id = atoi(nodeValue);
			xmlFree(nodeValue);
		} else continue;

		nodeValue = (char*)xmlGetProp(itemNode, (xmlChar*)"name");
		if(nodeValue){
			name = nodeValue;
			xmlFree(nodeValue);
		} else continue;
		
		ItemType* iType = g_itemsTypes->getType(id);
		if(iType) iType->name = name.c_str();
	}

	return false;
}
