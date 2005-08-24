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
	
	group = ITEM_GROUP_NONE;
	blockSolid = false;
	blockPickupable = true;
	blockPathFind = false;
	alwaysOnTop = false;
	stackable = false;
	useable = false;
	moveable = true;
	pickupable = false;
	rotable = false;
	readable = false;

	speed = 0;

	height = 32;
	width = 32;
	blendframes = 0;
	xdiv = 1;
	ydiv = 1;
	animcount = 1;
	numsprites = 0;
	
	imageID = NULL;
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
				//is no moveable
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
				//iType->noFloorChange = true;
				break;
			case 0x18: // cropses that don't decay
				break;
			case 0x14: // player color templates
				break;
			case 0x07: // writtable objects
				//sType->group = ITEM_GROUP_WRITEABLE;
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
