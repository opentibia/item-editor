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


#include "item_sprite.hpp"
#include "gui.hpp"
#include <iostream>

extern GUI *g_gui;
extern ItemsSprites *g_itemsSprites;

const char* readXmlProp(const char* propName, const xmlProp *props);

Sprite::Sprite()
{
	id = 0;
	size = 0;
	dump = NULL;

	internal = NULL;
}


long SpriteType::minClientId = 0;
long SpriteType::maxClientId = 0;

SpriteType::SpriteType()
{
	id = 0;
	
	groundtile = false;
	blocking = false;
	alwaysOnTop = false;
	container = false;
	stackable = false;
	useable = false;
	notMoveable = false;
	pickupable = false;
	fluid = false;
	rotable = false;
	speed = 0;

	height = 32;
	width = 32;
	blendframes = 0;
	xdiv = 1;
	ydiv = 1;
	animcount = 1;
	numsprites = 0;
	
	imageID = NULL;

	//xml
	name = "";
	descr = "";
	weight = 0.00;
  decayTo = 0;
  decayTime = 0;
	blockingProjectile = false;
	floorchange = false;
	slot_position = SLOT_HAND;

	floorChangeNorth = false;
	floorChangeSouth = false;
	floorChangeEast = false;
	floorChangeWest = false;

	//container
	maxItems = 20;

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

	//teleport
	isteleport = false;

	//magicfield
	ismagicfield = false;
	magicfieldtype = 0;

	//writeable
	readonlyId = 0;

	//key
	iskey = false;

	//splash
	issplash = false;
}


ItemsSprites::ItemsSprites()
{
	datLoaded = false;
	sprLoaded = false;
}

void ItemsSprites::unloadSpr()
{
	// Freeing dynamic memory
	SpriteMap::iterator it;
	for(it = sprite.begin(); it != sprite.end(); )
	{
		if(it->second->dump != NULL)
		{
			free(it->second->dump);
			it->second->dump = NULL;
		}
		g_gui->unloadSpriteInternal(it->second->internal);
		it->second->size = 0;
		delete it->second;
		sprite.erase(it++);
	}
	
	sprLoaded = false;
}

void ItemsSprites::unloadDat()
{
	// Freeing dynamic memory
	if(sprLoaded)
		unloadSpr();
	
	ItemMap::iterator it;
	for(it = item.begin(); it != item.end();)
	{
		if(it->second->imageID != NULL)
		{
			free(it->second->imageID);
			it->second->imageID = NULL;
		}
		delete it->second;
		item.erase(it++);
	}
	
	datLoaded = false;
}


bool ItemsSprites::loadFromSpr(const char *filename)
{
	if(sprLoaded || !datLoaded)
		return false;
	
	FILE *fp, *fp_spr;
	long spr_pos, first_spr_pos;
	char buf[6];
	unsigned short spr_size;
	unsigned short id;
	unsigned long loaded_pics = 0;
	unsigned long total_pics = 0;
	
	fp = fopen(filename, "rb");
	if(!fp)
		return false;
	
	fp_spr = fopen(filename, "rb");
	if(!fp_spr)
	{
		fclose(fp);
		return false;
	}
	
	// At the begining of the spr pointers
	fseek(fp, 0x06, SEEK_SET);
	
	// Read the first sprite image pointer
	fread(buf, 4, 1, fp);
	memcpy(&first_spr_pos, buf, 4);
	
	// At the begining of the spr pointers
	fseek(fp, 0x06, SEEK_SET);
	
	for(id=1; ftell(fp) < first_spr_pos; ++id)
	{
		// Read the sprite image pointer
		fread(buf, 4, 1, fp);
		memcpy(&spr_pos, buf, 4);
		if(spr_pos == 0)
			continue;

		// Now the spr reading
		fread(buf, 4, 1, fp_spr);
		fseek(fp_spr, spr_pos, SEEK_SET);
		fread(buf, 5, 1, fp_spr);
		memcpy(&spr_size, buf+3, 2);


		SpriteMap::iterator it = sprite.find(id);
		if(it != sprite.end()){
			Sprite *spr = it->second;
			spr->size = spr_size;
			if(spr_size){
				spr->dump = new unsigned char[spr_size]; 
				fread(spr->dump, spr_size, 1, fp_spr);
				if(g_gui->loadSpriteInternal(spr->dump, spr_size, &spr->internal)){
					delete spr->dump;
					spr->dump = NULL;
				}
			}
			loaded_pics++;
		}
		else{
			fseek(fp_spr, spr_size, SEEK_CUR);
		}
		total_pics++;
		//std::cout << ">> " << std::hex << spr_pos << " | " << std::dec << (int)spr_size << std::endl;
	}
	fclose(fp_spr);
	fclose(fp);
	
	sprLoaded = true;
	
	return true;
}

// ---------- THIS FUNCTION IS FROM OTSERV PROJECT (Thx)--------------
bool ItemsSprites::loadFromDat(const char *filename)
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
	SpriteType::maxClientId = read_short;
	SpriteType::minClientId = 100;

	fseek(fp, 0x0C, SEEK_SET);
	// loop throw all Items until we reach the end of file
	while(ftell(fp) < size && id <= SpriteType::maxClientId)
	{
		SpriteType *sType = new SpriteType();
		sType->id = id;
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
				sType->groundtile = true;
				if(speed == 0)
					sType->blocking = true;

				break;
			case 0x01: // all OnTop
				sType->alwaysOnTop = true;
				break;
			case 0x02: // can walk trough (open doors, arces, bug pen fence ??)
				break;
			case 0x03:
				//is a container
				sType->container = true;
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
				//is multitype !!! wrong definition (only water splash on floor)
				sType->fluid = true;
				break;
			case 0x0B:
				//is blocking
				sType->blocking = true;
				break;
			case 0x0C:
				//is on moveable
				sType->notMoveable = true;
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
				sType->fluid = true;
				break;
			case 0x0D: // blocks missiles (walls, magic wall etc)
				//iType->blockingProjectile = true;
				break;
			case 0x0E: // blocks monster movement (flowers, parcels etc)
				break;
			case 0x11: // can see what is under (ladder holes, stairs holes etc)
				break;
			case 0x12: // ground tiles that don't cause level change
				//iType->noFloorChange = true;
				break;
			case 0x18: // cropses that don't decay
				break;
			case 0x14: // player color templates
				break;
			case 0x07: // writtable objects
				fgetc(fp); //max characters that can be written in it (0 unlimited)
				fgetc(fp); //max number of  newlines ? 0, 2, 4, 7
				break;
			case 0x08: // writtable objects that can't be edited 
				fgetc(fp); //always 0 max characters that can be written in it (0 unlimited) 
				fgetc(fp); //always 4 max number of  newlines ? 
				break;
			case 0x13: // mostly blocking items, but also items that can pile up in level (boxes, chairs etc)
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
				fgetc(fp); // 86 -> openable holes, 77-> can be used to go down, 76 can be used to go up, 82 -> stairs up, 79 switch,    
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
		
		// Size and sprite data
		sType->width  = fgetc(fp);
		sType->height = fgetc(fp);
		if((sType->width > 1) || (sType->height > 1)){
			fgetc(fp);
		}
		
		sType->blendframes = fgetc(fp);
		sType->xdiv        = fgetc(fp);
		sType->ydiv        = fgetc(fp);
		sType->animcount   = fgetc(fp);
		
		sType->numsprites = sType->width * sType->height * sType->blendframes * sType->xdiv * sType->ydiv * sType->animcount;
		// Dynamic memoy reserve
		sType->imageID = new unsigned short[sType->numsprites];
		
		// Read the sprite ids
		for(int i = 0; i < sType->numsprites; ++i) {
			fread(&sType->imageID[i], sizeof(unsigned short), 1, fp);
			Sprite *newSprite = new Sprite();
			/*if(id == 460){
				g_gui->loadSpriteInternalTransparent(0xFF0000,&newSprite->internal);
				sType->imageID[i] = 0xF000;
			}
			else if(id == 459){
				g_gui->loadSpriteInternalTransparent(0xFFFF00,&newSprite->internal);
				sType->imageID[i] = 0xF001;
			}*/
			newSprite->id = sType->imageID[i];
			// Sprite added to the SpriteMap
			if(i < sType->width * sType->height * sType->blendframes)
				sprite[sType->imageID[i]] = newSprite;
			else
				delete newSprite;
		}
		
		// store the found item
		item[id] = sType;
		id++;
	}

	fclose(fp);
	
	datLoaded = true;
	
	return true;
}

int ItemsSprites::loadstatus;
bool ItemsSprites::loadFromXml(const char *filename)
{
	void *buff;
	int bytes_read;
	int eof;
	//SpriteType xml_sType;
	//xml_sType.id = 0;
	XML_Parser p = XML_ParserCreate(NULL);

	eof = 0;
	if(!p){
		return false;
	}

	XML_SetElementHandler(p, xmlstartNode, xmlendNode);
	//XML_SetUserData(p, &xml_sType);

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

void XMLCALL ItemsSprites::xmlstartNode(void *userData, const char *name, const char **atts)
{
	if(loadstatus == 1)
		return;
	
	const xmlProp *props = (const xmlProp *)atts;

	if(strcmp(name, "item") == 0){
		int id;
		const char* tmp;
		SpriteType* sType = NULL;

		if((tmp = readXmlProp("id", props)) != 0){
			id = atoi(tmp);
			sType = g_itemsSprites->getItem(id);
		}
		else
			loadstatus = 1;

		if(sType) {
			if((tmp = readXmlProp("name", props)) != 0){
				sType->name = tmp;
			}

			if((tmp = readXmlProp("descr", props)) != 0){
				sType->descr = tmp;
			}

			if((tmp = readXmlProp("weight", props)) != 0){
				sType->weight = atof(tmp);
			}

			if((tmp = readXmlProp("decayTo", props)) != 0){
				sType->decayTo = atoi(tmp);
			}

			if((tmp = readXmlProp("decayTime", props)) != 0){
				sType->decayTime = atoi(tmp);
			}

			if((tmp = readXmlProp("blockingprojectile", props)) != 0){
				sType->blockingProjectile = atoi(tmp);
			}

			if((tmp = readXmlProp("floorchange", props)) != 0){
				sType->floorchange = true;
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
					sType->slot_position = SLOT_NECKLACE;
				else if (!strcmp(tmp, "body"))
					sType->slot_position = SLOT_ARMOR;
				else if (!strcmp(tmp, "legs"))
					sType->slot_position = SLOT_LEGS;
				else if (!strcmp(tmp, "boots"))
					sType->slot_position = SLOT_FEET;
				else if (!strcmp(tmp, "ring"))
					sType->slot_position = SLOT_RING;
				else if (!strcmp(tmp, "backpack"))					 	
					sType->slot_position = SLOT_BACKPACK;
				/*else if (!strcmp(tmp, "weapon"))
					sType->slot_position = SLOT_HAND;*/
				/*else if (!strcmp(tmp, "twohand"))
					sType->slot_position = SLOTP_RIGHT | SLOTP_LEFT | SLOTP_TWO_HAND;*/
				/*else if (!strcmp(tmp, "hand"))
					sType->slot_position = SLOTP_RIGHT | SLOTP_LEFT | SLOTP_AMMO;*/
				
				//sType->slot_position |= SLOTP_LEFT | SLOTP_RIGHT | SLOTP_AMMO;
				sType->slot_position = SLOT_HAND;
			}
			
			// now set special properties...
			// first we check the type...
			const char* type;
			if((type = readXmlProp("type", props)) != 0) {

				//container
				if(strcmp(type, "container") == 0) {
					if((tmp = readXmlProp("maxitems", props)) != 0){
						sType->maxItems = atoi(tmp);
					}
				}
				else if(strcmp(type, "weapon") == 0) {
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
					sType->weaponType = WEAPON_AMO;
					
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
					if((tmp = readXmlProp("arm", props)) != 0) {
						sType->armor = atoi(tmp);
					}
					/*else
						std::cout << "missing arm tag for armor: " << id << std::endl;*/
				}
				//rune
				else if (!strcmp(type, "rune")) {
					if((tmp = readXmlProp("maglevel", props)) != 0) {
						sType->runeMagLevel = atoi(tmp);
					}
					/*else
						std::cout << "missing maglevel for rune: " << id << std::endl;*/
				}
				//teleport
				else if (!strcmp(type, "teleport")) {
					sType->isteleport = true;
				}
				//magicfield
				else if (!strcmp(type, "magicfield")) {
					sType->ismagicfield = true;

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
				//write1time
				else if(!strcmp(type, "write1time")) {
					if((tmp = readXmlProp("readonlyid", props)) != 0) {
						sType->readonlyId = atoi(tmp);
					}
				}
				//key
				else if(!strcmp(type, "key")) {
					sType->iskey = true;
				}
				//splash
				else if(!strcmp(type, "splash")) {
					sType->issplash = true;
				}
			}
		}
		/*else
			loadstatus = 1;*/
	}
}

void XMLCALL ItemsSprites::xmlendNode(void *userData, const char *name)
{
	/*
	SpriteType* pxml_sType = (SpriteType*)userData;
	pxml_sType->id = 0;
	*/
}

unsigned short ItemsSprites::getSpriteDump(unsigned short id, char *dump)
{
	SpriteMap::iterator it = sprite.find(id);
	
	if(it != sprite.end()) // Found
	{
		//dump = it->second->dump;
		memcpy(dump, it->second->dump, it->second->size);
		return it->second->size;
	}
	else // Not found
	{
		return 0; 
	}
}

unsigned short ItemsSprites::getItemSpriteID(unsigned short item_id, unsigned short frame )
{
	ItemMap::iterator it = item.find(item_id);
	
	if(it != item.end()) // Found
	{
		if(frame < it->second->numsprites)
			return it->second->imageID[frame];
		else
			return 0;
	}
	else
		return 0;
}

InternalSprite ItemsSprites::getSpriteInternalFormat(unsigned short item_id, unsigned short frame)
{
	unsigned short spriteid = getItemSpriteID(item_id, frame);

	SpriteMap::iterator it = sprite.find(spriteid);
	
	if(it != sprite.end()) // Found
	{
		return it->second->internal;
	}
	else // Not found
	{
		return 0; 
	}	
}

SpriteType* ItemsSprites::getItem(int id)
{
	ItemMap::iterator it = item.find(id);
	if ((it != item.end()) && (it->second != NULL))
		return it->second;
	   
	return NULL;
}

const SpriteType& ItemsSprites::operator[](int id)
{
	ItemMap::iterator it = item.find(id);
	if ((it != item.end()) && (it->second != NULL))
		return *it->second;
	   
	return m_dummy;
}
