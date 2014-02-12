﻿#region Licence
/**
* Copyright (C) 2005-2014 <https://github.com/opentibia/item-editor/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#endregion

using OTItemEditor;
using PluginInterface;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace Plugin850
{
	public class Item850 : SpriteItem
	{
		public bool hasCharges;

		public override bool IsEqual(Item item)
		{
			if (item is Item850)
			{
				if(((Item850)item).hasCharges != hasCharges)
				{
					return false;
				}
			}
			return base.IsEqual(item);
		}
	}

	public class Plugin : IPlugin
	{
		Dictionary<UInt16, Sprite> sprites = new Dictionary<UInt16, Sprite>();
		SpriteItems items = new SpriteItems();
		List<SupportedClient> supportedClients = new List<SupportedClient>();
		IPluginHost myHost = null;

		//IPlugin implementation
		public IPluginHost Host { get { return myHost; } set { myHost = value; } }

		public List<SupportedClient> SupportedClients { get { return supportedClients; } }
		public SpriteItems Items { get { return items; } set { items = value; } }

		public bool LoadClient(SupportedClient client, string datFullPath, string sprFullPath)
		{
			return loadDat(datFullPath, client.datSignature) && loadSprites(sprFullPath, client.sprSignature);
		}

		public void Initialize()
		{
			settings.Load("plugin850.xml");
			supportedClients = settings.GetSupportedClientList();
		}

		public void Dispose()
		{
			sprites.Clear();
			items.Clear();
		}

		//internal implementation
		public Settings settings = new Settings();

		public bool loadSprites(string filename, UInt32 signature)
		{
			return Sprite.LoadSprites(filename, ref sprites, signature);
		}

		public bool loadDat(string filename, UInt32 signature)
		{
			FileStream fileStream = new FileStream(filename, FileMode.Open);
			try
			{
				using (BinaryReader reader = new BinaryReader(fileStream))
				{
					UInt32 datSignature = reader.ReadUInt32();
					if (signature != 0 && datSignature != signature)
					{
						string message = "Plugin850: Bad dat signature. Expected signature is {0:X} and loaded signature is {1:X}.";
						Trace.WriteLine(String.Format(message, datSignature, signature));
						return false;
					}

					//get max id
					UInt16 itemCount = reader.ReadUInt16();
					UInt16 creatureCount = reader.ReadUInt16();
					UInt16 effectCount = reader.ReadUInt16();
					UInt16 distanceCount = reader.ReadUInt16();

					UInt16 minclientID = 100; //items starts at 100
					UInt16 maxclientID = itemCount;

					UInt16 id = minclientID;
					while (id <= maxclientID)
					{
						Item850 item = new Item850();
						item.id = id;
						items[id] = item;

						// read the options until we find 0xff
						byte optbyte;
						do
						{
							optbyte = reader.ReadByte();
							//Trace.WriteLine(String.Format("{0:X}", optbyte));

							switch (optbyte)
							{
								case 0x00: //groundtile
									{
										item.groundSpeed = reader.ReadUInt16();
										item.type = ItemType.Ground;
									} break;

								case 0x01: //all OnTop
									{
										item.alwaysOnTop = true;
										item.alwaysOnTopOrder = 1;
									} break;

								case 0x02: //can walk trough (open doors, arces, bug pen fence)
									{
										item.alwaysOnTop = true;
										item.alwaysOnTopOrder = 2;
									} break;

								case 0x03: //can walk trough (arces)
									{
										item.alwaysOnTop = true;
										item.alwaysOnTopOrder = 3;
									} break;

								case 0x04: //container
									{
										item.type = ItemType.Container;
									} break;

								case 0x05: //stackable
									{
										item.isStackable = true;
										break;
									}

								case 0x06:
									{
										//
									} break;

								case 0x07: //useable
									{
										item.multiUse = true;
									} break;

								case 0x08: //charges
									{
										item.hasCharges = true;
									} break;

								case 0x09: //read/write-able
									{
										item.isReadable = true;
										//item.isWriteable = true;
										item.maxReadWriteChars = reader.ReadUInt16();
									} break;

								case 0x0A: //readable objects
									{
										item.isReadable = true;
										item.maxReadChars = reader.ReadUInt16();
									} break;

								case 0x0B: //fluid containers
									{
										item.type = ItemType.Fluid;
									} break;

								case 0x0C: //splashes
									{
										item.type = ItemType.Splash;
									} break;

								case 0x0D: //blocks solid objects (creatures, walls etc)
									{
										item.isUnpassable = true;
									} break;

								case 0x0E: //not moveable
									{
										item.isMoveable = false;
									} break;

								case 0x0F: //blocks missiles (walls, magic wall etc)
									{
										item.blockMissiles = true;
									} break;

								case 0x10: //blocks pathfind algorithms (monsters)
									{
										item.blockPathfinder = true;
									} break;

								case 0x11: //blocks monster movement (flowers, parcels etc)
									{
										item.isPickupable = true;
									} break;

								case 0x12: //hangable objects (wallpaper etc)
									{
										item.isHangable = true;
									} break;

								case 0x13: //horizontal wall
									{
										item.isHorizontal = true;
									} break;

								case 0x14: //vertical wall
									{
										item.isVertical = true;
									} break;

								case 0x15: //rotatable
									{
										item.isRotatable = true;
									} break;

								case 0x16: //light info
									{
										item.lightLevel = reader.ReadUInt16();
										item.lightColor = reader.ReadUInt16();
									} break;

								case 0x17: //changes floor
									{
									} break;

								case 0x18: //unknown
									{
									} break;

								case 0x19: //unknown
									{
										reader.BaseStream.Seek(4, SeekOrigin.Current);
									} break;

								case 0x1A: //
									{
										item.hasElevation = true;
										UInt16 height = reader.ReadUInt16();
									} break;

								case 0x1B: //draw with height offset for all parts (2x2) of the sprite
									{
									} break;

								case 0x1C: //unknown
									{
										break;
									}

								case 0x1D: //minimap color
									{
										item.minimapColor = reader.ReadUInt16();
									} break;

								case 0x1E: //in-game help info
									{
										UInt16 opt = reader.ReadUInt16();
										if (opt == 1112)
										{
											item.isReadable = true;
										}
									} break;

								case 0x1F: //unknown
									{
									} break;

								case 0x20: //look through (borders)
									{
										item.ignoreLook = true;
									} break;

								case 0xFF: //end of attributes
									{
									} break;

								default:
									{
										Trace.WriteLine(String.Format("Unknown optbyte {0:X}", optbyte));
										return false;
									}
							}
						} while (optbyte != 0xFF);

						item.width = reader.ReadByte();
						item.height = reader.ReadByte();
						if ((item.width > 1) || (item.height > 1))
						{
							reader.BaseStream.Position++;
						}

						item.layers = reader.ReadByte();
						item.patternX = reader.ReadByte();
						item.patternY = reader.ReadByte();
						item.patternZ = reader.ReadByte();
						item.frames = reader.ReadByte();
						item.isAnimation = item.frames > 1;
						item.numSprites = (UInt32)item.width * item.height * item.layers * item.patternX * item.patternY * item.patternZ * item.frames;

						// Read the sprite ids
						for (UInt32 i = 0; i < item.numSprites; ++i)
						{
							UInt16 spriteId = reader.ReadUInt16();
							Sprite sprite;
							if (!sprites.TryGetValue(spriteId, out sprite))
							{
								sprite = new Sprite();
								sprite.id = spriteId;
								sprites[spriteId] = sprite;
							}

							item.spriteList.Add(sprite);
						}

						++id;
					}
				}
			}
			finally
			{
				fileStream.Close();
			}

			return true;
		}
	}
}
