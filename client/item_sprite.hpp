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
	
	//item info
	bool groundtile;
	bool blocking;
	bool alwaysOnTop;
	bool container;
	bool stackable;
	bool useable;
	bool notMoveable;
	bool pickupable;
	bool fluid;
	bool rotable;
	int speed;

	//sprite info
	unsigned char height;
	unsigned char width;
	unsigned char blendframes;
	unsigned char xdiv;
	unsigned char ydiv;
	unsigned char animcount;
	int numsprites;
	
	unsigned short *imageID;
};



class ItemsSprites
{
public:
	ItemsSprites();
	~ItemsSprites(){}
	
	bool loadFromDat(const char *filename);
	bool loadFromSpr(const char *filename);
	void unloadSpr();
	void unloadDat();
	
	unsigned short getSpriteDump(unsigned short id, char *dump);
	unsigned short getSpriteMapSize() { return sprite.size();}
	unsigned short getItemMapSize() { return item.size();}
	unsigned short getItemSpriteID(unsigned short item_id, unsigned short frame );
	
	InternalSprite getSpriteInternalFormat(unsigned short item_id, unsigned short frame);

	SpriteType* getSprite(int id);
	//const SpriteType& operator[](int id);

protected:
	bool datLoaded;
	bool sprLoaded;
	
	typedef std::map<unsigned short, SpriteType*> ItemMap;
	ItemMap item;
	
	typedef std::map<unsigned short, Sprite*> SpriteMap;
	SpriteMap sprite;

	SpriteType m_dummy;
	
};


#endif