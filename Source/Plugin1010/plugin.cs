#region Licence
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

namespace Plugin1010
{
	public class Plugin : IPlugin
	{
		Dictionary<UInt32, Sprite> sprites = new Dictionary<UInt32, Sprite>();
		SpriteItems items = new SpriteItems();
		List<SupportedClient> supportedClients = new List<SupportedClient>();

		IPluginHost myHost = null;

		//IPlugin implementation
		public IPluginHost Host { get { return myHost; } set { myHost = value; } }

		public List<SupportedClient> SupportedClients { get { return supportedClients; } }
		public SpriteItems Items { get { return items; } set { items = value; } }

		public bool LoadClient(SupportedClient client, string datFullPath, string sprFullPath)
		{
			if (!LoadDat(datFullPath, client.datSignature))
			{
				Trace.WriteLine("Failed to load dat.");
				return false;
			}

			if (!LoadSprites(sprFullPath, client.sprSignature))
			{
				Trace.WriteLine("Failed to load spr.");
				return false;
			}
			return true;
		}

		public void Initialize()
		{
			settings.Load("plugin1010.xml");
			supportedClients = settings.GetSupportedClientList();
		}

		public void Dispose()
		{
			sprites.Clear();
			items.Clear();
		}

		//internal implementation
		public Settings settings = new Settings();

		public bool LoadSprites(string filename, UInt32 signature)
		{
			return Sprite.LoadSprites(filename, ref sprites, signature);
		}

		public bool LoadDat(string filename, UInt32 signature)
		{
			FileStream fileStream = new FileStream(filename, FileMode.Open);
			try
			{
				using (BinaryReader reader = new BinaryReader(fileStream))
				{
					UInt32 datSignature = reader.ReadUInt32();
					if (signature != 0 && datSignature != signature)
					{
						string message = "Plugin1010: Bad dat signature. Expected signature is {0:X} and loaded signature is {1:X}.";
						Trace.WriteLine(String.Format(message, datSignature, signature));
						return false;
					}

					//get max id
					UInt16 itemCount = reader.ReadUInt16();
					Trace.WriteLine(String.Format("Plugin1010: itemCount is {0}", itemCount));
					UInt16 creatureCount = reader.ReadUInt16();
					UInt16 effectCount = reader.ReadUInt16();
					UInt16 distanceCount = reader.ReadUInt16();

					UInt16 minclientID = 100; //items starts at 100
					UInt16 maxclientID = itemCount;

					UInt16 id = minclientID;
					while (id <= maxclientID)
					{
						SpriteItem item = new SpriteItem();
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

								case 0x06: //unknown
									{
									} break;

								case 0x07: //useable
									{
										item.multiUse = true;
									} break;

								case 0x08: //read/write-able
									{
										item.isReadable = true;
										//item.isWriteable = true;
										item.maxReadWriteChars = reader.ReadUInt16();
									} break;

								case 0x09: //readable
									{
										item.isReadable = true;
										item.maxReadChars = reader.ReadUInt16();
									} break;

								case 0x0A: //fluid containers
									{
										item.type = ItemType.Fluid;
									} break;

								case 0x0B: //splashes
									{
										item.type = ItemType.Splash;
									} break;

								case 0x0C: //blocks solid objects (creatures, walls etc)
									{
										item.isUnpassable = true;
									} break;

								case 0x0D: //not moveable
									{
										item.isMoveable = false;
									} break;

								case 0x0E: //blocks missiles (walls, magic wall etc)
									{
										item.blockMissiles = true;
									} break;

								case 0x0F: //blocks pathfind algorithms (monsters)
									{
										item.blockPathfinder = true;
									} break;

								case 0x10: // no move animation
									{
										// item.noMoveAnimation = true;
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

								case 0x17: //unknown
									{
									} break;

								case 0x18: //changes floor
									{
									} break;

								case 0x19: //unknown
									{
										reader.BaseStream.Seek(4, SeekOrigin.Current);
									} break;

								case 0x1A:
									{
										item.hasElevation = true;
										UInt16 height = reader.ReadUInt16();
									} break;

								case 0x1B: //unknown
									{
									} break;


								case 0x1C: //unknown
									{
									} break;

								case 0x1D: //minimap color
									{
										item.minimapColor = reader.ReadUInt16();
										break;
									}

								case 0x1E: //in-game help info
									{
										UInt16 opt = reader.ReadUInt16();
										if (opt == 1112)
										{
											item.isReadable = true;
										}
									} break;

								case 0x1F: //full tile
									{
										item.fullGround = true;
									} break;

								case 0x20: //look through (borders)
									{
										item.ignoreLook = true;
									} break;

								case 0x21: //unknown
									{
										reader.ReadUInt16();
									} break;

								case 0x22: //market
									{
										reader.ReadUInt16(); // category
										item.tradeAs = reader.ReadUInt16(); // trade as
										reader.ReadUInt16(); // show as
										var size = reader.ReadUInt16();
										item.name = new string(reader.ReadChars(size));

										reader.ReadUInt16(); // profession
										reader.ReadUInt16(); // level
									}
									break;

								case 0xFF: //end of attributes
									{
									} break;

								default:
									{
										Trace.WriteLine(String.Format("Plugin1010: Error while parsing, unknown optbyte 0x{0:X} at id {1}", optbyte, id));
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

						item.numSprites =
							(UInt32)item.width * (UInt32)item.height *
							(UInt32)item.layers *
							(UInt32)item.patternX * (UInt32)item.patternY * item.patternZ *
							(UInt32)item.frames;

						// Read the sprite ids
						for (UInt32 i = 0; i < item.numSprites; ++i)
						{
							var spriteId = reader.ReadUInt32();
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
