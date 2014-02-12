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

using ImageSimilarity;
using PluginInterface;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Windows.Forms;

namespace OTItemEditor
{
	public partial class MainForm : Form
	{
		#region Private Properties

		public const string applicationName = "OTItemEditor";
		public const string versionString = "0.5.5";
		private const int itemMargin = 5;
		private const int spritePixels = 32;

		private bool showOnlyMismatchedItems = false;
		private bool showOnlyDeprecatedItems = false;
		private TextBoxTraceListener textBoxListener;
		private bool showUpdateOutput = true;
		private bool showOtbOutput = false;

		private OtbList items = new OtbList();
		private OtbItem currentItem = null;

		//The plugin that is used to compare, sync and display sprite/dat data
		public Host.Types.Plugin currentPlugin;
		public UInt32 currentOtbVersion = 0;
		string currentOtbFullPath = "";

		//The original plugin that was used to open the currently loaded OTB
		public Host.Types.Plugin previousPlugin;

		private bool loaded = false;
		private bool saved = true;
		private bool isTemp = false;

		#endregion

		#region Public Properties

		public bool Loaded
		{
			get { return this.loaded; }
		}

		public bool Saved
		{
			get { return this.saved; }
		}

		public UInt16 MinItemId
		{
			get { return items.minId; }
		}

		public UInt16 MaxItemId
		{
			get { return items.maxId; }
		}

		#endregion

		#region Construtor

		public MainForm()
		{
			InitializeComponent();
		}

		#endregion

		#region General Methods

		public void Open(string fileName = null)
		{
			if (String.IsNullOrEmpty(fileName))
			{
				FileDialog dialog = new OpenFileDialog();
				dialog.Filter = "OTB files (*.otb)|*.otb|All files (*.*)|*.*";
				dialog.Title = "Open OTB File";

				if (dialog.ShowDialog() != DialogResult.OK || dialog.FileName.Length == 0)
				{
					return;
				}

				fileName = dialog.FileName;
			}

			if (this.Loaded)
			{
				this.Clear();
			}

			if (Otb.open(fileName, ref items, showOtbOutput))
			{
				currentOtbFullPath = fileName;
				currentOtbVersion = items.dwMinorVersion;

				//try find a plugin that can handle this version of otb
				currentPlugin = Program.plugins.AvailablePlugins.Find(currentOtbVersion);
				if (currentPlugin == null)
				{
					items.Clear();
					MessageBox.Show(String.Format("Could not find a plugin that could handle client version {0}", currentOtbVersion));
					return;
				}

				if (!LoadClient(currentPlugin, currentOtbVersion))
				{
					currentPlugin = null;
					items.Clear();
					return;
				}

				this.BuildItemsListBox();
				this.fileSaveAsMenuItem.Enabled = true;
				this.fileSaveMenuItem.Enabled = true;
				this.editCreateItemMenuItem.Enabled = true;
				this.editFindItemMenuItem.Enabled = true;
				this.viewShowOnlyMismatchedMenuItem.Enabled = true;
				this.viewShowDecaptedItemsMenuItem.Enabled = true;
				this.viewUpdateItemsListMenuItem.Enabled = true;
				this.toolsUpdateVersionMenuItem.Enabled = true;
				this.toolsReloadItemAttributesMenuItem.Enabled = true;
				this.toolStripSaveButton.Enabled = true;
				this.toolStripSaveAsButton.Enabled = true;
				this.toolStripFindItemButton.Enabled = true;
				this.itemsListBox.Enabled = true;
				this.loaded = true;
			}
		}

		public void Save()
		{
			if (isTemp)
			{
				this.SaveAs();
				return;
			}

			if (!this.Loaded)
			{
				return;
			}

			try
			{
				Otb.save(currentOtbFullPath, ref items);
				saved = true;
				Trace.WriteLine("Saved.");
			}
			catch (UnauthorizedAccessException exception)
			{
				MessageBox.Show(exception.Message);
			}
		}

		public void SaveAs()
		{
			if (!this.Loaded)
			{
				return;
			}

			SaveFileDialog dialog = new SaveFileDialog();
			dialog.Filter = "OTB files (*.otb)|*.otb|All files (*.*)|*.*";
			dialog.Title = "Save OTB File";

			if (dialog.ShowDialog() == DialogResult.OK)
			{
				if (dialog.FileName.Length == 0)
				{
					return;
				}

				try
				{
					Otb.save(dialog.FileName, ref items);
					currentOtbFullPath = dialog.FileName;
					Trace.WriteLine("Saved.");
					this.isTemp = false;
					this.saved = true;
				}
				catch (UnauthorizedAccessException exception)
				{
					MessageBox.Show(exception.Message);
				}
			}
		}

		public bool SelectItem(UInt16 sid)
		{
			if (!this.Loaded)
			{
				return false;
			}

			if (sid >= items.minId && sid <= items.maxId)
			{
				OtbItem item = items.Find(i => i.id == sid);
				if (item != null)
				{
					return SelectItem(item);
				}
			}
			return false;
		}

		public bool SelectItem(OtbItem item)
		{
			if (!this.Loaded)
			{
				return false;
			}

			if (currentItem == item)
			{
				return true;
			}

			int index;
			if (item == null || (index = itemsListBox.Items.IndexOf(item)) == -1)
			{
				this.ResetControls();
				return false;
			}

			EditItem(item);
			editDuplicateItemMenuItem.Enabled = true;
			optionsGroupBox.Enabled = true;
			appearanceGroupBox.Enabled = true;
			itemsListBox.SelectedIndex = index;
			return true;
		}

		public OtbItem AddNewItem()
		{
			if (!this.Loaded)
			{
				return null;
			}

			OtbItem item = this.CreateItem();
			item.id = (UInt16)(items.maxId + 1);
			items.Add(item);
			itemsListBox.Items.Add(item);
			SelectItem(item);
			this.itemsCountLabel.Text = itemsListBox.Items.Count + " Items";

			if (showUpdateOutput)
			{
				Trace.WriteLine(String.Format("Create item id {0}", item.id));
			}
			return item;
		}

		public bool DuplicateItem(Item item)
		{
			return this.DuplicateItem(item as OtbItem);
		}

		public bool DuplicateItem(OtbItem item)
		{
			if (!this.Loaded || item == null)
			{
				return false;
			}

			OtbItem copyItem = this.CopyItem(item);
			copyItem.id = (UInt16)(items.maxId + 1);
			items.Add(copyItem);
			itemsListBox.Items.Add(copyItem);
			SelectItem(copyItem);
			this.itemsCountLabel.Text = itemsListBox.Items.Count + " Items";

			if (showUpdateOutput)
			{
				Trace.WriteLine(String.Format("Duplicate item id {0} to new item id {1}", item.id, copyItem.id));
			}
			return true;
		}

		public void CreateEmptyOTB(string filePath, SupportedClient client, bool isTemp = true)
		{
			OtbItem item = new OtbItem();
			item.spriteHash = new byte[16];
			item.spriteId = 100;
			item.id = 100;

			OtbList items = new OtbList();
			items.dwMajorVersion = 3;
			items.dwMinorVersion = client.otbVersion;
			items.dwBuildNumber = 1;
			items.clientVersion = client.version;
			items.Add(item);

			if (!File.Exists(filePath))
			{
				using (File.Create(filePath)) { }
			}

			if (Otb.save(filePath, ref items))
			{
				this.Open(filePath);
				this.isTemp = isTemp;
				this.saved = !isTemp;
			}
		}

		public void Clear()
		{
			this.SelectItem(null);
			this.currentItem = null;
			this.currentPlugin = null;
			this.previousPlugin = null;
			this.currentOtbVersion = 0;
			this.currentOtbFullPath = "";
			this.items.Clear();
			this.itemsListBox.Items.Clear();
			this.itemsListBox.Enabled = false;
			this.fileSaveMenuItem.Enabled = false;
			this.fileSaveAsMenuItem.Enabled = false;
			this.editCreateItemMenuItem.Enabled = false;
			this.editFindItemMenuItem.Enabled = false;
			this.viewShowOnlyMismatchedMenuItem.Enabled = false;
			this.viewShowDecaptedItemsMenuItem.Enabled = false;
			this.viewUpdateItemsListMenuItem.Enabled = false;
			this.toolsUpdateVersionMenuItem.Enabled = false;
			this.toolsReloadItemAttributesMenuItem.Enabled = false;
			this.toolStripSaveButton.Enabled = false;
			this.toolStripSaveAsButton.Enabled = false;
			this.toolStripFindItemButton.Enabled = false;
			this.loaded = false;
			this.textBoxListener.Clear();
		}

		private Bitmap GetBitmap(SpriteItem spriteItem)
		{
			int Width = spritePixels;
			int Height = spritePixels;

			if (spriteItem.width > 1 || spriteItem.height > 1)
			{
				Width = spritePixels * 2;
				Height = spritePixels * 2;
			}

			Bitmap canvas = new Bitmap(Width, Height, PixelFormat.Format24bppRgb);
			Graphics g = Graphics.FromImage(canvas);
			Rectangle rect = new Rectangle();
			//draw sprite
			for (int l = 0; l < spriteItem.layers; l++)
			{
				for (int h = 0; h < spriteItem.height; ++h)
				{
					for (int w = 0; w < spriteItem.width; ++w)
					{
						int frameIndex = w + h * spriteItem.width + l * spriteItem.width * spriteItem.height;
						Bitmap bmp = ImageUtils.getBitmap(spriteItem.GetRGBData(frameIndex), PixelFormat.Format24bppRgb, spritePixels, spritePixels);

						if (canvas.Width == spritePixels)
						{
							rect.X = 0;
							rect.Y = 0;
							rect.Width = bmp.Width;
							rect.Height = bmp.Height;
						}
						else
						{
							rect.X = Math.Max(spritePixels - w * spritePixels, 0);
							rect.Y = Math.Max(spritePixels - h * spritePixels, 0);
							rect.Width = bmp.Width;
							rect.Height = bmp.Height;
						}
						g.DrawImage(bmp, rect);
					}
				}
			}

			g.Save();
			return canvas;
		}

		private Bitmap GetSpriteBitmap(SpriteItem spriteItem)
		{
			int Width = spritePixels;
			int Height = spritePixels;

			if (spriteItem.width > 1 || spriteItem.height > 1)
			{
				Width = spritePixels * 2;
				Height = spritePixels * 2;
			}

			Bitmap canvas = new Bitmap(Width, Height, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(canvas))
			{
				g.FillRectangle(new SolidBrush(Color.FromArgb(0x11, 0x11, 0x11)), 0, 0, canvas.Width, canvas.Height);
				g.Save();
			}

			DrawSprite(ref canvas, spriteItem);

			Bitmap newImage = new Bitmap(Width, Height, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(newImage))
			{
				g.DrawImage(canvas, new Point(0, 0));
				g.Save();
			}

			newImage.MakeTransparent(Color.FromArgb(0x11, 0x11, 0x11));
			return newImage;
		}

		private void DrawSprite(ref Bitmap canvas, SpriteItem spriteItem)
		{
			Graphics g = Graphics.FromImage(canvas);
			Rectangle rect = new Rectangle();

			//draw sprite
			for (int l = 0; l < spriteItem.layers; l++)
			{
				for (int h = 0; h < spriteItem.height; ++h)
				{
					for (int w = 0; w < spriteItem.width; ++w)
					{
						int frameIndex = w + h * spriteItem.width + l * spriteItem.width * spriteItem.height;
						Bitmap bmp = ImageUtils.getBitmap(spriteItem.GetRGBData(frameIndex), PixelFormat.Format24bppRgb, spritePixels, spritePixels);

						if (canvas.Width == spritePixels)
						{
							rect.X = 0;
							rect.Y = 0;
							rect.Width = bmp.Width;
							rect.Height = bmp.Height;
						}
						else
						{
							rect.X = Math.Max(spritePixels - w * spritePixels, 0);
							rect.Y = Math.Max(spritePixels - h * spritePixels, 0);
							rect.Width = bmp.Width;
							rect.Height = bmp.Height;
						}
						g.DrawImage(bmp, rect);
					}
				}
			}

			g.Save();
		}

		private void DrawSprite(PictureBox picturBox, SpriteItem spriteItem)
		{
			Bitmap canvas = new Bitmap(64, 64, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(canvas))
			{
				g.FillRectangle(new SolidBrush(Color.FromArgb(0x11, 0x11, 0x11)), 0, 0, canvas.Width, canvas.Height);
				g.Save();
			}

			DrawSprite(ref canvas, spriteItem);

			Bitmap newImage = new Bitmap(64, 64, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(newImage))
			{
				g.DrawImage(canvas, new Point((canvas.Width > spritePixels ? 0 : spritePixels), (canvas.Height > spritePixels ? 0 : spritePixels)));
				g.Save();
			}

			newImage.MakeTransparent(Color.FromArgb(0x11, 0x11, 0x11));
			picturBox.Image = newImage;
		}

		private void BuildItemsListBox()
		{
			ClearItemsListBox();
			ResetControls();

			// Make the itemsListBox owner drawn.
			this.itemsListBox.DrawMode = DrawMode.OwnerDrawVariable;

			this.loadingItemsProgressBar.Visible = true;
			this.loadingItemsProgressBar.Minimum = items.minId;
			this.loadingItemsProgressBar.Maximum = items.maxId;
			this.loadingItemsProgressBar.Value = items.minId;

			foreach (OtbItem item in items)
			{
				if (showOnlyMismatchedItems && CompareItem(item, true))
				{
					continue;
				}

				if ((showOnlyDeprecatedItems && item.type != ItemType.Deprecated) ||
					(!showOnlyDeprecatedItems && item.type == ItemType.Deprecated))
				{
					continue;
				}

				itemsListBox.Items.Add(item);
				this.loadingItemsProgressBar.Value = item.id;
			}

			this.loadingItemsProgressBar.Visible = false;
			this.itemsCountLabel.Text = itemsListBox.Items.Count + " Items";
		}

		private void ClearItemsListBox()
		{
			itemsListBox.Items.Clear();
		}

		private bool CompareItem(OtbItem item, bool compareHash)
		{
			if (item.type == ItemType.Deprecated)
			{
				return true;
			}

			SpriteItem spriteItem;
			if (currentPlugin.Instance.Items.TryGetValue(item.spriteId, out spriteItem))
			{
				if (compareHash && !Utils.ByteArrayCompare(item.spriteHash, spriteItem.spriteHash))
				{
					return false;
				}

				return item.IsEqual(spriteItem);
			}

			return false;
		}

		private void ReloadItems()
		{
			foreach (OtbItem item in items)
			{
				if (!CompareItem(item, true))
				{
					ReloadItem(item);
				}
			}
		}

		private void ReloadItem(OtbItem item)
		{
			if (!Loaded || item == null)
			{
				return;
			}

			//to avoid problems with events
			OtbItem tmpItem = currentItem;
			currentItem = null;

			SpriteItem spriteItem;
			if (currentPlugin.Instance.Items.TryGetValue(item.spriteId, out spriteItem))
			{
				if (showUpdateOutput)
				{
					Trace.WriteLine(String.Format("Reloading item id: {0}.", item.id));
				}

				UInt16 tmpId = item.id;
				item.itemImpl = (ItemImpl)spriteItem.itemImpl.Clone();
				item.id = tmpId;
				Buffer.BlockCopy(spriteItem.spriteHash, 0, item.spriteHash, 0, spriteItem.spriteHash.Length);

				currentItem = tmpItem;
			}
		}

		private bool EditItem(OtbItem item)
		{
			currentItem = null;
			ResetDataBindings(this);
			ResetToolTips();

			if (item == null)
			{
				return false;
			}

			SpriteItem spriteItem;
			if (!currentPlugin.Instance.Items.TryGetValue(item.spriteId, out spriteItem))
			{
				return false;
			}

			DrawSprite(pictureBox, spriteItem);
			if (!item.isCustomCreated && item.spriteHash != null && spriteItem.spriteHash != null)
			{
				pictureBox.BackColor = ((Utils.ByteArrayCompare(item.spriteHash, spriteItem.spriteHash) ? Color.White : Color.Red));
			}

			typeCombo.Text = item.type.ToString();
			typeCombo.ForeColor = (item.type == spriteItem.type ? Color.Black : Color.Red);

			//
			serverIdLbl.DataBindings.Add("Text", item, "id");
			clientIdUpDown.Minimum = items.minId;
			clientIdUpDown.Maximum = (currentPlugin.Instance.Items.Count + items.minId) - 1;
			clientIdUpDown.DataBindings.Add("Value", spriteItem, "id");

			// Attributes
			AddBinding(unpassableCheck, "Checked", item, "isUnpassable", item.isUnpassable, spriteItem.isUnpassable);
			AddBinding(blockMissilesCheck, "Checked", item, "blockMissiles", item.blockMissiles, spriteItem.blockMissiles);
			AddBinding(blockPathfinderCheck, "Checked", item, "blockPathfinder", item.blockPathfinder, spriteItem.blockPathfinder);
			AddBinding(moveableCheck, "Checked", item, "isMoveable", item.isMoveable, spriteItem.isMoveable);
			AddBinding(hasElevationCheck, "Checked", item, "hasElevation", item.hasElevation, spriteItem.hasElevation);
			AddBinding(pickupableCheck, "Checked", item, "isPickupable", item.isPickupable, spriteItem.isPickupable);
			AddBinding(hangableCheck, "Checked", item, "isHangable", item.isHangable, spriteItem.isHangable);
			AddBinding(useableCheck, "Checked", item, "multiUse", item.multiUse, spriteItem.multiUse);
			AddBinding(rotatableCheck, "Checked", item, "isRotatable", item.isRotatable, spriteItem.isRotatable);
			AddBinding(stackableCheck, "Checked", item, "isStackable", item.isStackable, spriteItem.isStackable);
			AddBinding(verticalCheck, "Checked", item, "isVertical", item.isVertical, spriteItem.isVertical);
			AddBinding(fullGroundCheck, "Checked", item, "fullGround", item.fullGround, spriteItem.fullGround);
			AddBinding(horizontalCheck, "Checked", item, "isHorizontal", item.isHorizontal, spriteItem.isHorizontal);
			AddBinding(alwaysOnTopCheck, "Checked", item, "alwaysOnTop", item.alwaysOnTop, spriteItem.alwaysOnTop);
			AddBinding(readableCheck, "Checked", item, "isReadable", item.isReadable, spriteItem.isReadable);
			AddBinding(ignoreLookCheck, "Checked", item, "ignoreLook", item.ignoreLook, spriteItem.ignoreLook);
			AddBinding(groundSpeedText, "Text", item, "groundSpeed", item.groundSpeed, spriteItem.groundSpeed, true);
			AddBinding(topOrderText, "Text", item, "alwaysOnTopOrder", item.alwaysOnTopOrder, spriteItem.alwaysOnTopOrder, true);
			AddBinding(lightLevelText, "Text", item, "lightLevel", item.lightLevel, spriteItem.lightLevel, true);
			AddBinding(lightColorText, "Text", item, "lightColor", item.lightColor, spriteItem.lightColor, true);
			AddBinding(maxReadCharsText, "Text", item, "maxReadChars", item.maxReadChars, spriteItem.maxReadChars, true);
			AddBinding(maxReadWriteCharsText, "Text", item, "maxReadWriteChars", item.maxReadWriteChars, spriteItem.maxReadWriteChars, true);
			AddBinding(minimapColorText, "Text", item, "minimapColor", item.minimapColor, spriteItem.minimapColor, true);
			AddBinding(wareIdText, "Text", item, "tradeAs", item.tradeAs, spriteItem.tradeAs, true);
			AddBinding(nameText, "Text", item, "name", item.name, spriteItem.name, true);

			candidatesButton.Enabled = false;
			for (int i = 0; i < candidatesTableLayoutPanel.ColumnCount; ++i)
			{
				PictureBox box = (PictureBox)candidatesTableLayoutPanel.GetControlFromPosition(i, 0);
				box.Image = null;
			}
			
			if (previousPlugin != null)
			{
				SpriteItem prevSpriteItem;
				if (previousPlugin.Instance.Items.TryGetValue(item.prevSpriteId, out prevSpriteItem))
				{
					DrawSprite(previousPictureBox, prevSpriteItem);
					if (prevSpriteItem.spriteSignature != null)
					{
						//Sprite does not match, use the sprite signature to find possible candidates
						ShowSpriteCandidates(prevSpriteItem);
					}
				}
				else
				{
					previousPictureBox.Image = null;
				}
			}

			currentItem = item;
			return true;
		}

		private void AddBinding(Control control, string propertyName, object dataSource, string dataMember, object value, object clientValue, bool setToolTip = false)
		{
			bool equals = value.Equals(clientValue);
			control.DataBindings.Add(propertyName, dataSource, dataMember);
			control.ForeColor = equals ? Color.Black : Color.Red;

			if (!equals && setToolTip)
			{
				this.toolTip.SetToolTip(control, clientValue.ToString());
			}
		}

		private void ResetDataBindings(Control control)
		{
			control.DataBindings.Clear();
			if (control.HasChildren)
			{
				foreach (Control childControl in control.Controls)
				{
					ResetDataBindings(childControl);
				}
			}
		}

		private void ResetToolTips()
		{
			toolTip.RemoveAll();
		}

		private void ResetControls()
		{
			currentItem = null;
			editDuplicateItemMenuItem.Enabled = false;
			optionsGroupBox.Enabled = false;
			appearanceGroupBox.Enabled = false;
			pictureBox.Image = null;
			pictureBox.BackColor = Color.White;
			previousPictureBox.Image = null;
			previousPictureBox.BackColor = Color.White;
			clientIdUpDown.Value = clientIdUpDown.Minimum;
			serverIdLbl.Text = "0";
			typeCombo.Text = "";
			typeCombo.ForeColor = Color.Black;
			editDuplicateItemMenuItem.Enabled = false;
			candidatesButton.Enabled = false;

			foreach (Control control in optionsGroupBox.Controls)
			{
				if (control is CheckBox)
				{
					((CheckBox)control).Checked = false;
					control.ForeColor = Color.Black;
				}
				else if (control is TextBox)
				{
					((TextBox)control).Text = "";
					control.ForeColor = Color.Black;
				}
			}
		}

		private void ShowSpriteCandidates(SpriteItem spriteItem)
		{
			candidatesButton.Enabled = true;

			//list with the top 5 results
			List<KeyValuePair<double, OtbItem>> signatureList = new List<KeyValuePair<double, OtbItem>>();

			foreach (OtbItem cmpItem in items)
			{
				if (cmpItem.type == ItemType.Deprecated)
				{
					continue;
				}

				SpriteItem cmpSpriteItem;
				if (!currentPlugin.Instance.Items.TryGetValue(cmpItem.spriteId, out cmpSpriteItem))
				{
					continue;
				}

				double similarity = ImageUtils.CompareSignature(spriteItem.spriteSignature, cmpSpriteItem.spriteSignature);

				foreach (KeyValuePair<double, OtbItem> kvp in signatureList)
				{
					if (similarity < kvp.Key)
					{
						//TODO: Use isEqual aswell to match against attributes.
						signatureList.Remove(kvp);
						break;
					}
				}

				if (signatureList.Count < 5)
				{
					KeyValuePair<double, OtbItem> kvp = new KeyValuePair<double, OtbItem>(similarity, cmpItem);
					signatureList.Add(kvp);
				}
			}

			signatureList.Sort(
				delegate(KeyValuePair<double, OtbItem> item1, KeyValuePair<double, OtbItem> item2)
				{
					return item1.Key.CompareTo(item2.Key);
				});

			//those with lowest value are the closest match
			int index = 0;
			foreach (KeyValuePair<double, OtbItem> kvp in signatureList)
			{
				PictureBox box = (PictureBox)candidatesTableLayoutPanel.GetControlFromPosition(index, 0);
				toolTip.SetToolTip(box, kvp.Value.spriteId.ToString());
				box.Tag = kvp.Value;

				SpriteItem spriteCandidateItem;
				if (currentPlugin.Instance.Items.TryGetValue(kvp.Value.spriteId, out spriteCandidateItem))
				{
					DrawSprite(box, spriteCandidateItem);
				}
				++index;
			}
		}

		private OtbItem CreateItem(Item item = null)
		{
			//create a new otb item
			OtbItem newItem = new OtbItem(item);
			newItem.id = (UInt16)(items.maxId + 1);
			newItem.spriteHash = new byte[16];

			if (item != null)
			{
				newItem.spriteId = item.id;
				Buffer.BlockCopy(item.spriteHash, 0, newItem.spriteHash, 0, newItem.spriteHash.Length);
			}
			else
			{
				newItem.spriteId = items.minId;
				newItem.isCustomCreated = true;
			}

			return newItem;
		}

		private OtbItem CopyItem(OtbItem item)
		{
			if (item == null)
			{
				return null;
			}

			OtbItem copy = new OtbItem(item);
			copy.spriteHash = new byte[16];
			copy.spriteId = item.spriteId;
			Buffer.BlockCopy(item.spriteHash, 0, copy.spriteHash, 0, copy.spriteHash.Length);
			return copy;
		}

		private bool LoadClient(Host.Types.Plugin plugin, UInt32 otbVersion)
		{
			SupportedClient client = plugin.Instance.SupportedClients.Find(
				delegate(SupportedClient sc)
				{
					return sc.otbVersion == otbVersion;
				});

			if (client == null)
			{
				MessageBox.Show("The selected plugin does not support this version.");
				return false;
			}

			string dataFolder = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "data");
			if (!Directory.Exists(dataFolder))
			{
				Directory.CreateDirectory(dataFolder);
			}

			string datPath = FindClientFile(Path.Combine(dataFolder, client.version.ToString()), ".dat");
			string sprPath = FindClientFile(Path.Combine(dataFolder, client.version.ToString()), ".spr");

			if (!File.Exists(datPath) || !File.Exists(sprPath))
			{
				string text = String.Empty;

				if (!File.Exists(datPath))
				{
					text = String.Format("Unable to load dat file, please place a valid dat in 'data\\{0}\\'.", client.version);
				}

				if (!File.Exists(sprPath))
				{
					if (text != String.Empty)
					{
						text += "\n";
					}
					text += String.Format("Unable to load spr file, please place a valid spr in 'data\\{0}\\'.", client.version);
				}

				MessageBox.Show(text);
				return false;
			}

			Trace.WriteLine(String.Format("OTB version {0}.", otbVersion));

			bool result = plugin.Instance.LoadClient(client, datPath, sprPath);
			Trace.WriteLine("Loading client files.");
			if (!result)
			{
				MessageBox.Show(String.Format("The plugin could not load dat or spr."));
			}

			items.clientVersion = client.version;
			Trace.WriteLine(String.Format("Client version {0}.", client.version));
			return result;
		}

		private string FindClientFile(string path, string extension)
		{
			if (Directory.Exists(path))
			{
				foreach (string fileOn in Directory.GetFiles(path))
				{
					FileInfo file = new FileInfo(fileOn);
					if (file.Extension.Equals(extension))
					{
						return file.FullName;
					}
				}
			}
			return String.Empty;
		}

		private bool GenerateSpriteSignatures(ref SpriteItems items)
		{
			if (items.signatureCalculated)
			{
				return true;
			}

			ProgressForm progress = new ProgressForm();
			progress.StartPosition = FormStartPosition.Manual;
			progress.Location = new Point(Location.X + ((Width - progress.Width) / 2),
										  Location.Y + ((Height - progress.Height) / 2));
			progress.bar.Minimum = 0;
			progress.bar.Maximum = items.Count;
			progress.Show(this);

			foreach (SpriteItem spriteItem in items.Values)
			{
				Bitmap spriteBmp = GetBitmap(spriteItem);
				Bitmap ff2dBmp = Fourier.fft2dRGB(spriteBmp, false);
				spriteItem.spriteSignature = ImageUtils.CalculateEuclideanDistance(ff2dBmp, 1);

				if (progress.bar.Value % 20 == 0)
				{
					Application.DoEvents();
				}
				progress.progressLbl.Text = String.Format("Calculating image signature for item {0}.", spriteItem.id);
				++progress.bar.Value;
			}

			items.signatureCalculated = true;
			progress.Close();
			return true;
		}

		private bool CheckSave()
		{
			if (this.Loaded && !this.Saved)
			{
				DialogResult result = MessageBox.Show("Do you want to save the changes?", "Save", MessageBoxButtons.YesNoCancel);

				if (result == DialogResult.Yes)
				{
					this.Save();
				}
				else if (result == DialogResult.Cancel)
				{
					return false;
				}
			}
			return true;
		}

		#endregion

		#region Event Handlers

		private void MainForm_Load(object sender, EventArgs e)
		{
			this.Text = applicationName + " " + versionString;
			typeCombo.DataSource = Enum.GetNames(typeof(ItemType));

			this.candidatesDropDown.Items.Add(new ToolStripControlHost(this.candidatesTableLayoutPanel));

			Trace.Listeners.Clear();
			textBoxListener = new TextBoxTraceListener(outputTextBox);
			Trace.Listeners.Add(textBoxListener);

			toolsShowTraceUpdateMenuItem.Checked = showUpdateOutput;
			toolsShowTraceOTBMenuItem.Checked = showOtbOutput;
			SelectItem(null);

			Program.plugins.FindPlugins();
		}

		private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			if (!CheckSave())
			{
				e.Cancel = true;
			}
		}

		private void fileNewMenuItem_Click(object sender, EventArgs e)
		{
			NewOtbFileForm newOtbForm = new NewOtbFileForm();
			DialogResult result = newOtbForm.ShowDialog();

			if (result == DialogResult.OK)
			{
				this.CreateEmptyOTB(newOtbForm.FilePath, newOtbForm.SelectedClient);
			}
		}

		private void fileOpenMenuItem_Click(object sender, EventArgs e)
		{
			this.Open();
		}

		private void fileSaveMenuItem_Click(object sender, EventArgs e)
		{
			this.Save();
		}

		private void fileSaveAsMenuItem_Click(object sender, EventArgs e)
		{
			this.SaveAs();
		}

		private void fileExitMenuItem_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void itemsListBoxContextMenu_Opening(object sender, CancelEventArgs e)
		{
			this.itemsListBoxContextMenu.Items.Clear();
			if (this.Loaded)
			{
				this.itemsListBoxContextMenu.Items.Add("Duplicate");
				this.itemsListBoxContextMenu.Items.Add("Reload");
			}
		}

		private void itemsListBoxContextMenu_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
		{
			ToolStripItem menuItem = e.ClickedItem;
			switch (menuItem.Text)
			{
				case "Duplicate":
					DuplicateItem(currentItem);
					break;
				case "Reload":
					OtbItem item = currentItem;
					ReloadItem(currentItem);
					SelectItem(null);
					SelectItem(item);
					break;
			}
		}

		private void itemsListBox_DrawItem(object sender, DrawItemEventArgs e)
		{
			if (e.Index == -1)
			{
				return;
			}

			ListBox list = sender as ListBox;
			OtbItem item = (OtbItem)list.Items[e.Index];

			e.DrawBackground();
			e.Graphics.DrawRectangle(Pens.Gray, e.Bounds); // Border.

			Brush brush;
			if ((e.State & DrawItemState.Selected) == DrawItemState.Selected)
			{
				brush = SystemBrushes.HighlightText;
			}
			else
			{
				brush = new SolidBrush(e.ForeColor);
			}

			// Find the area in which to put the text and draw.
			int x = e.Bounds.Left + spritePixels + 3 * itemMargin;
			int y = e.Bounds.Top + itemMargin * 2;
			int width = e.Bounds.Right - itemMargin - x;
			int height = e.Bounds.Bottom - itemMargin - y;
			Rectangle layoutRect = new Rectangle(x, y, width, height);
			string text = item.id.ToString();
			e.Graphics.DrawString(text, this.Font, brush, layoutRect);

			Rectangle destRect = new Rectangle(e.Bounds.Left + itemMargin, e.Bounds.Top + itemMargin, spritePixels, spritePixels);

			SpriteItem spriteItem;
			if (currentPlugin.Instance.Items.TryGetValue(item.spriteId, out spriteItem))
			{
				Bitmap bitmap = GetSpriteBitmap(spriteItem);
				if (bitmap != null)
				{
					Rectangle sourceRect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);
					e.Graphics.DrawImage(bitmap, destRect, sourceRect, GraphicsUnit.Pixel);
				}
			}

			e.Graphics.DrawRectangle(new Pen(brush), destRect);
			e.DrawFocusRectangle();
		}

		private void itemsListBox_MeasureItem(object sender, MeasureItemEventArgs e)
		{
			e.ItemHeight = (int)(spritePixels + 2 * itemMargin);
		}

		private void itemsListBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			SelectItem(itemsListBox.SelectedItem as OtbItem);
		}

		private void itemsListBox_MouseDown(object sender, MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Right)
			{
				itemsListBox.SelectedIndex = itemsListBox.IndexFromPoint(e.Location);
				itemsListBoxContextMenu.Show();
			}
		}

		private void textBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (!Char.IsDigit(e.KeyChar) && e.KeyChar != '\b')
			{
				e.Handled = true;
			}
		}

		private void toolsCompareOtbFilesMenuItem_Click(object sender, EventArgs e)
		{
			CompareOtbForm form = new CompareOtbForm();
			form.ShowDialog();
		}

		private void showOnlyUnmatchedToolStripMenuItem_Click(object sender, EventArgs e)
		{
			showOnlyMismatchedItems = !showOnlyMismatchedItems;
			BuildItemsListBox();
		}

		private void viewShowDecaptedItemsMenuItem_Click(object sender, EventArgs e)
		{
			showOnlyDeprecatedItems = !showOnlyDeprecatedItems;
			BuildItemsListBox();
		}

		private void toolsShowTraceUpdateMenuItem_Click(object sender, EventArgs e)
		{
			showUpdateOutput = !showUpdateOutput;
		}

		private void toolsShowTraceOTBMenuItem_Click(object sender, EventArgs e)
		{
			showOtbOutput = !showOtbOutput;
		}

		private void toolsReloadItemAttributesMenuItem_Click(object sender, EventArgs e)
		{
			ReloadItems();
			EditItem(currentItem);
			BuildItemsListBox();
		}

		private void toolsUpdateVersionMenuItem_Click(object sender, EventArgs e)
		{
			UpdateForm form = new UpdateForm();
			form.mainForm = this;

			DialogResult result = form.ShowDialog();
			if (result == DialogResult.OK)
			{
				//Update OTB
				Host.Types.Plugin updatePlugin = form.selectedPlugin;
				SupportedClient updateClient = form.updateClient;

				if (updatePlugin == null)
				{
					return;
				}

				if (!LoadClient(updatePlugin, updateClient.otbVersion))
				{
					return;
				}

				UpdateSettingsForm updateSettingsForm = new UpdateSettingsForm();
				result = updateSettingsForm.ShowDialog();
				if (result != DialogResult.OK)
				{
					return;
				}

				if (updateSettingsForm.generateSignatureCheck.Checked)
				{
					//Calculate an image signature using fourier transformation and calculate a signature we can
					//use to compare it to other images (kinda similar to md5 hash) except this
					//can also be used to find images with some variation.
					SpriteItems currentSpriteItems = currentPlugin.Instance.Items;
					GenerateSpriteSignatures(ref currentSpriteItems);

					SpriteItems updateSpriteItems = updatePlugin.Instance.Items;
					GenerateSpriteSignatures(ref updateSpriteItems);
				}

				SpriteItems currentItems = currentPlugin.Instance.Items;
				SpriteItems updateItems = updatePlugin.Instance.Items;
				List<UInt16> assignedSpriteIdList = new List<UInt16>();

				//store the previous plugin (so we can display previous sprite, and do other comparisions)
				previousPlugin = currentPlugin;

				//update the current plugin the one we are updating to
				currentPlugin = updatePlugin;

				//update version information
				items.clientVersion = updateClient.version;
				items.dwMinorVersion = updateClient.otbVersion;
				items.dwBuildNumber = items.dwBuildNumber + 1;
				currentOtbVersion = items.dwMinorVersion;

				//Most items does have the same sprite after an update, so lets try that first
				UInt32 foundItemCounter = 0;
				foreach (OtbItem item in items)
				{
					item.spriteAssigned = false;

					if (item.type == ItemType.Deprecated)
					{
						continue;
					}

					SpriteItem updateSpriteItem;
					if (updateItems.TryGetValue(item.spriteId, out updateSpriteItem))
					{
						bool compareResult = updateSpriteItem.IsEqual(item);

						if (Utils.ByteArrayCompare(updateSpriteItem.spriteHash, item.spriteHash))
						{
							if (compareResult)
							{
								item.prevSpriteId = item.spriteId;
								item.spriteId = updateSpriteItem.id;
								item.spriteAssigned = true;

								assignedSpriteIdList.Add(updateSpriteItem.id);
								++foundItemCounter;

								if (showUpdateOutput)
								{
									//Trace.WriteLine(String.Format("Match found id: {0}, clientid: {1}", item.otb.id, item.dat.id));
								}
							}
							else
							{
								//Sprite matches, but not the other attributes.
								if (showUpdateOutput)
								{
									Trace.WriteLine(String.Format("Attribute changes found id: {0}.", item.id));
								}
							}
						}
					}
				}

				if (updateSettingsForm.reassignUnmatchedSpritesCheck.Checked)
				{
					foreach (Item updateItem in updateItems.Values)
					{
						foreach (OtbItem item in items)
						{
							if (item.type == ItemType.Deprecated)
							{
								continue;
							}

							if (item.spriteAssigned)
							{
								continue;
							}

							if (Utils.ByteArrayCompare(updateItem.spriteHash, item.spriteHash))
							{
								if (updateItem.IsEqual(item))
								{
									if (updateItem.id != item.spriteId)
									{
										if (showUpdateOutput)
										{
											Trace.WriteLine(String.Format("New sprite found id: {0}, old: {1}, new: {2}.", item.id, item.spriteId, updateItem.id));
										}
									}

									item.prevSpriteId = item.spriteId;
									item.spriteId = updateItem.id;
									item.spriteAssigned = true;

									assignedSpriteIdList.Add(updateItem.id);
									++foundItemCounter;
									break;
								}
							}
						}
					}
				}

				if (showUpdateOutput)
				{
					Trace.WriteLine(String.Format("Found {0} of {1}.", foundItemCounter, items.maxId));
				}

				if (updateSettingsForm.reloadItemAttributesCheck.Checked)
				{
					UInt32 reloadedItemCounter = 0;
					foreach (OtbItem item in items)
					{
						if (item.type == ItemType.Deprecated)
						{
							continue;
						}

						//implicit assigned
						item.prevSpriteId = item.spriteId;
						item.spriteAssigned = true;

						if (!assignedSpriteIdList.Contains(item.spriteId))
						{
							assignedSpriteIdList.Add(item.spriteId);
						}

						if (!CompareItem(item, true))
						{
							//sync with dat info
							ReloadItem(item);
							++reloadedItemCounter;
						}
					}

					if (showUpdateOutput)
					{
						Trace.WriteLine(String.Format("Reloaded {0} of {1} items.", reloadedItemCounter, items.maxId));
					}
				}

				if (updateSettingsForm.createNewItemsCheck.Checked)
				{
					UInt32 newItemCounter = 0;
					foreach (Item updateItem in updateItems.Values)
					{
						if (!assignedSpriteIdList.Contains(updateItem.id))
						{
							++newItemCounter;
							OtbItem newItem = CreateItem(updateItem);
							items.Add(newItem);
							if (showUpdateOutput)
							{
								Trace.WriteLine(String.Format("Creating item id {0}", newItem.id));
							}
						}
					}

					if (showUpdateOutput)
					{
						Trace.WriteLine(String.Format("Created {0} new items.", newItemCounter));
					}
				}

				//done
				BuildItemsListBox();
			}
		}

		private void candidatePictureBox_Click(object sender, EventArgs e)
		{
			if (currentItem != null)
			{
				PictureBox box = (PictureBox)sender;
				if (box.Tag is OtbItem)
				{
					OtbItem newItem = (OtbItem)box.Tag;

					SpriteItem spriteItem;
					if (!currentPlugin.Instance.Items.TryGetValue(newItem.spriteId, out spriteItem))
					{
						return;
					}

					if (!spriteItem.IsEqual(currentItem))
					{
						DialogResult result = MessageBox.Show(
							"The item attributes does not match the current information, would you like to continue anyway?",
							"Item attributes does not match",
							MessageBoxButtons.YesNo,
							MessageBoxIcon.Question,
							MessageBoxDefaultButton.Button1);

						if (result != DialogResult.Yes)
						{
							return;
						}
					}

					currentItem.prevSpriteId = currentItem.spriteId;
					currentItem.spriteId = spriteItem.id;
					EditItem(currentItem);
				}
			}
		}

		private void clientIdUpDown_ValueChanged(object sender, EventArgs e)
		{
			if (currentItem != null)
			{
				SpriteItem newSpriteItem;
				if (currentPlugin.Instance.Items.TryGetValue((UInt16)clientIdUpDown.Value, out newSpriteItem))
				{
					currentItem.prevSpriteId = currentItem.spriteId;
					currentItem.spriteId = newSpriteItem.id;
					EditItem(currentItem);
				}
			}
		}

		private void helpAboutMenuItem_Click(object sender, EventArgs e)
		{
			MessageBox.Show("Information how to use this program check http://www.otfans.net.",
				"About " + applicationName,
				MessageBoxButtons.OK,
				MessageBoxIcon.None,
				MessageBoxDefaultButton.Button1,
				0);
		}

		private void editDuplicateItemMenuItem_Click(object sender, EventArgs e)
		{
			this.DuplicateItem(currentItem);
		}

		private void editCreateItemMenuItem_Click(object sender, EventArgs e)
		{
			this.AddNewItem();
		}

		private void typeCombo_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (currentItem != null)
			{
				currentItem.type = (ItemType)Enum.Parse(typeof(ItemType), typeCombo.SelectedValue.ToString());
			}
		}

		private void viewUpdateItemsListMenuItem_Click(object sender, EventArgs e)
		{
			this.ResetControls();
			this.BuildItemsListBox();
		}

		private void editFindItemMenuItem_Click(object sender, EventArgs e)
		{
			FindItemForm form = new FindItemForm();
			form.MainForm = this;
			form.Show(this);
		}

		private void candidatesButton_Click(object sender, EventArgs e)
		{
			this.candidatesDropDown.Show(this, new Point(192, 355));
		}

		#endregion
	}
}
