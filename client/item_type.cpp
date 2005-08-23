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

const char* readXmlProp(const char* propName, const xmlProp *props);
extern ItemsTypes *g_itemsTypes;

long ItemType::minClientId = 0;
long ItemType::maxClientId = 0;

ItemType::ItemType()
{
	group     = ITEM_GROUP_NONE;
	id			  = 100;
	clientid	= 100;

	blockSolid	= false;
	blockProjectile = false;
	blockPickupable = false;
	blockPathFind = false;

	//container	= false;
	//isammo = false;
	//groundtile	= false;
	//fluid = false;

	alwaysOnTop	= false;
	stackable	= false;
	useable		= false;
	moveable	= true;
	pickupable	= false;
	rotable = false;
	readable = false;
	
	rotateTo = 0;

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
			sType->blockPickupable = true; //default for .dat

			sType->id = id;
			sType->clientid = id;
			addType(id, sType);
		}

		// read the options until we find a 0xff
		int optbyte;
		
		while (((optbyte = fgetc(fp)) >= 0) && (optbyte != 0xFF))
		{                                                            
			switch (optbyte)
			{
			case 0x00:
				//is groundtile
				fread(&read_short, 2, 1, fp); 
				speed = read_short;
				sType->speed = speed;
				sType->group = ITEM_GROUP_GROUND;
				break;

			case 0x01: // all OnTop
				sType->alwaysOnTop = true;
				break;
			case 0x02: // can walk trough (open doors, arces, bug pen fence ??)
				break;
			case 0x03:
				//is a container
				sType->group = ITEM_GROUP_CONTAINER;
				break;
			case 0x04:
				//is stackable
				sType->stackable = true;
				break;
			case 0x05:
				//is useable
				sType->useable = true;
				break;
			case 0x0A:
				sType->group = ITEM_GROUP_SPLASH;
				break;
			case 0x0B:
				//is blocking
				sType->blockSolid = true;
				break;
			case 0x0C:
				//is not moveable
				sType->moveable = false;
				break;
			case 0x0F:
				//can be equipped
				sType->pickupable = true;
				break;
			case 0x10:
				//makes light (skip 4 bytes)
				fgetc(fp); //number of tiles around
				fgetc(fp); // always 0
				fgetc(fp); // 215 items, 208 fe non existant items other values
				fgetc(fp); // always 0
				break;
			case 0x06: // ladder up (id 1386)   why a group for just 1 item ???   
				break;
			case 0x09: //can contain fluids
				sType->group = ITEM_GROUP_FLUID;
				break;
			case 0x0D: // blocks missiles (walls, magic wall etc)
				sType->blockProjectile = true;
				break;
			case 0x0E: // blocks monster movement (flowers, parcels etc)
				sType->blockPathFind = true;
				break;
			case 0x11: // can see what is under (ladder holes, stairs holes etc)
				break;
			case 0x12: // ground tiles that don't cause level change
				//sType->floorchange = false;
				break;
			case 0x18: // cropses that don't decay
				break;
			case 0x14: // player color templates
				break;
			case 0x07: // writtable objects
				sType->group = ITEM_GROUP_WRITEABLE;
				sType->readable = true;
				fgetc(fp); //max characters that can be written in it (0 unlimited)
				fgetc(fp); //max number of  newlines ? 0, 2, 4, 7
				break;
			case 0x08: // writtable objects that can't be edited
				sType->readable = true;
				fgetc(fp); //always 0 max characters that can be written in it (0 unlimited) 
				fgetc(fp); //always 4 max number of  newlines ? 
				break;
			case 0x13: // mostly blocking items, but also items that can pile up in level (boxes, chairs etc)
				sType->blockPickupable = false;
				fgetc(fp); //always 8
				fgetc(fp); //always 0
				break;
			case 0x16: // ground, blocking items and mayby some more 
				fgetc(fp); //12, 186, 210, 129 and other.. 
				fgetc(fp); //always 0
				break;
			case 0x1A: 
				//7.4 (change no data ?? ) action that can be performed (doors-> open, hole->open, book->read) not all included ex. wall torches
				break;  
			case 0x1D:  // line spot ...
				int tmp;
				tmp = fgetc(fp); // 86 -> openable holes, 77-> can be used to go down, 76 can be used to go up, 82 -> stairs up, 79 switch,    
				if(tmp == 0x58)
					sType->readable = true;
				fgetc(fp); // always 4                  
				break;         
			case 0x1B:  // walls 2 types of them same material (total 4 pairs)                  
				break;
			case 0x19:  // wall items                 
				break;    
			case 0x17:  // seems like decorables with 4 states of turning (exception first 4 are unique statues)                 
				sType->rotable = true;
				break;
			case 0x1C:  // ?? ...                 
				break;            
			default:
				//std::cout << "unknown byte: " << (unsigned short)optbyte << std::endl;
				return false;
				break;
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


int ItemsTypes::loadOtb(const char *filename)
{
	ItemLoader *f = new ItemLoader();
	if(!f->openFile(filename, false)) {
		return f->getError();
	}
	
	unsigned long type,len;
	NODE node = f->getChildNode(NULL, type);
	//f->getProps(node, len);
	node = f->getChildNode(node, type);

	const unsigned char* data;
	while(node != NO_NODE) {
		data = f->getProps(node, len);
		if(data == NULL && f->getError() != ERROR_NONE)
			return f->getError();
		
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
							sType->blockPickupable = ((flags & FLAG_BLOCK_PICKUPABLE) == FLAG_BLOCK_PICKUPABLE);
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

								memcpy(&sType->id, p, sizeof(datalen));
								break;
							}

							case ITEM_ATTR_CLIENTID:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->clientid, p, sizeof(datalen));
								break;
							}
							case ITEM_ATTR_NAME:
							{
								if(datalen > sizeof(sType->name))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->name[0], p, datalen);
								break;
							}
							case ITEM_ATTR_DESCR:
							{
								if(datalen > sizeof(sType->descr))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->descr[0], p, datalen);
								break;
							}
							case ITEM_ATTR_SPEED:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->speed, p, sizeof(datalen));
								break;
							}
							case ITEM_ATTR_SLOT:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->slot_position, p, sizeof(datalen));
								break;
							}
							case ITEM_ATTR_MAXITEMS:
							{
								if(datalen != sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->maxItems, p, sizeof(datalen));
								break;
							}
							case ITEM_ATTR_WEIGHT:
							{
								if(datalen != sizeof(double))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->weight, p, sizeof(datalen));
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
								if(datalen < sizeof(armorBlock))
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
								if(datalen < sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;
								
								memcpy(&sType->runeMagLevel, p, sizeof(datalen));
								
								break;
							}
							case ITEM_ATTR_MAGFIELDTYPE:
							{
								if(datalen < sizeof(unsigned char))
									return ERROR_INVALID_FORMAT;
								
								memcpy(&sType->magicfieldtype, p, sizeof(datalen));

								break;
							}
							case ITEM_ATTR_WRITEABLE:
							{
								if(datalen < sizeof(writeableBlock))
									return ERROR_INVALID_FORMAT;

								struct writeableBlock wb;
								memcpy(&wb, p, sizeof(datalen));

								sType->readOnlyId = wb.readOnlyId;

								break;
							}
							case ITEM_ATTR_ROTATETO:
							{
								if(datalen < sizeof(unsigned short))
									return ERROR_INVALID_FORMAT;

								memcpy(&sType->rotateTo, p, sizeof(datalen));

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

		node = f->getNextNode(node, type);
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

	ItemMap::iterator it;
	for(it = item.begin(); it != item.end(); it++){
		f->startNode(it->second->group);

		flags_t flags = 0;

		switch(it->second->group) {
			case ITEM_GROUP_GROUND:
			{
				if(it->second->blockSolid)
					flags |= FLAG_BLOCK_SOLID;

				if(it->second->blockProjectile)
					flags |= FLAG_BLOCK_PROJECTILE;

				if(it->second->blockPathFind)
					flags |= FLAG_BLOCK_PATHFIND;

				if(it->second->blockPickupable)
					flags |= FLAG_BLOCK_PICKUPABLE;

				if(it->second->useable)
					flags |= FLAG_USEABLE;

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

				f->setFlags(flags);
				
				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				f->setProps(ITEM_ATTR_SPEED, &it->second->speed, sizeof(unsigned short));

				if(it->second->decayTo != 0) {
					struct decayBlock db;
					db.decayTo = it->second->decayTo;
					db.decayTime = it->second->decayTime;
					f->setProps(ITEM_ATTR_DECAY, &db, sizeof(db));
				}
				
				break;
			}			
			case ITEM_GROUP_CONTAINER:
			{
				if(it->second->useable)
					flags |= FLAG_USEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				f->setProps(ITEM_ATTR_WEIGHT, &it->second->weight, sizeof(double));
				f->setProps(ITEM_ATTR_MAXITEMS, &it->second->maxItems, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_SLOT, &it->second->slot_position, sizeof(unsigned short));

				if(it->second->decayTo != 0) {
					struct decayBlock db;
					db.decayTo = it->second->decayTo;
					db.decayTime = it->second->decayTime;
					f->setProps(ITEM_ATTR_DECAY, &db, sizeof(db));
				}

				break;
			}
			case ITEM_GROUP_WEAPON:
			{
				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->stackable)
					flags |= FLAG_STACKABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				f->setProps(ITEM_ATTR_WEIGHT, &it->second->weight, sizeof(double));
				f->setProps(ITEM_ATTR_SLOT, &it->second->slot_position, sizeof(unsigned short));

				weaponBlock wb;
				wb.weaponType = it->second->weaponType;
				wb.shootType = it->second->shootType;
				wb.amuType = it->second->amuType;
				wb.attack = it->second->attack;
				wb.defence = it->second->defence;
				f->setProps(ITEM_ATTR_WEAPON, &wb, sizeof(wb));

				break;
			}

			case ITEM_GROUP_AMMUNITION:
			{
				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->stackable)
					flags |= FLAG_STACKABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				f->setProps(ITEM_ATTR_WEIGHT, &it->second->weight, sizeof(double));
				f->setProps(ITEM_ATTR_SLOT, &it->second->slot_position, sizeof(unsigned short));

				amuBlock ab;
				ab.shootType = it->second->shootType;
				ab.amuType = it->second->amuType;
				ab.attack = it->second->attack;
				f->setProps(ITEM_ATTR_AMU, &ab, sizeof(ab));

				break;
			}

			case ITEM_GROUP_ARMOR:
			{
				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				armorBlock ab;
				ab.armor = it->second->armor;
				ab.slot_position = it->second->slot_position;
				ab.weight = it->second->weight;
				f->setProps(ITEM_ATTR_ARMOR, &ab, sizeof(armorBlock));
				
				break;
			}

			case ITEM_GROUP_RUNE:
			{
				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				f->setProps(ITEM_ATTR_MAGLEVEL, &it->second->runeMagLevel, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_WEIGHT, &it->second->weight, sizeof(double));

				break;
			}

			case ITEM_GROUP_TELEPORT:
			{
				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				break;
			}

			case ITEM_GROUP_MAGICFIELD:
			{
				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//
				
				f->setProps(ITEM_ATTR_MAGFIELDTYPE, &it->second->magicfieldtype, sizeof(unsigned char));
				break;
			}

			case ITEM_GROUP_WRITEABLE:
			{
				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				if(it->second->readOnlyId) {
					struct writeableBlock wb;
					wb.readOnlyId = it->second->readOnlyId;

					f->setProps(ITEM_ATTR_WRITEABLE, &wb, sizeof(wb));
				}

				break;
			}

			case ITEM_GROUP_KEY:
			{
				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->pickupable)
					flags |= FLAG_PICKUPABLE;

				if(it->second->moveable)
					flags |= FLAG_MOVEABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				break;
			}

			case ITEM_GROUP_SPLASH:
			{
				if(it->second->blockSolid)
					flags |= FLAG_BLOCK_SOLID;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				if(it->second->decayTo != 0) {
					struct decayBlock db;
					db.decayTo = it->second->decayTo;
					db.decayTime = it->second->decayTime;
					f->setProps(ITEM_ATTR_DECAY, &db, sizeof(db));
				}

				break;
			}

			case ITEM_GROUP_FLUID:
			{
				if(it->second->blockSolid)
					flags |= FLAG_BLOCK_SOLID;

				if(it->second->useable)
					flags |= FLAG_MOVEABLE;

				if(it->second->readable)
					flags |= FLAG_READABLE;

				f->setFlags(flags);

				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				if(it->second->decayTo != 0) {
					struct decayBlock db;
					db.decayTo = it->second->decayTo;
					db.decayTime = it->second->decayTime;
					f->setProps(ITEM_ATTR_DECAY, &db, sizeof(db));
				}

				break;
			}

			case ITEM_GROUP_NONE:
			{
				if(it->second->blockSolid)
					flags |= FLAG_BLOCK_SOLID;

				if(it->second->blockProjectile)
					flags |= FLAG_BLOCK_PROJECTILE;

				if(it->second->blockPathFind)
					flags |= FLAG_BLOCK_PATHFIND;

				if(it->second->blockPickupable)
					flags |= FLAG_BLOCK_PICKUPABLE;

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

				f->setFlags(flags);
				
				//
				f->setProps(ITEM_ATTR_SERVERID, &it->second->id, sizeof(unsigned short));
				f->setProps(ITEM_ATTR_CLIENTID, &it->second->clientid, sizeof(unsigned short));

				if(strlen(it->second->name) > 0)
					f->setProps(ITEM_ATTR_NAME, &it->second->name, strlen(it->second->name));

				if(strlen(it->second->descr) > 0)
					f->setProps(ITEM_ATTR_DESCR, &it->second->descr, strlen(it->second->descr));
				//

				if(it->second->moveable || it->second->pickupable) {
					f->setProps(ITEM_ATTR_WEIGHT, &it->second->weight, sizeof(double));
				}

				if(it->second->rotateTo != 0) {
					f->setProps(ITEM_ATTR_ROTATETO, &it->second->rotateTo, sizeof(unsigned short));
				}

				if(it->second->decayTo != 0) {
					struct decayBlock db;
					db.decayTo = it->second->decayTo;
					db.decayTime = it->second->decayTime;
					f->setProps(ITEM_ATTR_DECAY, &db, sizeof(db));
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
