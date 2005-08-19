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

ItemType::ItemType()
{
	group = ITEM_GROUP_NONE;
	id			= 100;
	clientid	= 100;
	groundtile	= false;
	blocking	= false;
	alwaysOnTop	= false;
	container	= false;
	stackable	= false;
	useable		= false;
	notMoveable	= false;
	pickupable	= false;
	fluid = false;
	rotable = false;
	isammo = false;
	
	rotateTo = 0;

	//xml
	name[0] = '\0';
	descr[0] = '\0'; 
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

	//ground
	speed = 0;

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
	write1time = false;
	
	//key
	iskey = false;
	
	//splash
	issplash = false;

}


int ItemsTypes::loadstatus;
bool ItemsTypes::loadFromXml(const char *filename)
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
			sType = g_itemsTypes->getItem(id);
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

			if((tmp = readXmlProp("decayTo", props)) != 0){
				sType->decayTo = atoi(tmp);
			}

			if((tmp = readXmlProp("decayTime", props)) != 0){
				sType->decayTime = atoi(tmp);
			}

			if((tmp = readXmlProp("blockingprojectile", props)) != 0){
				if(atoi(tmp) == 0)
					sType->blockingProjectile = false;
				else
					sType->blockingProjectile = true;
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
					sType->isammo = true;
					
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
						sType->write1time = true;
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

void XMLCALL ItemsTypes::xmlendNode(void *userData, const char *name)
{
	//
}

bool ItemsTypes::setGroup(int id, itemgroup_t newgroup)
{
	ItemType* it = getItem(id);
	if(it) {
		it->group = newgroup;
		return true;
	}

	return false;
}

ItemType* ItemsTypes::getItem(int id)
{
	ItemMap::iterator it = item.find(id);
	if ((it != item.end()) && (it->second != NULL))
		return it->second;
	   
	return NULL;
}
