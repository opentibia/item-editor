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

const char* readXmlProp(const char* propName, const xmlProp *props);
extern ItemsTypes *g_itemsTypes;
extern ItemsSprites *g_itemsSprites;
extern void getImageHash(unsigned short cid, void* output);

long ItemType::minClientId = 0;
long ItemType::maxClientId = 0;
long ItemType::minServerId = 0;
long ItemType::maxServerId = 0;
long ItemType::dwMajorVersion = 0;
long ItemType::dwMinorVersion = 0;
long ItemType::dwBuildNumber = 0;

ItemType::ItemType()
{
	group     = ITEM_GROUP_NONE;
	id			  = 100;
	clientid	= 100;
	foundNewImage = false;

	blockSolid	= false;
	blockProjectile = false;
	hasHeight = false;
	blockPathFind = false;

	alwaysOnTop	= false;
	stackable	= false;
	useable		= false;
	moveable	= true;
	pickupable	= false;
	rotable = false;
	readable = false;
	canNotDecay = false;
	
	rotateTo = 0;

	isVertical = false;
	isHorizontal = false;

	isHangable = false;

	miniMapColor = 0;
	subParam07 = 0;
	subParam08 = 0;

	//xml
	memset(name, '\0', sizeof(name));
	memset(descr, '\0', sizeof(descr));
	memset(sprHash, '\0', sizeof(sprHash));
	
	weight = 0.00;
	decayTo = 0;
	decayTime = 0;
	slot_position = SLOT_HAND;

	floorChangeDown = false;
	floorChangeNorth = false;
	floorChangeSouth = false;
	floorChangeEast = false;
	floorChangeWest = false;

	lightLevel = 0;
	lightColor = 0;

	//ground
	speed = 0;

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
	
	//magicfield
	magicfieldtype = 0;
	
	//writeable
	readOnlyId = 0;

	//
	//canWalkThrough = false;
	//ladderUp = false;
	//canSeeThrough = false;
	//wallObject = false;
	//container	= false;
	//isammo = false;
	//groundtile	= false;
	//fluid = false;
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
	stackable = stype->stackable ;
	useable = stype->useable;
	moveable = stype->moveable;
	pickupable = stype->pickupable;
	rotable = stype->rotable;
	readable = stype->readable;
	canNotDecay = false;
	speed = stype->speed;

	miniMapColor = stype->miniMapColor;
	subParam07 = stype->subParam07;
	subParam08 = stype->subParam08;

	#ifdef __SPRITE_SEARCH__
	memcpy(sprHash, stype->sprHash, 16);
	#else
	getImageHash(stype->id, sprHash);
	#endif
	
	rotateTo = 0;

	//xml
	memset(name, '\0', sizeof(name));
	memset(descr, '\0', sizeof(descr));
	
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
	
	//magicfield
	magicfieldtype = 0;
	
	//writeable
	readOnlyId = 0;

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
		if(group != stype->group)
			return false;
	}
	
	if(blockSolid != stype->blockSolid)
		return false;
	
	if(blockPathFind != stype->blockPathFind)
		return false;

	if(alwaysOnTop != stype->alwaysOnTop)
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

	if(miniMapColor != stype->miniMapColor)
		return false;
	
	if(subParam07 != stype->subParam07)
		return false;

	if(subParam08 != stype->subParam08)
		return false;

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

	return true;
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

bool ItemsTypes::loadFromDat(const char *filename)
{
	if(datLoaded)
		return false;
	
	unsigned short id = 100;  // tibia.dat start with id 100
	FILE *fp;
	long size;
	int speed;
	short read_short;
	
	fp = fopen(filename, "rb");
	if(!fp)
		return false;
	
	fseek(fp,0,SEEK_END);
	size = ftell(fp);

	//get max id
	fseek(fp, 0x04, SEEK_SET);
	fread(&read_short, 2, 1, fp); 
	ItemType::maxClientId = read_short;
	ItemType::minClientId = 100;

	fseek(fp, 0x0C, SEEK_SET);
	// loop throw all Items until we reach the end of file
	while(ftell(fp) < size && id <= ItemType::maxClientId)
	{
		ItemType *sType = getType(id);
		if(!sType) {
			sType = new ItemType();

			sType->id = id;
			sType->clientid = id;
			getImageHash(sType->clientid, sType->sprHash);
			addType(id, sType);
		}

		// read the options until we find a 0xff
		int optbyte;
		
		while (((optbyte = fgetc(fp)) >= 0) && (optbyte != 0xFF))
		{                                                            
			switch (optbyte)
			{
				case 0x00: //is groundtile
				{
					fread(&read_short, 2, 1, fp); 
					speed = read_short;
					sType->speed = speed;
					sType->group = ITEM_GROUP_GROUND;
					break;
				}
				case 0x01: //all on top
				{
					sType->alwaysOnTop = true;
					break;
				}
				case 0x02: //can walk trough (open doors, arces, bug pen fence ??)
				{
					sType->alwaysOnTop = true;
					//sType->canWalkThrough = true;
					break;
				}
				case 0x03: //is a container
				{
					sType->group = ITEM_GROUP_CONTAINER;
					break;
				}
				case 0x04: //is stackable
				{
					sType->stackable = true;
					break;
				}
				case 0x05: //is useable
				{
					sType->useable = true;
					break;
				}
				case 0x0A: //liquid with states
				{
					sType->group = ITEM_GROUP_SPLASH;
					break;
				}
				case 0x0B: //is blocking
				{
					sType->blockSolid = true;
					break;
				}
				case 0x0C: //is not moveable
				{
					sType->moveable = false;
					break;
				}
				case 0x0F: //can be equipped
				{
					sType->pickupable = true;
					break;
				}
				case 0x10: //makes light (skip 4 bytes)
				{
					unsigned short lightlevel;
					fread(&lightlevel, sizeof(lightlevel), 1, fp);
					sType->lightLevel = lightlevel;

					unsigned short lightcolor;
					fread(&lightcolor, sizeof(lightcolor), 1, fp);
					sType->lightColor = lightcolor;

					break;
				}
				case 0x06: //ladder up (id 1386)   why a group for just 1 item ???   
				{
					//sType->ladderUp = true;
					break;
				}
				case 0x09: //can contain fluids
				{
					sType->group = ITEM_GROUP_FLUID;
					break;
				}
				case 0x0D: //"visibility"- for drawing visible view
				{
					sType->blockProjectile = true;
					break;
				}
				case 0x0E: //blocks creature movement (flowers, parcels etc)
				{
					sType->blockPathFind = true;
					break;
				}
				case 0x11: //can see what is under (ladder holes, stairs holes etc)
				{
					//sType->canSeeThrough = true;
					break;
				}
				case 0x12: //ground tiles that don't cause level change
				{
					//sType->floorchange = false;
					break;
				}
				case 0x18: //draw with height offset for all parts (2x2) of the sprite
				{
					break;
				}
				case 0x14: //sprite-drawing related
				{
					//sType->hasParameter14 = true;
					break;
				}
				case 0x07: //writtable objects
				{
					sType->group = ITEM_GROUP_WRITEABLE;
					sType->readable = true;

					unsigned short us;
					fread(&us, sizeof(us), 1, fp); //unknown, values like 80, 200, 512, 1024, 2000
					sType->subParam07 = us;
					break;
				}
				case 0x08: //writtable objects that can't be edited
				{
					sType->readable = true;
					unsigned short us;
					fread(&us, sizeof(us), 1, fp); //unknown, all have the value 1024
					sType->subParam08 = us;
					break;
				}
				case 0x13: //items that have height
				{
					sType->hasHeight = true;

					unsigned short heightdisp;
					fread(&heightdisp, sizeof(heightdisp), 1, fp);
					//sType->heightdisplacement = heightdisp;
					break;
				}
				case 0x16: //minimap drawing
				{
					unsigned short color;
					fread(&color, sizeof(color), 1, fp);
					sType->miniMapColor = color;

					break;
				}
				case 0x1A: //vertical objects (walls to hang objects on etc)
				{
					sType->isHorizontal = true;
					break;  
				}
				case 0x1B: //walls 2 types of them same material (total 4 pairs)                  
				{
					sType->isVertical = true;
					break;
				}
				case 0x1D: //line spot ...
				{
					int tmp;
					tmp = fgetc(fp); // 86 -> openable holes, 77-> can be used to go down, 76 can be used to go up, 82 -> stairs up, 79 switch,    
					if(tmp == 0x58)
						sType->readable = true;
					fgetc(fp); // always 4
					break;         
				}
				case 0x19: //hangable objects
				{
					sType->isHangable = true;
					break;    
				}
				case 0x17: //seems like decorables with 4 states of turning (exception first 4 are unique statues)                 
				{
					sType->rotable = true;
					break;
				}
				case 0x1C: //monster has animation even when iddle (rot, wasp, slime, fe)
				{
					//sType->hasAnimation = true;
					break;            
				}
				default:
				{
					//std::cout << "unknown byte: " << (unsigned short)optbyte << std::endl;
					return false;
					break;
				}
			}
		}

		// now skip the size and sprite data		
 		int width  = fgetc(fp);
 		int height = fgetc(fp);
 		if ((width > 1) || (height > 1))
 		   fgetc(fp);
 		   
		int blendframes = fgetc(fp);
		int xdiv        = fgetc(fp);
		int ydiv        = fgetc(fp);
		int animcount   = fgetc(fp);

		fseek(fp, width*height*blendframes*xdiv*ydiv*animcount*2, SEEK_CUR);
		++id;
	}

	datLoaded = true;
	return true;
}

int ItemsTypes::loadstatus;
bool ItemsTypes::loadFromXml(const char *filename)
{
	void *buff;
	int bytes_read;
	int eof;

	XML_Parser p = XML_ParserCreate(NULL);

	eof = 0;
	if(!p){
		return false;
	}

	XML_SetElementHandler(p, xmlstartNode, xmlendNode);

	FILE *f = fopen(filename,"rb");
	if(!f){
		XML_ParserFree(p);
		return false;
	}

	loadstatus = 0;
	do{
		buff = XML_GetBuffer(p, 1024);

		bytes_read = fread(buff, 1, 1024, f);
		if(bytes_read != 1024){
			if(feof(f) == 0){
				//error
				XML_ParserFree(p);
				return false;
			}
			else{
				eof = 1;
			}
		}

		XML_Status status = XML_ParseBuffer(p, bytes_read, eof);
		switch (status) {
		case XML_STATUS_ERROR:
			XML_ParserFree(p);
			return false;
		}
	}while(eof == 0 && loadstatus == 0);

	XML_ParserFree(p);
	if(loadstatus != 0){
		return false;
	}
	else{
		return true;
	}

	return false;
}

const char* readXmlProp(const char* propName, const xmlProp *props)
{
	int i = 0;
	while(props[i].name != NULL){
		if(strcmp(props[i].name, propName) == 0){
			return props[i].value;
		}
		i++;
	}
	return NULL;

}

void XMLCALL ItemsTypes::xmlstartNode(void *userData, const char *name, const char **atts)
{
	if(loadstatus == 1)
		return;
	
	const xmlProp *props = (const xmlProp *)atts;

	if(strcmp(name, "item") == 0){
		int id;
		const char* tmp;
		ItemType* sType = NULL;

		if((tmp = readXmlProp("id", props)) != 0){
			id = atoi(tmp);
			sType = g_itemsTypes->getType(id);
			if(!sType) {
				sType = new ItemType();

				if(id < ItemType::minClientId){
					sType->id = id + 20000;
				}
				else{
					sType->id = id;
				}

				if(id > ItemType::maxClientId || id < ItemType::minClientId){
					sType->clientid = 0;
				}
				else{
					sType->clientid = id;
				}
					
				g_itemsTypes->addType(sType->id, sType);
			}
		}
		else
			loadstatus = 1;

		if(sType) {
			if((tmp = readXmlProp("name", props)) != 0){
				//sType->name = tmp;
				strncpy(sType->name, tmp, 127);
			}

			if((tmp = readXmlProp("descr", props)) != 0){
				//sType->descr = tmp;
				strncpy(sType->descr, tmp, 127);
			}

			if((tmp = readXmlProp("weight", props)) != 0){
				sType->weight = atof(tmp);
			}

			if((tmp = readXmlProp("decayto", props)) != 0){
				sType->decayTo = atoi(tmp);
			}

			if((tmp = readXmlProp("decaytime", props)) != 0){
				sType->decayTime = atoi(tmp);
			}

			if((tmp = readXmlProp("blockingprojectile", props)) != 0){
				if(atoi(tmp) == 0)
					sType->blockProjectile = false;
				else
					sType->blockProjectile = true;
			}

			if((tmp = readXmlProp("floorchange", props)) != 0){
				sType->floorChangeDown = true;
			}

			if((tmp = readXmlProp("floorchangenorth", props)) != 0){
				sType->floorChangeNorth = true;
			}

			if((tmp = readXmlProp("floorchangesouth", props)) != 0){
				sType->floorChangeSouth = true;
			}

			if((tmp = readXmlProp("floorchangeeast", props)) != 0){
				sType->floorChangeEast = true;
			}

			if((tmp = readXmlProp("floorchangewest", props)) != 0){
				sType->floorChangeWest = true;
			}
			
			if((tmp = readXmlProp("position", props)) != 0){
				if (!strcmp(tmp, "helmet") || !strcmp(tmp, "head"))
					sType->slot_position = SLOT_HEAD;
				else if (!strcmp(tmp, "amulet"))
					sType->slot_position = SLOT_AMULET;
				else if (!strcmp(tmp, "body"))
					sType->slot_position = SLOT_BODY;
				else if (!strcmp(tmp, "legs"))
					sType->slot_position = SLOT_LEGS;
				else if (!strcmp(tmp, "boots"))
					sType->slot_position = SLOT_FEET;
				else if (!strcmp(tmp, "ring"))
					sType->slot_position = SLOT_RING;
				else if (!strcmp(tmp, "backpack"))					 	
					sType->slot_position = SLOT_BACKPACK;
				else if (!strcmp(tmp, "weapon"))
					sType->slot_position = SLOT_HAND;
				else if (!strcmp(tmp, "twohand"))
					sType->slot_position = SLOT_2HAND;
				else if (!strcmp(tmp, "hand"))
					sType->slot_position = SLOT_HAND;
				
				//sType->slot_position = SLOT_HAND;
			}
			
			// now set special properties...
			// first we check the type...
			const char* type;
			if((type = readXmlProp("type", props)) != 0) {

				//container
				if(strcmp(type, "container") == 0) {
					sType->group = ITEM_GROUP_CONTAINER;
					if((tmp = readXmlProp("maxitems", props)) != 0){
						sType->maxItems = atoi(tmp);
					}
				}
				//weapon
				else if(strcmp(type, "weapon") == 0) {
					sType->group = ITEM_GROUP_WEAPON;

					const char* skill;
					if((skill = readXmlProp("skill", props)) != 0){
						if (!strcmp(skill, "sword"))
							sType->weaponType = WEAPON_SWORD;
						else if (!strcmp(skill, "club"))
							sType->weaponType = WEAPON_CLUB;
						else if (!strcmp(skill, "axe"))
							sType->weaponType = WEAPON_AXE;
						else if (!strcmp(skill, "shielding"))
							sType->weaponType = WEAPON_SHIELD;
						else if (!strcmp(skill, "distance")){
							sType->weaponType = WEAPON_DIST;

							const char *amutype;
							if((amutype = readXmlProp("amutype", props)) != 0){
								if (!strcmp(amutype, "bolt"))
									sType->amuType = AMU_BOLT;
								else if (!strcmp(amutype, "arrow"))
									sType->amuType = AMU_ARROW;
							}
							else{ //no ammunition, check shoottype
								const char *sshoottype;
								if((sshoottype = readXmlProp("shottype", props)) != 0){
									if (!strcmp(sshoottype, "throwing-star"))
							    	sType->shootType = DIST_THROWINGSTAR;
							    else if (!strcmp(sshoottype, "throwing-knife"))
							    	sType->shootType = DIST_THROWINGKNIFE;
							    else if (!strcmp(sshoottype, "small-stone"))
							    	sType->shootType = DIST_SMALLSTONE;
							    else if (!strcmp(sshoottype, "sudden-death"))
							    	sType->shootType = DIST_SUDDENDEATH;
							    else if (!strcmp(sshoottype, "large-rock"))
							    	sType->shootType = DIST_LARGEROCK;
							    else if (!strcmp(sshoottype, "snowball"))
							    	sType->shootType = DIST_SNOWBALL;
							    else if (!strcmp(sshoottype, "spear"))
							    	sType->shootType = DIST_SPEAR;

                				/*else
										std::cout << "wrong shootype tag" << std::endl;*/
								}
								/*else
									std::cout << "missing shoottype type for distante-item: " << id << std::endl;*/
							}
						}
						//magic
						else if(!strcmp(skill, "magic")){
							sType->weaponType = WEAPON_MAGIC;

							const char *sshoottype;
							if((sshoottype = readXmlProp("shottype", props)) != 0) {
								if (!strcmp(sshoottype, "fire"))
							    sType->shootType = DIST_FIRE;
							  else if (!strcmp(sshoottype, "energy"))
							    sType->shootType = DIST_ENERGY;
							  /*else
							    std::cout << "wrong shootype tag" << std::endl;*/
							}								
						}
						//shielding
						else if(!strcmp(skill, "shielding")) {
							sType->weaponType = WEAPON_SHIELD;
						/*else
							std::cout << "wrong skill tag for weapon" << std::endl;*/
						}
					}//skills
					
					const char* attack;
					if((attack = readXmlProp("attack", props)) != 0) {
						sType->attack = atoi(attack);
					}

					const char* defence;
					if((defence = readXmlProp("defence", props)) != 0) {
						sType->defence = atoi(defence);
					}
				}
				//ammunition
				else if(strcmp(type, "amunition") == 0) {
					sType->group = ITEM_GROUP_AMMUNITION;
					
					const char *amutype;
					if((amutype = readXmlProp("amutype", props)) != 0) {
						if (!strcmp(amutype, "bolt"))
							sType->amuType = AMU_BOLT;
						else if (!strcmp(amutype, "arrow"))
							sType->amuType = AMU_ARROW;
					}
					/*else
						std::cout << "wrong amutype tag for item: " << id << std::endl;*/

					const char *sshoottype;
					if((sshoottype = readXmlProp("shottype", props)) != 0) {
						if (!strcmp(sshoottype, "bolt"))
							sType->shootType = DIST_BOLT;
						else if (!strcmp(sshoottype, "arrow"))
							sType->shootType = DIST_ARROW;
						else if (!strcmp(sshoottype, "poison-arrow"))
							sType->shootType = DIST_POISONARROW;
						else if (!strcmp(sshoottype, "burst-arrow"))
							sType->shootType = DIST_BURSTARROW;
						else if (!strcmp(sshoottype, "power-bolt"))
							sType->shootType = DIST_POWERBOLT;
						/*else
							std::cout << "wrong shootype tag for item: " << id << std::endl;*/
					}
					/*else
						std::cout << "missing shoottype for item: " << id <<  std::endl;*/

					const char* attack;
					if((attack = readXmlProp("attack", props)) != 0) {
						sType->attack = atoi(attack);
					}
				}
				//armor
				else if(strcmp(type, "armor") == 0) {
					sType->group = ITEM_GROUP_ARMOR;

					if((tmp = readXmlProp("arm", props)) != 0) {
						sType->armor = atoi(tmp);
					}
					/*else
						std::cout << "missing arm tag for armor: " << id << std::endl;*/
				}
				//rune
				else if (!strcmp(type, "rune")) {
					sType->group = ITEM_GROUP_RUNE;

					if((tmp = readXmlProp("maglevel", props)) != 0) {
						sType->runeMagLevel = atoi(tmp);
					}
					/*else
						std::cout << "missing maglevel for rune: " << id << std::endl;*/
				}
				//teleport
				else if (!strcmp(type, "teleport")) {
					sType->group = ITEM_GROUP_TELEPORT;
				}
				//magicfield
				else if (!strcmp(type, "magicfield")) {
					sType->group = ITEM_GROUP_MAGICFIELD;

					if((tmp = readXmlProp("fieldtype", props)) != 0) {
						if (!strcmp(tmp, "fire"))
							sType->magicfieldtype = MAGIC_FIELD_FIRE;
						else if (!strcmp(tmp, "energy"))
							sType->magicfieldtype = MAGIC_FIELD_ENERGY;
						else if (!strcmp(tmp, "poison"))
							sType->magicfieldtype = MAGIC_FIELD_POISON;
						/*else
							std::cout << "wrong field type tag for item: " << id << std::endl;*/
					}
					/*else
	       		std::cout << "missing field type for field: " << id << std::endl;*/
				}
				//oneTimeWrite
				else if(!strcmp(type, "write1time")) {
					sType->group = ITEM_GROUP_WRITEABLE;
					sType->readable = true;;
					if((tmp = readXmlProp("readonlyid", props)) != 0) {
						sType->readOnlyId = atoi(tmp);
					}
				}
				//key
				else if(!strcmp(type, "key")) {
					sType->group = ITEM_GROUP_KEY;
				}
				//splash
				else if(!strcmp(type, "splash")) {
					sType->group = ITEM_GROUP_SPLASH;
				}
			}
		}
		/*else
			loadstatus = 1;*/
	}
}

void XMLCALL ItemsTypes::xmlendNode(void *userData, const char *name)
{
	//
}

bool ItemsTypes::importFromXml(const char *filename)
{
	void *buff;
	int bytes_read;
	int eof;

	XML_Parser p = XML_ParserCreate(NULL);

	eof = 0;
	if(!p){
		return false;
	}

	XML_SetElementHandler(p, xmlstartNodeImport, xmlendNode);

	FILE *f = fopen(filename,"rb");
	if(!f){
		XML_ParserFree(p);
		return false;
	}

	loadstatus = 0;
	do{
		buff = XML_GetBuffer(p, 1024);

		bytes_read = fread(buff, 1, 1024, f);
		if(bytes_read != 1024){
			if(feof(f) == 0){
				//error
				XML_ParserFree(p);
				return false;
			}
			else{
				eof = 1;
			}
		}

		XML_Status status = XML_ParseBuffer(p, bytes_read, eof);
		switch (status) {
		case XML_STATUS_ERROR:
			XML_ParserFree(p);
			return false;
		}
	}while(eof == 0 && loadstatus == 0);

	XML_ParserFree(p);
	if(loadstatus != 0){
		return false;
	}
	else{
		return true;
	}

	return false;
}

bool ItemsTypes::exportToXml(const char *filename)
{
	FILE *f;
	f = fopen(filename, "w");
	if(!f)
		return false;

	fprintf(f, "<?xml version=\"1.0\"?><items>\n");

	ItemMap::iterator it = getTypes();
	for(;it != getEnd(); it++){
		fprintf(f, "<item id=\"%d\" name=\"%s\"/>\n", it->first, it->second->name);
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

void XMLCALL ItemsTypes::xmlstartNodeImport(void *userData, const char *name, const char **atts)
{
	if(loadstatus == 1)
		return;
	
	const xmlProp *props = (const xmlProp *)atts;

	if(strcmp(name, "item") == 0){
		int id;
		const char* tmp;
		ItemType* sType = NULL;

		if((tmp = readXmlProp("id", props)) != 0){
			id = atoi(tmp);
			sType = g_itemsTypes->getType(id);
			//sType = g_itemsTypes->getTypeBySrpite(id);
		}
		else
			loadstatus = 1;

		if(sType) {
			if((tmp = readXmlProp("name", props)) != 0){
				if(strlen(sType->name) != 0){
					//__asm int 3
					return;
				}
				strncpy(sType->name, tmp, 127);
			}
			else{
				return;
			}


			if((tmp = readXmlProp("descr", props)) != 0){
				strncpy(sType->descr, tmp, 127);
			}

			if((tmp = readXmlProp("weight", props)) != 0){
				sType->weight = atof(tmp);
			}

			if((tmp = readXmlProp("decayto", props)) != 0){
				sType->decayTo = atoi(tmp);
			}

			if((tmp = readXmlProp("decaytime", props)) != 0){
				sType->decayTime = atoi(tmp);
			}

			if((tmp = readXmlProp("blockingprojectile", props)) != 0){
				if(atoi(tmp) == 0)
					sType->blockProjectile = false;
				else
					sType->blockProjectile = true;
			}

			if((tmp = readXmlProp("floorchange", props)) != 0){
				sType->floorChangeDown = true;
			}

			if((tmp = readXmlProp("floorchangenorth", props)) != 0){
				sType->floorChangeNorth = true;
			}

			if((tmp = readXmlProp("floorchangesouth", props)) != 0){
				sType->floorChangeSouth = true;
			}

			if((tmp = readXmlProp("floorchangeeast", props)) != 0){
				sType->floorChangeEast = true;
			}

			if((tmp = readXmlProp("floorchangewest", props)) != 0){
				sType->floorChangeWest = true;
			}
			
			if((tmp = readXmlProp("position", props)) != 0){
				if (!strcmp(tmp, "helmet") || !strcmp(tmp, "head"))
					sType->slot_position = SLOT_HEAD;
				else if (!strcmp(tmp, "amulet"))
					sType->slot_position = SLOT_AMULET;
				else if (!strcmp(tmp, "body"))
					sType->slot_position = SLOT_BODY;
				else if (!strcmp(tmp, "legs"))
					sType->slot_position = SLOT_LEGS;
				else if (!strcmp(tmp, "boots"))
					sType->slot_position = SLOT_FEET;
				else if (!strcmp(tmp, "ring"))
					sType->slot_position = SLOT_RING;
				else if (!strcmp(tmp, "backpack"))					 	
					sType->slot_position = SLOT_BACKPACK;
				else if (!strcmp(tmp, "weapon"))
					sType->slot_position = SLOT_HAND;
				else if (!strcmp(tmp, "twohand"))
					sType->slot_position = SLOT_2HAND;
				else if (!strcmp(tmp, "hand"))
					sType->slot_position = SLOT_HAND;
				
			}
			
			// now set special properties...
			// first we check the type...
			const char* type;
			if((type = readXmlProp("type", props)) != 0) {

				//container
				if(strcmp(type, "container") == 0) {
					sType->group = ITEM_GROUP_CONTAINER;
					if((tmp = readXmlProp("maxitems", props)) != 0){
						sType->maxItems = atoi(tmp);
					}
				}
				//weapon
				else if(strcmp(type, "weapon") == 0) {
					sType->group = ITEM_GROUP_WEAPON;

					const char* skill;
					if((skill = readXmlProp("skill", props)) != 0){
						if (!strcmp(skill, "sword"))
							sType->weaponType = WEAPON_SWORD;
						else if (!strcmp(skill, "club"))
							sType->weaponType = WEAPON_CLUB;
						else if (!strcmp(skill, "axe"))
							sType->weaponType = WEAPON_AXE;
						else if (!strcmp(skill, "shielding"))
							sType->weaponType = WEAPON_SHIELD;
						else if (!strcmp(skill, "distance")){
							sType->weaponType = WEAPON_DIST;

							const char *amutype;
							if((amutype = readXmlProp("amutype", props)) != 0){
								if (!strcmp(amutype, "bolt"))
									sType->amuType = AMU_BOLT;
								else if (!strcmp(amutype, "arrow"))
									sType->amuType = AMU_ARROW;
							}
							else{ //no ammunition, check shoottype
								const char *sshoottype;
								if((sshoottype = readXmlProp("shottype", props)) != 0){
									if (!strcmp(sshoottype, "throwing-star"))
							    	sType->shootType = DIST_THROWINGSTAR;
							    else if (!strcmp(sshoottype, "throwing-knife"))
							    	sType->shootType = DIST_THROWINGKNIFE;
							    else if (!strcmp(sshoottype, "small-stone"))
							    	sType->shootType = DIST_SMALLSTONE;
							    else if (!strcmp(sshoottype, "sudden-death"))
							    	sType->shootType = DIST_SUDDENDEATH;
							    else if (!strcmp(sshoottype, "large-rock"))
							    	sType->shootType = DIST_LARGEROCK;
							    else if (!strcmp(sshoottype, "snowball"))
							    	sType->shootType = DIST_SNOWBALL;
							    else if (!strcmp(sshoottype, "spear"))
							    	sType->shootType = DIST_SPEAR;
#ifdef WIN32
                					else
									#ifdef __GNUC__
                                    __asm__("int $3");
                                    #else
                                    __asm int 3
                                    #endif
#endif
								}
#ifdef WIN32
								else
									#ifdef __GNUC__
                                    __asm__("int $3");
                                    #else
                                    __asm int 3
                                    #endif
#endif
							}
						}
						//magic
						else if(!strcmp(skill, "magic")){
							sType->weaponType = WEAPON_MAGIC;

							const char *sshoottype;
							if((sshoottype = readXmlProp("shottype", props)) != 0) {
								if (!strcmp(sshoottype, "fire"))
							    sType->shootType = DIST_FIRE;
							  else if (!strcmp(sshoottype, "energy"))
							    sType->shootType = DIST_ENERGY;
#ifdef WIN32
							  else
                                #ifdef __GNUC__
							    __asm__("int $3");
							    #else
							    __asm int 3
							    #endif
#endif
							}								
						}
						//shielding
						else if(!strcmp(skill, "shielding")) 
							sType->weaponType = WEAPON_SHIELD;
#ifdef WIN32
						else
						   #ifdef __GNUC__
                           __asm__("int $3");
                           #else
                           __asm int 3
                           #endif
#endif
					}//skills
					
					const char* attack;
					if((attack = readXmlProp("attack", props)) != 0) {
						sType->attack = atoi(attack);
					}

					const char* defence;
					if((defence = readXmlProp("defence", props)) != 0) {
						sType->defence = atoi(defence);
					}
				}
				//ammunition
				else if(strcmp(type, "amunition") == 0) {
					sType->group = ITEM_GROUP_AMMUNITION;
					
					const char *amutype;
					if((amutype = readXmlProp("amutype", props)) != 0) {
						if (!strcmp(amutype, "bolt"))
							sType->amuType = AMU_BOLT;
						else if (!strcmp(amutype, "arrow"))
							sType->amuType = AMU_ARROW;
					}
#ifdef WIN32
					else
					    #ifdef __GNUC__
						__asm__("int $3");
						#else
						__asm int 3
						#endif
#endif

					const char *sshoottype;
					if((sshoottype = readXmlProp("shottype", props)) != 0) {
						if (!strcmp(sshoottype, "bolt"))
							sType->shootType = DIST_BOLT;
						else if (!strcmp(sshoottype, "arrow"))
							sType->shootType = DIST_ARROW;
						else if (!strcmp(sshoottype, "poison-arrow"))
							sType->shootType = DIST_POISONARROW;
						else if (!strcmp(sshoottype, "burst-arrow"))
							sType->shootType = DIST_BURSTARROW;
						else if (!strcmp(sshoottype, "power-bolt"))
							sType->shootType = DIST_POWERBOLT;
#ifdef WIN32
						else
						    #ifdef __GNUC__
							__asm__("int $3");
							#else
							__asm int 3
							#endif
#endif
					}
#ifdef WIN32
					else
					    #ifdef __GNUC__
						__asm__("int $3");
						#else
						__asm int 3
						#endif
#endif

					const char* attack;
					if((attack = readXmlProp("attack", props)) != 0) {
						sType->attack = atoi(attack);
					}
				}
				//armor
				else if(strcmp(type, "armor") == 0) {
					sType->group = ITEM_GROUP_ARMOR;

					if((tmp = readXmlProp("arm", props)) != 0) {
						sType->armor = atoi(tmp);
					}
#ifdef WIN32
					else
					    #ifdef __GNUC__
						__asm__("int $3");
						#else
						__asm int 3
						#endif
#endif
				}
				//rune
				else if (!strcmp(type, "rune")) {
					sType->group = ITEM_GROUP_RUNE;

					if((tmp = readXmlProp("maglevel", props)) != 0) {
						sType->runeMagLevel = atoi(tmp);
					}
#ifdef WIN32
					else
					    #ifdef __GNUC__
						__asm__("int $3");
						#else
						__asm int 3
						#endif
#endif
				}
				//teleport
				else if (!strcmp(type, "teleport")) {
					sType->group = ITEM_GROUP_TELEPORT;
				}
				//magicfield
				else if (!strcmp(type, "magicfield")) {
					sType->group = ITEM_GROUP_MAGICFIELD;

					if((tmp = readXmlProp("fieldtype", props)) != 0) {
						if (!strcmp(tmp, "fire"))
							sType->magicfieldtype = MAGIC_FIELD_FIRE;
						else if (!strcmp(tmp, "energy"))
							sType->magicfieldtype = MAGIC_FIELD_ENERGY;
						else if (!strcmp(tmp, "poison"))
							sType->magicfieldtype = MAGIC_FIELD_POISON;
#ifdef WIN32
						else
						    #ifdef __GNUC__
							__asm__("int $3");
							#else
							__asm int 3
							#endif
#endif
					}
#ifdef WIN32
					else
					    #ifdef __GNUC__
	       				__asm__("int $3");
	       				#else
	       				__asm int 3
	       				#endif
#endif
				}
				//oneTimeWrite
				else if(!strcmp(type, "write1time")) {
					sType->group = ITEM_GROUP_WRITEABLE;
					sType->readable = true;;
					if((tmp = readXmlProp("readonlyid", props)) != 0) {
						sType->readOnlyId = atoi(tmp);
					}
				}
				//key
				else if(!strcmp(type, "key")) {
					sType->group = ITEM_GROUP_KEY;
				}
				//splash
				else if(!strcmp(type, "splash")) {
					sType->group = ITEM_GROUP_SPLASH;
				}
			}

		}
	}
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

	NODE node = f.getChildNode(NULL, type);
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
					case ITEM_GROUP_RUNE:
					case ITEM_GROUP_TELEPORT:
					case ITEM_GROUP_MAGICFIELD:
					case ITEM_GROUP_WRITEABLE:
					case ITEM_GROUP_KEY:
					case ITEM_GROUP_SPLASH:
					case ITEM_GROUP_FLUID:
					{
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
							sType->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
							sType->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
							sType->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
							sType->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
							sType->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
							sType->alwaysOnTop = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
							sType->readable = ((flags & FLAG_READABLE) == FLAG_READABLE);
							sType->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
							sType->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
							sType->isVertical = ((flags & FLAG_VERTICAL) == FLAG_VERTICAL);
							sType->isHorizontal = ((flags & FLAG_HORIZONTAL) == FLAG_HORIZONTAL);
							sType->canNotDecay = ((flags & FLAG_CANNOTDECAY) == FLAG_CANNOTDECAY);

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

								memcpy(sType->name, p, datalen);
								sType->name[datalen] = 0;
								break;
							}
							case ITEM_ATTR_DESCR:
							{
								if(datalen >= sizeof(sType->descr))
									return ERROR_INVALID_FORMAT;

								memcpy(sType->descr, p, datalen);
								sType->descr[datalen] = 0;
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

								memcpy(&sType->slot_position, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_MAXITEMS:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->maxItems, p, sizeof(unsigned short));
								break;
							}
							case ITEM_ATTR_WEIGHT:
							{
								if(datalen != sizeof(double))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->weight, p, sizeof(double));
								break;
							}
							case ITEM_ATTR_WEAPON:
							{
								if(datalen != sizeof(weaponBlock))
									return ERROR_INVALID_FORMAT;

								weaponBlock wb;
								memcpy(&wb, p, sizeof(weaponBlock));
								sType->weaponType = (WeaponType)wb.weaponType;
								sType->shootType = (subfight_t)wb.shootType;
								sType->amuType = (amu_t)wb.amuType;
								sType->attack = wb.attack;
								sType->defence = wb.defence;
								break;
							}
							case ITEM_ATTR_AMU:
							{
								if(datalen != sizeof(amuBlock))
									return ERROR_INVALID_FORMAT;

								amuBlock ab;
								memcpy(&ab, p, sizeof(amuBlock));
								sType->shootType = (subfight_t)ab.shootType;
								sType->amuType = (amu_t)ab.amuType;
								sType->attack = ab.attack;
								break;
							}
							case ITEM_ATTR_ARMOR:
							{
								if(datalen != sizeof(armorBlock))
									return ERROR_INVALID_FORMAT;

								armorBlock ab;
								memcpy(&ab, p, sizeof(armorBlock));
									
								sType->armor = ab.armor;
								sType->weight = ab.weight;
								sType->slot_position = (slots_t)ab.slot_position;

								break;
							}
							case ITEM_ATTR_MAGLEVEL:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;
								
								memcpy(&sType->runeMagLevel, p, sizeof(unsigned short));
								
								break;
							}
							case ITEM_ATTR_MAGFIELDTYPE:
							{
								if(datalen != sizeof(unsigned char))
									return ERROR_INVALID_FORMAT;
								
								memcpy(&sType->magicfieldtype, p, sizeof(unsigned char));

								break;
							}
							case ITEM_ATTR_WRITEABLE:
							{
								if(datalen != sizeof(writeableBlock))
									return ERROR_INVALID_FORMAT;

								struct writeableBlock wb;
								memcpy(&wb, p, sizeof(writeableBlock));

								sType->readOnlyId = wb.readOnlyId;

								break;
							}
							case ITEM_ATTR_ROTATETO:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->rotateTo, p, sizeof(unsigned short));

								break;
							}
							case ITEM_ATTR_DECAY:
							{
								if(datalen != sizeof(decayBlock))
									return ERROR_INVALID_FORMAT;

								decayBlock db;
								memcpy(&db, p, sizeof(decayBlock));
								sType->decayTime = db.decayTime;
								sType->decayTo = db.decayTo;
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
								sType->decayTime = db2.decayTime;
								sType->decayTo = db2.decayTo;
								break;
							}

							case ITEM_ATTR_WEAPON2:
							{
								if(datalen != sizeof(weaponBlock2))
									return ERROR_INVALID_FORMAT;

								weaponBlock2 wb2;
								memcpy(&wb2, p, sizeof(weaponBlock2));
								sType->weaponType = (WeaponType)wb2.weaponType;
								sType->shootType = (subfight_t)wb2.shootType;
								sType->amuType = (amu_t)wb2.amuType;
								sType->attack = wb2.attack;
								sType->defence = wb2.defence;
								break;
							}

							case ITEM_ATTR_AMU2:
							{
								if(datalen != sizeof(amuBlock2))
									return ERROR_INVALID_FORMAT;

								amuBlock2 ab2;
								memcpy(&ab2, p, sizeof(amuBlock2));
								sType->shootType = (subfight_t)ab2.shootType;
								sType->amuType = (amu_t)ab2.amuType;
								sType->attack = ab2.attack;
								break;
							}

							case ITEM_ATTR_ARMOR2:
							{
								if(datalen != sizeof(armorBlock2))
									return ERROR_INVALID_FORMAT;

								armorBlock2 ab2;
								memcpy(&ab2, p, sizeof(armorBlock2));									
								sType->armor = ab2.armor;
								sType->weight = ab2.weight;
								sType->slot_position = (slots_t)ab2.slot_position;

								break;
							}

							case ITEM_ATTR_WRITEABLE2:
							{
								if(datalen != sizeof(writeableBlock2))
									return ERROR_INVALID_FORMAT;

								struct writeableBlock2 wb2;
								memcpy(&wb2, p, sizeof(writeableBlock2));
								sType->readOnlyId = wb2.readOnlyId;

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
			/// GET EXTRA INFOS FROM TIBIA.DAT/////
			/*SpriteType *st = g_itemsSprites->getSprite(sType->clientid);
			if(st){
				sType->isVertical = st->isVertical;
				sType->isHorizontal = st->isHorizontal;
				sType->isHangable = st->isHangable ;
				//sType->alwaysOnTop = st->alwaysOnTop;
				//sType->lightLevel = st->lightLevel;
				//sType->lightColor = st->lightColor;
			}
			///*/
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

	vi.dwMajorVersion = 1;
	vi.dwMinorVersion = CLIENT_VERSION_755;
	vi.dwBuildNumber = ItemType::dwBuildNumber + 1;
	char str_version[128];
	sprintf(str_version, "OTB 1.2.%d-7.55", vi.dwBuildNumber);
	strcpy(vi.CSDVersion, str_version);

	f->setProps(ROOT_ATTR_VERSION, &vi, sizeof(VERSIONINFO));

	ItemMap::iterator it;
	for(it = item.begin(); it != item.end(); it++){
		f->startNode(it->second->group);

		flags_t flags = 0;
		std::list<itemattrib_t> saveAttr;
		
		saveAttr.push_back(ITEM_ATTR_SERVERID);
		saveAttr.push_back(ITEM_ATTR_CLIENTID);

		if(strlen(it->second->name) > 0)
			saveAttr.push_back(ITEM_ATTR_NAME);

		if(strlen(it->second->descr) > 0)
			saveAttr.push_back(ITEM_ATTR_DESCR);

		if(it->second->moveable || it->second->pickupable) {
			saveAttr.push_back(ITEM_ATTR_WEIGHT);
		}
		
		if(it->second->decayTo != 0) {
			saveAttr.push_back(ITEM_ATTR_DECAY2);
		}

		if(it->second->rotateTo != 0) {
			saveAttr.push_back(ITEM_ATTR_ROTATETO);
		}

#ifdef __SPRITE_SEARCH__

		if(it->second->id < 20000){
			saveAttr.push_back(ITEM_ATTR_SPRITEHASH);
			getImageHash(it->second->clientid, it->second->sprHash);
			it->second->miniMapColor = g_itemsSprites->getSprite(it->second->clientid)->miniMapColor;
			if(it->second->miniMapColor){
				saveAttr.push_back(ITEM_ATTR_MINIMAPCOLOR);
			}

			if(g_itemsSprites->getSprite(it->second->clientid)->subParam07 != 0){
				saveAttr.push_back(ITEM_ATTR_07);
				it->second->subParam07 = g_itemsSprites->getSprite(it->second->clientid)->subParam07;
			}

			if(g_itemsSprites->getSprite(it->second->clientid)->subParam08 != 0){
				saveAttr.push_back(ITEM_ATTR_08);
				it->second->subParam08 = g_itemsSprites->getSprite(it->second->clientid)->subParam08;
			}
			//if(g_itemsSprites->getSprite(it->second->clientid)->lightLevel != 0 || 
			//	g_itemsSprites->getSprite(it->second->clientid)->lightColor != 0){
			//	it->second->lightLevel = g_itemsSprites->getSprite(it->second->clientid)->lightLevel;
			//	it->second->lightColor = g_itemsSprites->getSprite(it->second->clientid)->lightColor;
			//	saveAttr.push_back(ITEM_ATTR_LIGHT2);
			//}
		}
#endif
		if(it->second->lightLevel != 0 || it->second->lightColor != 0) {
			saveAttr.push_back(ITEM_ATTR_LIGHT2);
		}

		switch(it->second->group) {
			case ITEM_GROUP_GROUND:
			{				
				saveAttr.push_back(ITEM_ATTR_SPEED);
				break;
			}			
			case ITEM_GROUP_CONTAINER:
			{
				saveAttr.push_back(ITEM_ATTR_SLOT);
				saveAttr.push_back(ITEM_ATTR_MAXITEMS);

				break;
			}
			case ITEM_GROUP_WEAPON:
			{				
				saveAttr.push_back(ITEM_ATTR_SLOT);
				saveAttr.push_back(ITEM_ATTR_WEAPON2);
				break;
			}

			case ITEM_GROUP_AMMUNITION:
			{
				saveAttr.push_back(ITEM_ATTR_SLOT);
				saveAttr.push_back(ITEM_ATTR_AMU2);
				break;
			}

			case ITEM_GROUP_ARMOR:
			{
				saveAttr.push_back(ITEM_ATTR_SLOT);
				saveAttr.push_back(ITEM_ATTR_ARMOR2);

				break;
			}

			case ITEM_GROUP_RUNE:
			{
				saveAttr.push_back(ITEM_ATTR_MAGLEVEL);
				break;
			}

			case ITEM_GROUP_TELEPORT:
			{
				break;
			}

			case ITEM_GROUP_MAGICFIELD:
			{
				saveAttr.push_back(ITEM_ATTR_MAGFIELDTYPE);

				break;
			}

			case ITEM_GROUP_WRITEABLE:
			{
				if(it->second->readOnlyId) {
					saveAttr.push_back(ITEM_ATTR_WRITEABLE2);
				}

				break;
			}

			case ITEM_GROUP_KEY:
			{
				break;
			}

			case ITEM_GROUP_SPLASH:
			{
				break;
			}

			case ITEM_GROUP_FLUID:
			{
				break;
			}

			case ITEM_GROUP_NONE:
			{
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
		
		if(it->second->canNotDecay)
			flags |= FLAG_CANNOTDECAY;

		if(it->second->useable)
			flags |= FLAG_USEABLE;
		
		if(it->second->pickupable)
			flags |= FLAG_PICKUPABLE;
		
		if(it->second->moveable)
			flags |= FLAG_MOVEABLE;
		
		if(it->second->stackable)
			flags |= FLAG_STACKABLE;
		
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
		
		if(it->second->alwaysOnTop)
			flags |= FLAG_ALWAYSONTOP;
		
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
				case ITEM_ATTR_NAME:
				{
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));
					break;
				}
				case ITEM_ATTR_DESCR:
				{
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
					break;
				}
				case ITEM_ATTR_SPEED:
				{
					f->setProps(ITEM_ATTR_SPEED, &it->second->speed, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_SLOT:
				{
					f->setProps(ITEM_ATTR_SLOT, &it->second->slot_position, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_MAXITEMS:
				{
					f->setProps(ITEM_ATTR_MAXITEMS, &it->second->maxItems, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_WEIGHT:
				{
					f->setProps(ITEM_ATTR_WEIGHT, &it->second->weight, sizeof(double));
					break;
				}
				case ITEM_ATTR_WEAPON2:
				{
					weaponBlock2 wb2;
					wb2.weaponType = it->second->weaponType;
					wb2.shootType = it->second->shootType;
					wb2.amuType = it->second->amuType;
					wb2.attack = it->second->attack;
					wb2.defence = it->second->defence;
					f->setProps(ITEM_ATTR_WEAPON2, &wb2, sizeof(wb2));
					break;
				}
				case ITEM_ATTR_AMU2:
				{
					amuBlock2 ab2;
					ab2.shootType = it->second->shootType;
					ab2.amuType = it->second->amuType;
					ab2.attack = it->second->attack;
					f->setProps(ITEM_ATTR_AMU2, &ab2, sizeof(ab2));
					break;
				}
				case ITEM_ATTR_ARMOR2:
				{
					armorBlock2 ab2;
					ab2.armor = it->second->armor;
					ab2.slot_position = it->second->slot_position;
					ab2.weight = it->second->weight;
					f->setProps(ITEM_ATTR_ARMOR2, &ab2, sizeof(ab2));
					break;
				}
				case ITEM_ATTR_MAGLEVEL:
				{
					f->setProps(ITEM_ATTR_MAGLEVEL, &it->second->runeMagLevel, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_MAGFIELDTYPE:
				{
					f->setProps(ITEM_ATTR_MAGFIELDTYPE, &it->second->magicfieldtype, sizeof(unsigned char));
					break;
				}
				case ITEM_ATTR_WRITEABLE2:
				{
					struct writeableBlock2 wb2;
					wb2.readOnlyId = it->second->readOnlyId;
					f->setProps(ITEM_ATTR_WRITEABLE2, &wb2, sizeof(wb2));
					break;
				}
				case ITEM_ATTR_ROTATETO:
				{
					f->setProps(ITEM_ATTR_ROTATETO, &it->second->rotateTo, sizeof(unsigned short));
					break;
				}
				case ITEM_ATTR_DECAY2:
				{
					struct decayBlock2 db2;
					db2.decayTo = it->second->decayTo;
					db2.decayTime = it->second->decayTime;
					f->setProps(ITEM_ATTR_DECAY2, &db2, sizeof(db2));
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
			}
		}

		f->endNode();
	}

	f->endNode();

	delete f;
	f = NULL;
	return 1;
}
