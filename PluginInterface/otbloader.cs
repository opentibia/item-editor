using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using PluginInterface;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace otitemeditor
{
	class OtbLoader : BinaryReader
	{
		enum SpecialChars
		{
			NODE_START = 0xFE,
			NODE_END = 0xFF,
			ESCAPE_CHAR = 0xFD,
		};

		public OtbLoader(Stream input)
			: base(input)
		{
			//
		}

		public BinaryReader getRootNode()
		{
			return getChildNode();
		}

		public BinaryReader getChildNode()
		{
			advance();
			return getNodeData();
		}

		public BinaryReader getNextNode()
		{
			BaseStream.Seek(currentNodePos, SeekOrigin.Begin);

			byte value = ReadByte();
			if((SpecialChars)value != SpecialChars.NODE_START){
				return null;
			}
			
			value = ReadByte();

			Int32 level = 1;
			while(true){
				value = ReadByte();		
				if((SpecialChars)value == SpecialChars.NODE_END){
					--level;
					if(level == 0){
						value = ReadByte();
						if((SpecialChars)value == SpecialChars.NODE_END){
							return null;
						}
						else if((SpecialChars)value != SpecialChars.NODE_START){
							return null;
						}
						else{
							currentNodePos = BaseStream.Position - 1;
							return getNodeData();
						}
					}
				}
				else if((SpecialChars)value == SpecialChars.NODE_START){
					++level;
				}
				else if((SpecialChars)value == SpecialChars.ESCAPE_CHAR){
					ReadByte();
				}
			}
		}

		private BinaryReader getNodeData()
		{
			BaseStream.Seek(currentNodePos, SeekOrigin.Begin);

			//read node type
			byte value = ReadByte();

			if((SpecialChars)value != SpecialChars.NODE_START){
				return null;
			}

			MemoryStream ms = new MemoryStream(200);

			currentNodeSize = 0;
			while (true)
			{
				value = ReadByte();
				if ((SpecialChars)value == SpecialChars.NODE_END || (SpecialChars)value == SpecialChars.NODE_START)
					break;
				else if ((SpecialChars)value == SpecialChars.ESCAPE_CHAR)
				{
					value = ReadByte();
				}
				++currentNodeSize;
				ms.WriteByte(value);
			}

			BaseStream.Seek(currentNodePos, SeekOrigin.Begin);
			ms.Position = 0;
			return new BinaryReader(ms);
		}

		private bool advance()
		{
			try{
				Int64 seekPos = 0;
				if (currentNodePos == 0)
				{
					seekPos = 4;
				}
				else
				{
					seekPos = currentNodePos;
				}

				BaseStream.Seek(seekPos, SeekOrigin.Begin);

				byte value = ReadByte();
				if((SpecialChars)value != SpecialChars.NODE_START){
					return false;
				}

				if (currentNodePos == 0)
				{
					currentNodePos = BaseStream.Position - 1;
					return true;
				}
				else
				{
					value = ReadByte();

					while (true)
					{
						value = ReadByte();
						if ((SpecialChars)value == SpecialChars.NODE_END)
						{
							return false;
						}
						else if ((SpecialChars)value == SpecialChars.NODE_START)
						{
							currentNodePos = BaseStream.Position - 1;
							return true;
						}
						else if ((SpecialChars)value == SpecialChars.ESCAPE_CHAR)
						{
							ReadByte();
						}
					}
				}
			}
			catch(Exception)
			{
				return false;
			}
		}

		public void createNode(byte type)
		{
			writeByte((byte)SpecialChars.NODE_START, false);
			writeByte(type);
		}

		public void closeNode()
		{
			writeByte((byte)SpecialChars.NODE_END, false);
		}

		public void writeByte(byte value)
		{
			byte[] bytes = new byte[1] {value};
			writeBytes(bytes, true);
		}

		public void writeByte(byte value, bool unescape)
		{
			byte[] bytes = new byte[1] { value };
			writeBytes(bytes, unescape);
		}

		public void writeUInt16(UInt16 value)
		{
			byte[] bytes = BitConverter.GetBytes(value);
			writeBytes(bytes, true);
		}

		public void writeUInt16(UInt16 value, bool unescape)
		{
			byte[] bytes = BitConverter.GetBytes(value);
			writeBytes(bytes, unescape);
		}

		public void writeUInt32(UInt32 value)
		{
			byte[] bytes = BitConverter.GetBytes(value);
			writeBytes(bytes, true);
		}

		public void writeUInt32(UInt32 value, bool unescape)
		{
			byte[] bytes = BitConverter.GetBytes(value);
			writeBytes(bytes, unescape);
		}

		public void writeProp(otb.itemattrib_t attr, BinaryWriter writer)
		{
			writer.BaseStream.Position = 0;
			byte[] bytes = new byte[writer.BaseStream.Length];
			writer.BaseStream.Read(bytes, 0, (int)writer.BaseStream.Length);
			writer.BaseStream.Position = 0;
			writer.BaseStream.SetLength(0);

			writeProp((byte)attr, bytes);
		}

		public void writeProp(otb.rootattrib_t attr, BinaryWriter writer)
		{
			writer.BaseStream.Position = 0;
			byte[] bytes = new byte[writer.BaseStream.Length];
			writer.BaseStream.Read(bytes, 0, (int)writer.BaseStream.Length);
			writer.BaseStream.Position = 0;
			writer.BaseStream.SetLength(0);

			writeProp((byte)attr, bytes);
		}

		private void writeProp(byte attr, byte[] bytes)
		{
			writeByte((byte)attr);
			writeUInt16((UInt16)bytes.Length);
			writeBytes(bytes, true);
		}

		public void writeBytes(byte[] bytes, bool unescape)
		{
			foreach(byte b in bytes)
			{
				if(unescape && (b == (byte)SpecialChars.NODE_START || b == (byte)SpecialChars.NODE_END || b == (byte)SpecialChars.ESCAPE_CHAR))
				{
					BaseStream.WriteByte((byte)SpecialChars.ESCAPE_CHAR);
				}

				BaseStream.WriteByte(b);
			}
		}

		public Int64 currentNodePos = 0;
		public UInt32 currentNodeSize = 0;
	};

	public class OtbItem : Item
	{
		public OtbItem() : base()
		{
			//
		}

		public OtbItem(Item item)
			: base()
		{
			if (item != null)
			{
				this.itemImpl = (ItemImpl)item.itemImpl.Clone();
			}
		}

		public UInt16 spriteId;
		public UInt16 prevSpriteId;

		//Used during an update to indicate if this item has been updated
		public bool spriteAssigned = false;

		//An custom created item id
		public bool isCustomCreated = false;
	}

	public class OtbList : List<OtbItem>
	{
		public UInt16 minId = 100;
		public UInt16 maxId { get { return (UInt16)(minId + Count - 1); } }

		public UInt32 dwMajorVersion;
		public UInt32 dwMinorVersion;
		public UInt32 dwBuildNumber;

		public UInt32 clientVersion;
	}

	public class otb
	{
		public enum clientversion_t
		{
			CLIENT_VERSION_UNKNOWN = 0,
			CLIENT_VERSION_750 = 1,
			CLIENT_VERSION_755 = 2,
			CLIENT_VERSION_760 = 3,
			CLIENT_VERSION_770 = 3,
			CLIENT_VERSION_780 = 4,
			CLIENT_VERSION_790 = 5,
			CLIENT_VERSION_792 = 6,
			CLIENT_VERSION_800 = 7,
			CLIENT_VERSION_810 = 8,
			CLIENT_VERSION_811 = 9,
			CLIENT_VERSION_820 = 10,
			CLIENT_VERSION_830 = 11,
			CLIENT_VERSION_840 = 12,
			CLIENT_VERSION_841 = 13,
			CLIENT_VERSION_842 = 14,
			CLIENT_VERSION_850 = 15,
			CLIENT_VERSION_854_BAD = 16,
			CLIENT_VERSION_854 = 17,
			CLIENT_VERSION_855 = 18,
			CLIENT_VERSION_860_OLD = 19,
			CLIENT_VERSION_860 = 20,
			CLIENT_VERSION_861 = 21,
			CLIENT_VERSION_862 = 22,
			CLIENT_VERSION_870 = 23,
			CLIENT_VERSION_871 = 24,
			CLIENT_VERSION_872 = 25,
			CLIENT_VERSION_873 = 26,
			CLIENT_VERSION_900 = 27,
			CLIENT_VERSION_910 = 28,
			CLIENT_VERSION_920 = 29,
			CLIENT_VERSION_940 = 30
		};

		public enum itemgroup_t
		{
			ITEM_GROUP_NONE = 0,
			ITEM_GROUP_GROUND,
			ITEM_GROUP_CONTAINER,
			ITEM_GROUP_WEAPON,
			ITEM_GROUP_AMMUNITION,
			ITEM_GROUP_ARMOR,
			ITEM_GROUP_CHARGES,
			ITEM_GROUP_TELEPORT,
			ITEM_GROUP_MAGICFIELD,
			ITEM_GROUP_WRITEABLE,
			ITEM_GROUP_KEY,
			ITEM_GROUP_SPLASH,
			ITEM_GROUP_FLUID,
			ITEM_GROUP_DOOR,
			ITEM_GROUP_DEPRECATED,
			ITEM_GROUP_LAST
		};

		public enum itemattrib_t
		{
			ITEM_ATTR_FIRST = 0x10,
			ITEM_ATTR_SERVERID = ITEM_ATTR_FIRST,
			ITEM_ATTR_CLIENTID,
			ITEM_ATTR_NAME,				/*deprecated*/
			ITEM_ATTR_DESCR,			/*deprecated*/
			ITEM_ATTR_SPEED,
			ITEM_ATTR_SLOT,				/*deprecated*/
			ITEM_ATTR_MAXITEMS,			/*deprecated*/
			ITEM_ATTR_WEIGHT,			/*deprecated*/
			ITEM_ATTR_WEAPON,			/*deprecated*/
			ITEM_ATTR_AMU,				/*deprecated*/
			ITEM_ATTR_ARMOR,			/*deprecated*/
			ITEM_ATTR_MAGLEVEL,			/*deprecated*/
			ITEM_ATTR_MAGFIELDTYPE,		/*deprecated*/
			ITEM_ATTR_WRITEABLE,		/*deprecated*/
			ITEM_ATTR_ROTATETO,			/*deprecated*/
			ITEM_ATTR_DECAY,			/*deprecated*/
			ITEM_ATTR_SPRITEHASH,
			ITEM_ATTR_MINIMAPCOLOR,
			ITEM_ATTR_07,
			ITEM_ATTR_08,
			ITEM_ATTR_LIGHT,			/*deprecated*/

			//1-byte aligned
			ITEM_ATTR_DECAY2,			/*deprecated*/
			ITEM_ATTR_WEAPON2,			/*deprecated*/
			ITEM_ATTR_AMU2,				/*deprecated*/
			ITEM_ATTR_ARMOR2,			/*deprecated*/
			ITEM_ATTR_WRITEABLE2,		/*deprecated*/
			ITEM_ATTR_LIGHT2,
			ITEM_ATTR_TOPORDER,
			ITEM_ATTR_WRITEABLE3,		/*deprecated*/
			ITEM_ATTR_WAREID,
			ITEM_ATTR_NAME2,

			ITEM_ATTR_LAST
		};

		public enum rootattrib_t
		{
			ROOT_ATTR_VERSION = 0x01
		};

		public class VERSIONINFO{
			public UInt32 dwMajorVersion;
			public UInt32 dwMinorVersion;
			public UInt32 dwBuildNumber;
			public string CSDVersion;
		};

		[FlagsAttribute]
		public enum itemflags_t
		{
			FLAG_BLOCK_SOLID = 1,
			FLAG_BLOCK_PROJECTILE = 2,
			FLAG_BLOCK_PATHFIND = 4,
			FLAG_HAS_HEIGHT = 8,
			FLAG_USEABLE = 16,
			FLAG_PICKUPABLE = 32,
			FLAG_MOVEABLE = 64,
			FLAG_STACKABLE = 128,
			FLAG_FLOORCHANGEDOWN = 256,
			FLAG_FLOORCHANGENORTH = 512,
			FLAG_FLOORCHANGEEAST = 1024,
			FLAG_FLOORCHANGESOUTH = 2048,
			FLAG_FLOORCHANGEWEST = 4096,
			FLAG_ALWAYSONTOP = 8192,
			FLAG_READABLE = 16384,
			FLAG_ROTABLE = 32768,
			FLAG_HANGABLE = 65536,
			FLAG_VERTICAL = 131072,
			FLAG_HORIZONTAL = 262144,
			FLAG_CANNOTDECAY = 524288,
			FLAG_ALLOWDISTREAD = 1048576,	/*deprecated*/
			FLAG_CORPSE = 2097152,			/*deprecated*/
			FLAG_CLIENTCHARGES = 4194304,	/*deprecated*/
			FLAG_LOOKTHROUGH = 8388608,
			FLAG_ANIMATION = 16777216,
			FLAG_WALKSTACK = 33554432
		};

		public static bool open(string filename, ref OtbList items, bool outputDebug)
		{
			FileStream fileStream = new FileStream(filename, FileMode.Open);
			try
			{
				using (OtbLoader reader = new OtbLoader(fileStream))
				{
					//get root node
					BinaryReader nodeReader = reader.getRootNode();
					if (nodeReader == null)
					{
						return false;
					}

					nodeReader.ReadByte(); //first byte of otb is 0
					nodeReader.ReadUInt32(); //4 bytes flags, unused

					byte attr = nodeReader.ReadByte();
					if ((rootattrib_t)attr == rootattrib_t.ROOT_ATTR_VERSION)
					{
						UInt16 datalen = nodeReader.ReadUInt16();
						if (datalen != 4 + 4 + 4 + 1 * 128)
						{
							//error = wxT("items.otb: Size of version header is invalid, updated .otb version?");
							if (outputDebug)
							{
								Trace.WriteLine(String.Format("Size of version header is invalid."));
							}
							return false;
						}

						items.dwMajorVersion = nodeReader.ReadUInt32(); //major, file version
						items.dwMinorVersion = nodeReader.ReadUInt32(); //minor, client version
						items.dwBuildNumber = nodeReader.ReadUInt32(); //build number, revision
						nodeReader.BaseStream.Seek(128, SeekOrigin.Current);
					}

					nodeReader = reader.getChildNode();
					if (nodeReader == null)
					{
						return false;
					}

					do
					{
						OtbItem item = new OtbItem();

						byte itemGroup = nodeReader.ReadByte();
						if (outputDebug)
						{
							Trace.WriteLine(String.Format("Node:ItemGroup {0}", (itemgroup_t)itemGroup));
						}

						switch ((itemgroup_t)itemGroup)
						{
							case itemgroup_t.ITEM_GROUP_NONE: item.type = ItemType.None; break;
							case itemgroup_t.ITEM_GROUP_GROUND: item.type = ItemType.Ground; break;
							case itemgroup_t.ITEM_GROUP_SPLASH: item.type = ItemType.Splash; break;
							case itemgroup_t.ITEM_GROUP_FLUID: item.type = ItemType.Fluid; break;
							case itemgroup_t.ITEM_GROUP_CONTAINER: item.type = ItemType.Container; break;
							case itemgroup_t.ITEM_GROUP_DEPRECATED: item.type = ItemType.Deprecated; break;
							default: break;
						}

						itemflags_t flags = (itemflags_t)nodeReader.ReadUInt32();
						if (outputDebug)
						{
							Trace.WriteLine(String.Format("Node:flags {0}", flags));
						}

						item.blockObject = ((flags & itemflags_t.FLAG_BLOCK_SOLID) == itemflags_t.FLAG_BLOCK_SOLID);
						item.blockProjectile = ((flags & itemflags_t.FLAG_BLOCK_PROJECTILE) == itemflags_t.FLAG_BLOCK_PROJECTILE);
						item.blockPathFind = ((flags & itemflags_t.FLAG_BLOCK_PATHFIND) == itemflags_t.FLAG_BLOCK_PATHFIND);
						item.isPickupable = ((flags & itemflags_t.FLAG_PICKUPABLE) == itemflags_t.FLAG_PICKUPABLE);
						item.isMoveable = ((flags & itemflags_t.FLAG_MOVEABLE) == itemflags_t.FLAG_MOVEABLE);
						item.isStackable = ((flags & itemflags_t.FLAG_STACKABLE) == itemflags_t.FLAG_STACKABLE);
						item.alwaysOnTop = ((flags & itemflags_t.FLAG_ALWAYSONTOP) == itemflags_t.FLAG_ALWAYSONTOP);
						item.isVertical = ((flags & itemflags_t.FLAG_VERTICAL) == itemflags_t.FLAG_VERTICAL);
						item.isHorizontal = ((flags & itemflags_t.FLAG_HORIZONTAL) == itemflags_t.FLAG_HORIZONTAL);
						item.isHangable = ((flags & itemflags_t.FLAG_HANGABLE) == itemflags_t.FLAG_HANGABLE);
						item.isRotatable = ((flags & itemflags_t.FLAG_ROTABLE) == itemflags_t.FLAG_ROTABLE);
						item.isReadable = ((flags & itemflags_t.FLAG_READABLE) == itemflags_t.FLAG_READABLE);
						item.hasUseWith = ((flags & itemflags_t.FLAG_USEABLE) == itemflags_t.FLAG_USEABLE);
						item.hasHeight = ((flags & itemflags_t.FLAG_HAS_HEIGHT) == itemflags_t.FLAG_HAS_HEIGHT);
						item.lookThrough = ((flags & itemflags_t.FLAG_LOOKTHROUGH) == itemflags_t.FLAG_LOOKTHROUGH);
						item.allowDistRead = ((flags & itemflags_t.FLAG_ALLOWDISTREAD) == itemflags_t.FLAG_ALLOWDISTREAD);
						item.isAnimation = ((flags & itemflags_t.FLAG_ANIMATION) == itemflags_t.FLAG_ANIMATION);
						item.walkStack = ((flags & itemflags_t.FLAG_WALKSTACK) == itemflags_t.FLAG_WALKSTACK);

						while (nodeReader.PeekChar() != -1)
						{
							byte attribute = nodeReader.ReadByte();
							UInt16 datalen = nodeReader.ReadUInt16();

							if (outputDebug)
							{
								Trace.WriteLine(String.Format("Node[{0}]:attribut {1}, size: {2}",
									reader.currentNodePos, ((itemattrib_t)attribute), datalen, reader.currentNodePos + nodeReader.BaseStream.Position));
							}

							switch ((itemattrib_t)attribute)
							{
								case itemattrib_t.ITEM_ATTR_SERVERID:
								{
									if (datalen != sizeof(UInt16))
									{
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Unexpected data length of server id block (Should be 2 bytes)"));
										}
										return false;
									}

									item.id = nodeReader.ReadUInt16();
									if (outputDebug)
									{
										System.Diagnostics.Debug.WriteLine(String.Format("Node:attribute:data {0}", item.id));
									}

								} break;

								case itemattrib_t.ITEM_ATTR_CLIENTID:
								{
									if (datalen != sizeof(UInt16))
									{
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Unexpected data length of client id block (Should be 2 bytes)"));
										}
										return false;
									}

									item.spriteId = nodeReader.ReadUInt16();
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Node:attribute:data {0}", item.spriteId));
									}
								} break;

								case itemattrib_t.ITEM_ATTR_WAREID:
								{
									if (datalen != sizeof(UInt16))
									{
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Unexpected data length of ware id block (Should be 2 bytes)"));
										}
										return false;
									}

									item.wareId = nodeReader.ReadUInt16();
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Node:attribute:data {0}", item.wareId));
									}
								} break;

								case itemattrib_t.ITEM_ATTR_SPEED:
								{
									if (datalen != sizeof(UInt16))
									{
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Unexpected data length of speed block (Should be 2 bytes)"));
										}
										return false;
									}

									item.groundSpeed = nodeReader.ReadUInt16();
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Node:attribute:data {0}", item.groundSpeed));
									}
								} break;

								case itemattrib_t.ITEM_ATTR_NAME2:
								{
									if (datalen != sizeof(UInt16))
									{
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Unexpected data length of name block (Should be 2 bytes)"));
										}
										return false;
									}

									UInt16 size = nodeReader.ReadUInt16();
									item.name = new string(nodeReader.ReadChars(size));
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Node:attribute:data {0}", item.name));
									}
								} break;

								case itemattrib_t.ITEM_ATTR_SPRITEHASH:
								{
									if (datalen != 16)
									{
										return false;
									}

									item.spriteHash = nodeReader.ReadBytes(16);
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Node:attribute:data {0}", item.alwaysOnTopOrder));
									}
								} break;
								
								case itemattrib_t.ITEM_ATTR_MINIMAPCOLOR:
								{
									if (datalen != 2)
									{
										return false;
									}

									item.minimapColor = nodeReader.ReadUInt16();
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Node:attribute:data {0}", item.minimapColor));
									}

								} break;

								case itemattrib_t.ITEM_ATTR_07:
									{
										//read/write-able
										if (datalen != 2)
										{
											return false;
										}

										item.maxReadWriteChars = nodeReader.ReadUInt16();
									} break;

								case itemattrib_t.ITEM_ATTR_08:
								{
									//readable
									if (datalen != 2)
									{
										return false;
									}

									item.maxReadChars = nodeReader.ReadUInt16();
								} break;

								case itemattrib_t.ITEM_ATTR_LIGHT2:
									{
										if (datalen != sizeof(UInt16) * 2)
										{
											if (outputDebug)
											{
												Trace.WriteLine(String.Format("Unexpected data length of item light (2) block"));
											}
											return false;
										}

										item.lightLevel = nodeReader.ReadUInt16();
										item.lightColor = nodeReader.ReadUInt16();
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Node:attribute:data {0}, {1}", item.lightLevel, item.lightColor));
										}
									} break;

								case itemattrib_t.ITEM_ATTR_TOPORDER:
									{
										if (datalen != sizeof(byte))
										{
											if (outputDebug)
											{
												Trace.WriteLine(String.Format("Unexpected data length of item toporder block (Should be 1 byte)"));
											}
											return false;
										}

										item.alwaysOnTopOrder = nodeReader.ReadByte();
										if (outputDebug)
										{
											Trace.WriteLine(String.Format("Node:attribute:data {0}", item.alwaysOnTopOrder));
										}
									} break;

								default:
								{
									//skip unknown attributes
									nodeReader.BaseStream.Seek(datalen, SeekOrigin.Current);
									if (outputDebug)
									{
										Trace.WriteLine(String.Format("Skipped unknown attribute"));
									}
								} break;
							}
						}

						items.Add(item);

						nodeReader = reader.getNextNode();
					} while (nodeReader != null);
				}
			}
			finally
			{
				fileStream.Close();
			}

			return true;
		}

		public static bool save(string filename, ref OtbList items)
		{
			FileStream fileStream = new FileStream(filename, FileMode.Create);
			try
			{
				using (OtbLoader writer = new OtbLoader(fileStream))
				{
					writer.writeUInt32(0, false); //version, always 0

					writer.createNode(0); //root node
					writer.writeUInt32(0, true); //flags, unused for root node

					VERSIONINFO vi = new VERSIONINFO();

					vi.dwMajorVersion = items.dwMajorVersion;
					vi.dwMinorVersion = items.dwMinorVersion;
					vi.dwBuildNumber = items.dwBuildNumber;
					vi.CSDVersion = String.Format("OTB {0}.{1}.{2}-{3}.{4}", vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber, items.clientVersion / 100, items.clientVersion % 100);

					MemoryStream ms = new MemoryStream();
					BinaryWriter property = new BinaryWriter(ms);
					property.Write(vi.dwMajorVersion);
					property.Write(vi.dwMinorVersion);
					property.Write(vi.dwBuildNumber);
					byte[] CSDVersion = System.Text.Encoding.ASCII.GetBytes(vi.CSDVersion);
					Array.Resize(ref CSDVersion, 128);
					property.Write(CSDVersion);

					writer.writeProp(rootattrib_t.ROOT_ATTR_VERSION, property);

					foreach (OtbItem item in items)
					{
						List<itemattrib_t> saveAttributeList = new List<itemattrib_t>();

						saveAttributeList.Add(itemattrib_t.ITEM_ATTR_SERVERID);

						if (item.type == ItemType.Deprecated)
						{
							//no other attributes should be saved for this type
						}
						else
						{
							saveAttributeList.Add(itemattrib_t.ITEM_ATTR_CLIENTID);
							saveAttributeList.Add(itemattrib_t.ITEM_ATTR_SPRITEHASH);

							if (item.minimapColor != 0)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_MINIMAPCOLOR);
							}

							if (item.maxReadWriteChars != 0)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_07);
							}

							if (item.maxReadChars != 0)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_08);
							}

							if (item.lightLevel != 0 || item.lightColor != 0)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_LIGHT2);
							}

							if (item.type == ItemType.Ground)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_SPEED);
							}

							if (item.alwaysOnTop)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_TOPORDER);
							}

							if (item.wareId != 0)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_WAREID);
							}

							if (item.name.Length != 0)
							{
								saveAttributeList.Add(itemattrib_t.ITEM_ATTR_NAME2);
							}
						}

						switch(item.type)
						{
							case ItemType.Container: writer.createNode((byte)itemgroup_t.ITEM_GROUP_CONTAINER); break;
							case ItemType.Fluid: writer.createNode((byte)itemgroup_t.ITEM_GROUP_FLUID); break;
							case ItemType.Ground: writer.createNode((byte)itemgroup_t.ITEM_GROUP_GROUND); break;
							case ItemType.Splash: writer.createNode((byte)itemgroup_t.ITEM_GROUP_SPLASH); break;
							case ItemType.Deprecated: writer.createNode((byte)itemgroup_t.ITEM_GROUP_DEPRECATED); break;
							default: writer.createNode((byte)itemgroup_t.ITEM_GROUP_NONE); break;
						}

						UInt32 flags = 0;
						if (item.blockObject)
							flags |= (UInt32)itemflags_t.FLAG_BLOCK_SOLID;

						if (item.blockProjectile)
							flags |= (UInt32)itemflags_t.FLAG_BLOCK_PROJECTILE;

						if (item.blockPathFind)
							flags |= (UInt32)itemflags_t.FLAG_BLOCK_PATHFIND;

						if (item.hasHeight)
							flags |= (UInt32)itemflags_t.FLAG_HAS_HEIGHT;

						if (item.hasUseWith)
							flags |= (UInt32)itemflags_t.FLAG_USEABLE;

						if (item.isPickupable)
							flags |= (UInt32)itemflags_t.FLAG_PICKUPABLE;

						if (item.isMoveable)
							flags |= (UInt32)itemflags_t.FLAG_MOVEABLE;

						if (item.isStackable)
							flags |= (UInt32)itemflags_t.FLAG_STACKABLE;

						if (item.alwaysOnTop)
							flags |= (UInt32)itemflags_t.FLAG_ALWAYSONTOP;

						if (item.isReadable)
							flags |= (UInt32)itemflags_t.FLAG_READABLE;

						if (item.isRotatable)
							flags |= (UInt32)itemflags_t.FLAG_ROTABLE;

						if (item.isHangable)
							flags |= (UInt32)itemflags_t.FLAG_HANGABLE;

						if (item.isVertical)
							flags |= (UInt32)itemflags_t.FLAG_VERTICAL;

						if (item.isHorizontal)
							flags |= (UInt32)itemflags_t.FLAG_HORIZONTAL;

						if (item.lookThrough)
							flags |= (UInt32)itemflags_t.FLAG_LOOKTHROUGH;

						if (item.allowDistRead)
							flags |= (UInt32)itemflags_t.FLAG_ALLOWDISTREAD;

						if (item.walkStack)
							flags |= (UInt32)itemflags_t.FLAG_WALKSTACK;

						writer.writeUInt32(flags, true);

						foreach (itemattrib_t attribute in saveAttributeList)
						{
							switch (attribute)
							{
								case itemattrib_t.ITEM_ATTR_SERVERID:
									{
										property.Write((UInt16)item.id);
										writer.writeProp(itemattrib_t.ITEM_ATTR_SERVERID, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_WAREID:
									{
										property.Write((UInt16)item.wareId);
										writer.writeProp(itemattrib_t.ITEM_ATTR_WAREID, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_CLIENTID:
									{
										property.Write((UInt16)item.spriteId);
										writer.writeProp(itemattrib_t.ITEM_ATTR_CLIENTID, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_SPEED:
									{
										property.Write((UInt16)item.groundSpeed);
										writer.writeProp(itemattrib_t.ITEM_ATTR_SPEED, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_NAME2:
									{
										property.Write((UInt16)item.name.Length);
										property.Write(item.name);
										writer.writeProp(itemattrib_t.ITEM_ATTR_NAME2, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_SPRITEHASH:
									{
										property.Write(item.spriteHash);
										writer.writeProp(itemattrib_t.ITEM_ATTR_SPRITEHASH, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_MINIMAPCOLOR:
									{
										property.Write((UInt16)item.minimapColor);
										writer.writeProp(itemattrib_t.ITEM_ATTR_MINIMAPCOLOR, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_07:
									{
										property.Write((UInt16)item.maxReadWriteChars);
										writer.writeProp(itemattrib_t.ITEM_ATTR_07, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_08:
									{
										property.Write((UInt16)item.maxReadChars);
										writer.writeProp(itemattrib_t.ITEM_ATTR_08, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_LIGHT2:
									{
										property.Write((UInt16)item.lightLevel);
										property.Write((UInt16)item.lightColor);
										writer.writeProp(itemattrib_t.ITEM_ATTR_LIGHT2, property);
										break;
									}

								case itemattrib_t.ITEM_ATTR_TOPORDER:
									{
										property.Write((byte)item.alwaysOnTopOrder);
										writer.writeProp(itemattrib_t.ITEM_ATTR_TOPORDER, property);
										break;
									}
							}
						}

						writer.closeNode();
					}

					writer.closeNode();
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
