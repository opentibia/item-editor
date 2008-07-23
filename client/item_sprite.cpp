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
extern void getImageHash(unsigned short cid, void* output);

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
	memset(sprHash, '\0', sizeof(sprHash));	
	group = ITEM_GROUP_NONE;
	blockSolid = false;
	hasHeight = false;
	blockPathFind = false;
	blockProjectile = false;
	alwaysOnTop = false;
	alwaysOnTopOrder = 0;
	stackable = false;
	useable = false;
	moveable = true;
	pickupable = false;
	rotable = false;
	readable = false;
	
	speed = 0;

	lightLevel = 0;
	lightColor = 0;

	isVertical = false;
	isHorizontal = false;

	isHangable = false;

	miniMapColor = 0;
	
	subParam07 = 0;
	subParam08 = 0;

	height = 32;
	width = 32;
	blendframes = 0;
	xdiv = 1;
	ydiv = 1;
	animcount = 1;
	numsprites = 0;
	
	imageID = NULL;
}

bool SpriteType::compareOptions(const SpriteType *stype)
{
	if(!stype)
		return false;

	switch(stype->group){
	case ITEM_GROUP_GROUND:
	case ITEM_GROUP_CONTAINER:
	case ITEM_GROUP_SPLASH:
	case ITEM_GROUP_FLUID:
	case ITEM_GROUP_WRITEABLE:
	case ITEM_GROUP_CHARGES:
		if(group != stype->group)
			return false;
	}
	
	if(blockSolid != stype->blockSolid)
		return false;
	
	if(blockPathFind != stype->blockPathFind)
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

	if(readable != stype->readable)
		return false;

	if(clientCharges != stype->clientCharges)
		return false;

	//if(alwaysOnTop != stype->alwaysOnTop)
	//	return false;

	//if(alwaysOnTopOrder != stype->alwaysOnTopOrder)
	//	return false;
	
	return true;
}

ItemsSprites::ItemsSprites()
{
	datLoaded = false;
	sprLoaded = false;

	datVersion = 0;
	sprVersion = 0;
}

ItemsSprites::~ItemsSprites()
{
	versionVector::iterator it;
	for(it = versions.begin(); it != versions.end(); ++it){
		delete *it;
	}
}


bool ItemsSprites::loadKnownVersions(const char* filename)
{
	graphicsVersion* version;
	FILE* f = fopen(filename, "r");
	if(!f)
		return false;

	while(!feof(f)){
		version = new graphicsVersion;
		fscanf(f, "%x %x %d %d\n", &version->dat,  &version->spr, &version->client, &version->otb);
		versions.push_back(version);
	}
	fclose(f);
	return true;
}

const graphicsVersion* ItemsSprites::getVersion()
{
	versionVector::iterator it;
	for(it = versions.begin(); it != versions.end(); ++it){
		if((*it)->spr == sprVersion && (*it)->dat == datVersion){
			return (*it);
		}
	}
	return NULL;
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
	

	fseek(fp, 0, SEEK_SET);
	fread(&sprVersion, 4, 1, fp);
	const graphicsVersion* version = getVersion();
	if(!version){
		return false;
	}

	// At the begining of the spr pointers
	fseek(fp, 0x06, SEEK_SET);
	
	// Read the first sprite image pointer
	fread(&first_spr_pos, 1, 4, fp);
	
	// At the begining of the spr pointers
	fseek(fp, 0x06, SEEK_SET);
	
	for(id=1; ftell(fp) < first_spr_pos; ++id)
	{
		// Read the sprite image pointer
		fread(&spr_pos, 1, 4, fp);
		if(spr_pos == 0)
			continue;

		// Now the spr reading
		fseek(fp_spr, spr_pos + 3, SEEK_SET);
		fread(&spr_size, 1, 2, fp_spr);


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

bool ItemsSprites::loadFromDat(const char *filename)
{
	if(datLoaded)
		return false;
	
	unsigned short id = 100;  // tibia.dat start with id 100
	FILE *fp;
	long size;
	int speed;
	short read_short;
	unsigned short us;

	fp = fopen(filename, "rb");
	if(!fp)
		return false;
	
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	
	fseek(fp, 0, SEEK_SET);
	fread(&datVersion, 4, 1, fp);
	//get max id
	fseek(fp, 0x04, SEEK_SET);
	fread(&read_short, 2, 1, fp);
	SpriteType::maxClientId = read_short;
	SpriteType::minClientId = 100;

	fread(&read_short, 2, 1, fp);
	unsigned short monsters = read_short;
	fread(&read_short, 2, 1, fp);
	unsigned short effects = read_short;
	fread(&read_short, 2, 1, fp);
	unsigned short distance = read_short;

	//SpriteType::maxClientId = SpriteType::maxClientId + monsters + effects + distance;

	// loop throw all Items until we reach the end of file
	while(ftell(fp) < size && id <= SpriteType::maxClientId){
		SpriteType *sType = new SpriteType();
		sType->id = id;
		// read the options until we find a 0xff
		int optbyte;

		while(((optbyte = fgetc(fp)) >= 0) && (optbyte != 0xFF)){
			switch(optbyte){
			case 0x00: //is groundtile
				fread(&read_short, 2, 1, fp);
				speed = read_short;
				sType->speed = speed;
				sType->group = ITEM_GROUP_GROUND;
				break;
			case 0x01: //all OnTop
				sType->alwaysOnTop = true;
				sType->alwaysOnTopOrder = 1;
				break;
			case 0x02: //can walk trough (open doors, arces, bug pen fence ??)
				sType->alwaysOnTop = true;
				sType->alwaysOnTopOrder = 2;
				break;
			case 0x03: //can walk trough (arces)
				sType->alwaysOnTop = true;
				sType->alwaysOnTopOrder = 3;
				break;
			case 0x04: //is a container
				sType->group = ITEM_GROUP_CONTAINER;
				break;
			case 0x05: //is stackable
				sType->stackable = true;
				break;
			case 0x06: //ladders
				break;
			case 0x07: //is useable
				sType->useable = true;
				break;
			case 0x08: //charges
				//sType->group = ITEM_GROUP_CHARGES;
				sType->clientCharges = true;
				break;
			case 0x09: //writtable objects
				sType->group = ITEM_GROUP_WRITEABLE;
				sType->readable = true;
				fread(&us, sizeof(us), 1, fp); //unknown, values like 80, 200, 512, 1024, 2000
				sType->subParam07 = us;
				break;
			case 0x0A: //writtable objects that can't be edited 
				sType->readable = true;
				fread(&us, sizeof(us), 1, fp); //unknown, all have the value 1024
				sType->subParam08 = us;
				break;
			case 0x0B: //can contain fluids
				sType->group = ITEM_GROUP_FLUID;
				break;
			case 0x0C: //liquid with states 
				sType->group = ITEM_GROUP_SPLASH;
				break;
			case 0x0D: //is blocking
				sType->blockSolid = true;
				break;
			case 0x0E: //is no moveable
				sType->moveable = false;
				break;
			case 0x0F: //blocks missiles (walls, magic wall etc)
				sType->blockProjectile = true;
				break;
			case 0x10: //blocks monster movement (flowers, parcels etc)
				sType->blockPathFind = true;
				break;
			case 0x11: //can be equipped
				sType->pickupable = true;
				break;
			case 0x12: //wall items
				sType->isHangable = true;
				break;
			case 0x13:
				sType->isHorizontal = true;
				break;
			case 0x14:
				sType->isVertical = true;
				break;
			case 0x15: //rotable items
				sType->rotable = true;
				break;
			case 0x16: //light info .. //sprite-drawing related
				unsigned short lightlevel;
				fread(&lightlevel, sizeof(lightlevel), 1, fp);
				sType->lightLevel = lightlevel;
				unsigned short lightcolor;
				fread(&lightcolor, sizeof(lightcolor), 1, fp);
				sType->lightColor = lightcolor;
				break;
			case 0x17:  //floor change 
				break;
			case 0x18:
				optbyte = optbyte;
				break;
			case 0x19: //???
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				break;
			case 0x1A:
				sType->hasHeight = true;
				fgetc(fp); //always 8
				fgetc(fp); //always 0
				break;
			case 0x1B://draw with height offset for all parts (2x2) of the sprite
				break;
			case 0x1C://some monsters
				break;
			case 0x1D:
				unsigned short color;
				fread(&color, sizeof(color), 1, fp);
				sType->miniMapColor = color;
				break;
			case 0x1E:  //line spot
				int tmp;
				tmp = fgetc(fp); // 86 -> openable holes, 77-> can be used to go down, 76 can be used to go up, 82 -> stairs up, 79 switch,    
				if(tmp == 0x58)
					sType->readable = true;
				fgetc(fp); // always 4
				break;
			case 0x1F:
				break;
			default:
				optbyte = optbyte;
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
		unsigned char unk1 = fgetc(fp);
		sType->animcount   = fgetc(fp);

		sType->numsprites = sType->width * sType->height * sType->blendframes * sType->xdiv * sType->ydiv * sType->animcount * unk1;

		// Dynamic memoy reserve
		sType->imageID = new unsigned short[sType->numsprites];
		
		// Read the sprite ids
		for(int i = 0; i < sType->numsprites; ++i) {
			fread(&sType->imageID[i], sizeof(unsigned short), 1, fp);

			// Sprite added to the SpriteMap
			if(i < sType->width * sType->height * sType->blendframes || sType->group == ITEM_GROUP_FLUID){
				Sprite *newSprite = new Sprite();
				newSprite->id = sType->imageID[i];
				sprite[sType->imageID[i]] = newSprite;
			}
		}
		
		// store the found item
		item[id] = sType;
		id++;
	}
	fclose(fp);
	datLoaded = true;
	
	return true;
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

//const SpriteType& ItemsSprites::operator[](int id)
SpriteType* ItemsSprites::getSprite(int id)
{
	ItemMap::iterator it = item.find(id);
	if ((it != item.end()) && (it->second != NULL))
		return it->second;
	   
	return NULL;
}

bool ItemsSprites::loadHash()
{
	ItemMap::iterator it;
	for(it = item.begin(); it != item.end();it++)
	{
		getImageHash(it->second->id, it->second->sprHash);
	}
	return true;
}
