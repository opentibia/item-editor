using System;
using System.Collections.Generic;
using System.Text;
using otitemeditor;
using System.IO;

namespace otitemeditor
{
	public enum ItemType
	{
		None,
		Ground,
		Container,
		Fluid,
		Splash,
		Deprecated
	};

	public class ItemImpl : ICloneable
	{
		public object Clone()
		{
			ItemImpl clone = (ItemImpl)this.MemberwiseClone();
			return clone;
		}

		public UInt16 id;
		public UInt16 groundSpeed;
		public ItemType type;
		public bool alwaysOnTop;
		public UInt16 alwaysOnTopOrder;
		public bool hasUseWith;
		public UInt16 maxReadChars;
		public UInt16 maxReadWriteChars;
		public bool hasHeight;
		public UInt16 minimapColor;
		public bool lookThrough;
		public UInt16 lightLevel;
		public UInt16 lightColor;
		public bool isStackable;
		public bool isReadable; 
		public bool isMoveable;
		public bool isPickupable;
		public bool isHangable;
		public bool isHorizontal;
		public bool isVertical;
		public bool isRotatable;
		public bool blockObject;
		public bool blockProjectile;
		public bool blockPathFind;
		public bool allowDistRead;
		public bool isAnimation;
	    public bool walkStack;
	    public UInt16 wareId;
	    public string name;
	}

	public class Item
	{
		public ItemImpl itemImpl = new ItemImpl();

		public Item()
		{
			isMoveable = true;
		    name = "";
		}

		public virtual bool isEqual(Item item)
		{
			if (type != item.type)
			{
				return false;
			}

			/*
			if (compareSprite && !Utils.ByteArrayCompare(spriteHash, item.spriteHash))
			{
				return false;
			}
			*/

            if (name.CompareTo(item.name) != 0)
            {
                return false;
            }

            if (wareId != item.wareId)
            {
                return false;
            }

            if (walkStack != item.walkStack)
            {
                return false;
            }

			if (isAnimation != item.isAnimation)
			{
				return false;
			}

			if (alwaysOnTop != item.alwaysOnTop)
			{
				return false;
			}

			if (alwaysOnTopOrder != item.alwaysOnTopOrder)
			{
				return false;
			}

			if (blockObject != item.blockObject)
			{
				return false;
			}

			if (blockPathFind != item.blockPathFind)
			{
				return false;
			}

			if (blockProjectile != item.blockProjectile)
			{
				return false;
			}

			if (groundSpeed != item.groundSpeed)
			{
				return false;
			}

			if (hasHeight != item.hasHeight)
			{
				return false;
			}

			if (hasUseWith != item.hasUseWith)
			{
				return false;
			}

			if (isHangable != item.isHangable)
			{
				return false;
			}

			if (isHorizontal != item.isHorizontal)
			{
				return false;
			}

			if (isMoveable != item.isMoveable)
			{
				return false;
			}

			if (isPickupable != item.isPickupable)
			{
				return false;
			}

			if (isReadable != item.isReadable)
			{
				return false;
			}

			if (isRotatable != item.isRotatable)
			{
				return false;
			}

			if (isStackable != item.isStackable)
			{
				return false;
			}

			if (isVertical != item.isVertical)
			{
				return false;
			}

			if (lightColor != item.lightColor)
			{
				return false;
			}

			if (lightLevel != item.lightLevel)
			{
				return false;
			}

			if (lookThrough != item.lookThrough)
			{
				return false;
			}

			if (maxReadChars != item.maxReadChars)
			{
				return false;
			}

			if (maxReadWriteChars != item.maxReadWriteChars)
			{
				return false;
			}

			if (minimapColor != item.minimapColor)
			{
				return false;
			}

			return true;
		}

		public UInt16 id { get { return itemImpl.id; } set { itemImpl.id = value; } }
		public UInt16 groundSpeed { get { return itemImpl.groundSpeed; } set { itemImpl.groundSpeed = value; } }
		public ItemType type { get { return itemImpl.type; } set { itemImpl.type = value; } }
		public bool alwaysOnTop { get { return itemImpl.alwaysOnTop; } set { itemImpl.alwaysOnTop = value; } }
		public UInt16 alwaysOnTopOrder { get { return itemImpl.alwaysOnTopOrder; } set { itemImpl.alwaysOnTopOrder = value; } }
		public bool hasUseWith { get { return itemImpl.hasUseWith; } set { itemImpl.hasUseWith = value; } }
		public UInt16 maxReadChars { get { return itemImpl.maxReadChars; } set { itemImpl.maxReadChars = value; } }
		public UInt16 maxReadWriteChars { get { return itemImpl.maxReadWriteChars; } set { itemImpl.maxReadWriteChars = value; } }
		public bool hasHeight { get { return itemImpl.hasHeight; } set { itemImpl.hasHeight = value; } }
		public UInt16 minimapColor { get { return itemImpl.minimapColor; } set { itemImpl.minimapColor = value; } }
		public bool lookThrough { get { return itemImpl.lookThrough; } set { itemImpl.lookThrough = value; } }
		public UInt16 lightLevel { get { return itemImpl.lightLevel; } set { itemImpl.lightLevel = value; } }
		public UInt16 lightColor { get { return itemImpl.lightColor; } set { itemImpl.lightColor = value; } }
		public bool isStackable { get { return itemImpl.isStackable; } set { itemImpl.isStackable = value; } }
		public bool isReadable { get { return itemImpl.isReadable; } set { itemImpl.isReadable = value; } }
		public bool isMoveable { get { return itemImpl.isMoveable; } set { itemImpl.isMoveable = value; } }
		public bool isPickupable { get { return itemImpl.isPickupable; } set { itemImpl.isPickupable = value; } }
		public bool isHangable { get { return itemImpl.isHangable; } set { itemImpl.isHangable = value; } }
		public bool isHorizontal { get { return itemImpl.isHorizontal; } set { itemImpl.isHorizontal = value; } }
		public bool isVertical { get { return itemImpl.isVertical; } set { itemImpl.isVertical = value; } }
		public bool isRotatable { get { return itemImpl.isRotatable; } set { itemImpl.isRotatable = value; } }
		public bool blockObject { get { return itemImpl.blockObject; } set { itemImpl.blockObject = value; } }
		public bool blockProjectile { get { return itemImpl.blockProjectile; } set { itemImpl.blockProjectile = value; } }
		public bool blockPathFind { get { return itemImpl.blockPathFind; } set { itemImpl.blockPathFind = value; } }
		public bool allowDistRead { get { return itemImpl.allowDistRead; } set { itemImpl.allowDistRead = value; } }
		public bool isAnimation { get { return itemImpl.isAnimation; } set { itemImpl.isAnimation = value; } }
        public bool walkStack { get { return itemImpl.walkStack; } set { itemImpl.walkStack = value; } }
        public string name
        {
            get { return itemImpl.name; }
            set { itemImpl.name = value; }
        }
        public UInt16 wareId
        {
            get { return itemImpl.wareId; }
            set { itemImpl.wareId = value; }
        }

		//used to find sprites during updates
		protected byte[] _spriteHash = null;
		public virtual byte[] spriteHash
		{
			get {return _spriteHash;}
			set {_spriteHash = value;}
		}
	}

	public class SpriteItem : Item
	{
		//sprite meta-data
		public byte width;
		public byte height;
		public byte frames;
		public byte xdiv;
		public byte ydiv;
		public byte zdiv;
		public byte animationLength;
		public UInt32 numSprites;
		public List<Sprite> spriteList = new List<Sprite>();

		public override byte[] spriteHash
		{
			get
			{
				if (_spriteHash == null)
				{
					System.Security.Cryptography.MD5 md5 = System.Security.Cryptography.MD5.Create();

					Int32 spriteSize = (Int32)width * (Int32)height * (Int32)animationLength;
					Int32 spriteBase = 0;

					MemoryStream stream = new MemoryStream();

					for (Int32 frame = 0; frame < frames; frame++)
					{
						for (Int32 cy = 0; cy < height; cy++)
						{
							for (Int32 cx = 0; cx < width; cx++)
							{
								Int32 frameIndex = spriteBase + cx + cy * width + frame * width * height;
								Sprite sprite = spriteList[frameIndex];
								if (sprite != null)
								{
									stream.Write(sprite.getRGBAData(), 0, 32 * 32 * 4);
								}
							}
						}
					}

					stream.Position = 0;
					_spriteHash = md5.ComputeHash(stream);
				}

				return _spriteHash;
			}

			set
			{
				_spriteHash = value;
			}
		}

		//contains sprite signatures using Euclidean distance (4x4 blocks) on a ff2d generated image of the sprite
		private double[,] _spriteSignature = null;
		public double[,] spriteSignature
		{
			get {return _spriteSignature;}
			set { _spriteSignature = value; }
		}

		//Used to calculate fourier transformation
		public byte[] getRGBData()
		{
			return spriteList[0].getRGBData();
		}

		public byte[] getRGBData(int frameIndex)
		{
			return spriteList[frameIndex].getRGBData();
		}

		//used for drawing and calculating MD5
		public byte[] getRGBAData()
		{
			return spriteList[0].getRGBAData();
		}

		public byte[] getRGBAData(int frameIndex)
		{
			return spriteList[frameIndex].getRGBAData();
		}
	}
}
