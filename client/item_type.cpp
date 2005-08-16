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


ItemType::ItemType()
{
	id			= 100;
	groundtile	= false;
	blocking	= false;
	alwaysOnTop	= false;
	container	= false;
	stackable	= false;
	useable		= false;
	notMoveable	= false;
	pickupable	= false;
	fluidcontainer = false;
	creature = false;
	
	
	height		= 1;
	width		= 1;
	blendframes	= 0;
	xdiv			= 1;
	ydiv			= 1;
	animcount		= 1;
	
	numsprites	= 0;
	imageID		= NULL;
}


Items::Items()
{
	datLoaded = false;
	sprLoaded = false;
}

void Items::unloadSpr()
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

void Items::unloadDat()
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


bool Items::loadFromSpr(const char *filename)
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
bool Items::loadFromDat(const char *filename)
{
	if(datLoaded)
		return false;
	
	unsigned short id = 100;  // tibia.dat start with id 100
	FILE *fp;
	long size;
	int speed;
	
	fp = fopen(filename, "rb");
	if(!fp)
		return false;
	
	fseek(fp,0,SEEK_END);
	size=ftell(fp);

	fseek(fp, 0x0C, SEEK_SET);
	// loop throw all Items until we reach the end of file
	while(ftell(fp) < size)
	{
		ItemType *iType = new ItemType();
		iType->id = id;
		// read the options until we find a 0xff
		int optbyte;
		
		while (((optbyte = fgetc(fp)) >= 0) && (optbyte != 0xFF))
		{                                                            
			switch (optbyte)
			{
			case 0x00:
				//is groundtile
				iType->groundtile=true;
				speed=(int)fgetc(fp);
				
				if(speed==0) {
					iType->blocking=true;
				}
				fgetc(fp);
				break;
			case 0x01: // all OnTop
				iType->alwaysOnTop=true;
				break;
			case 0x02: // can walk trough (open doors, arces, bug pen fence ??)
				iType->alwaysOnTop=true;
				break;
			case 0x03:
				//is a container
				iType->container=true;
				break;
			case 0x04:
				//is stackable
				iType->stackable=true;
				break;
			case 0x05:
				//is useable
				iType->useable=true;
				break;
			case 0x0A:
				//is multitype !!! wrong definition (only water splash on floor)
				iType->fluidcontainer = true;
				break;
			case 0x0B:
				//is blocking
				iType->blocking=true;
				break;
			case 0x0C:
				//is on moveable
				iType->notMoveable=true;
				break;
			case 0x0F:
				//can be equipped
				iType->pickupable=true;
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
				iType->fluidcontainer = true;
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
				//unsigned char a;
				/*a =*/ fgetc(fp); //12, 186, 210, 129 and other.. 
				fgetc(fp); //always 0
				//if(a == 210)
				//printf("%d - %d %d\n", iType->id, a);
				//iType->floorChange = true;
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
		iType->width  = fgetc(fp);
		iType->height = fgetc(fp);
		if((iType->width > 1) || (iType->height > 1)){
			fgetc(fp);
		}
		
		iType->blendframes = fgetc(fp);
		iType->xdiv        = fgetc(fp);
		iType->ydiv        = fgetc(fp);
		iType->animcount   = fgetc(fp);
		
		//fseek(fp, iType->width*iType->height*iType->blendframes*iType->xdiv*iType->ydiv*iType->animcount*2, SEEK_CUR);
		
		iType->numsprites = iType->width * iType->height * iType->blendframes * iType->xdiv * iType->ydiv * iType->animcount;
		// Dynamic memoy reserve
		//iType->imageID = (unsigned short *)calloc(iType->numsprites, sizeof(unsigned short));
		iType->imageID = new unsigned short[iType->numsprites];
		
		// Read the sprite ids
		for(int i = 0; i < iType->numsprites; ++i) {
			fread(&iType->imageID[i], sizeof(unsigned short), 1, fp);
			Sprite *newSprite = new Sprite();
			if(id == 460){
				g_gui->loadSpriteInternalTransparent(0xFF0000,&newSprite->internal);
				iType->imageID[i] = 0xF000;
			}
			else if(id == 459){
				g_gui->loadSpriteInternalTransparent(0xFFFF00,&newSprite->internal);
				iType->imageID[i] = 0xF001;
			}
			newSprite->id = iType->imageID[i];
			// Sprite added to the SpriteMap
			sprite[iType->imageID[i]] = newSprite;
		}
		
		// store the found item
		item[id] = iType;
		id++;
	}
	
	fclose(fp);
	
	datLoaded = true;
	
	return true;
}

unsigned short Items::getSpriteDump(unsigned short id, char *dump)
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

unsigned short Items::getItemSpriteID(unsigned short item_id, unsigned short frame )
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

InternalSprite Items::getSpriteInternalFormat(unsigned short item_id, unsigned short frame)
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


const ItemType& Items::operator[](int id)
{
	ItemMap::iterator it = item.find(id);
	if ((it != item.end()) && (it->second != NULL))
		return *it->second;
	   
	return m_dummy;
}
