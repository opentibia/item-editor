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

#ifndef __OTITEMEDITOR_ITEM_SPRITE_H__
#define __OTITEMEDITOR_ITEM_SPRITE_H__

#include <map>
#include <vector>
#include "item_type.hpp"

typedef void* InternalSprite;

class Sprite
{
public:
	Sprite();
	~Sprite(){}

	unsigned short id;
	unsigned char *dump;
	unsigned short size;
	InternalSprite internal;
};

class SpriteType
{
public:
	SpriteType();
	~SpriteType(){}

	static long minClientId;
	static long maxClientId;
	
	unsigned short id;
	char sprHash[16];
	
	//item info
	itemgroup_t group;
	
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
	bool corpse;

	int speed;

	int lightLevel;
	int lightColor;

	bool isVertical;
	bool isHorizontal;

	bool isHangable;

	unsigned short miniMapColor;
	
	unsigned short subParam07;
	unsigned short subParam08;

	//sprite info
	unsigned char height;
	unsigned char width;
	unsigned char blendframes;
	unsigned char xdiv;
	unsigned char ydiv;
	unsigned char animcount;
	int numsprites;
	
	unsigned short *imageID;

	bool compareOptions(const SpriteType *stype);
};


struct graphicsVersion{
	unsigned int dat;
	unsigned int spr;
	unsigned int client;
	unsigned int otb;
};


class ItemsSprites
{
public:
	ItemsSprites();
	~ItemsSprites();
	
	bool loadFromDat(const char *filename);
	bool loadFromSpr(const char *filename);
	bool loadKnownVersions(const char* filename);
	bool loadHash();
	void unloadSpr();
	void unloadDat();
	
	unsigned short getSpriteDump(unsigned short id, char *dump);
	size_t getSpriteMapSize() { return sprite.size();}
	size_t getItemMapSize() { return item.size();}
	unsigned short getItemSpriteID(unsigned short item_id, unsigned short frame );
	
	InternalSprite getSpriteInternalFormat(unsigned short item_id, unsigned short frame);

	SpriteType* getSprite(int id);
	
	const graphicsVersion* getVersion();

protected:
	bool datLoaded;
	bool sprLoaded;
	
	typedef std::map<unsigned short, SpriteType*> ItemMap;
	ItemMap item;
	
	typedef std::map<unsigned short, Sprite*> SpriteMap;
	SpriteMap sprite;

	SpriteType m_dummy;
	
	typedef std::vector<graphicsVersion*> versionVector;
	versionVector versions;

	unsigned int datVersion;
	unsigned int sprVersion;
};


#endif
