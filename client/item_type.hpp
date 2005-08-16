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

#ifndef __OTITEMEDITOR_ITEM_TYPE_H__
#define __OTITEMEDITOR_ITEM_TYPE_H__

#include <map>

enum itemgroup_t{
	ITEM_GROUP_NONE,
	ITEM_GROUP_GROUND,
	ITEM_GROUP_CONTAINER,
	ITEM_GROUP_SPLASH
};


class ItemType
{
public:
	ItemType();
	~ItemType(){}
	
	unsigned short id;
	bool groundtile;
	bool blocking;
	bool alwaysOnTop;
	bool container;
	bool stackable;
	bool useable;
	bool notMoveable;
	bool pickupable;
	bool fluidcontainer;
	bool creature;

};


#endif 
