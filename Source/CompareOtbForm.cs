using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using PluginInterface;

namespace otitemeditor
{
	public partial class CompareOtbForm : Form
	{
		public CompareOtbForm()
		{
			InitializeComponent();
		}

		private bool compareItems()
		{
			if (System.IO.File.Exists(file1Text.Text) && System.IO.File.Exists(file2Text.Text))
			{
				OtbList items1 = new OtbList();
				OtbList items2 = new OtbList();

				bool result;
				result = otb.open(file1Text.Text, ref items1, false);
				if (!result)
				{
					MessageBox.Show("Could not open {0}", file1Text.Text);
					return false;
				}

				result = otb.open(file2Text.Text, ref items2, false);
				if (!result)
				{
					MessageBox.Show("Could not open {0}", file2Text.Text);
					return false;
				}

				IEnumerator<OtbItem> enumerator1 = items1.GetEnumerator();
				IEnumerator<OtbItem> enumerator2 = items2.GetEnumerator();

				if (items1.Count != items2.Count)
				{
					resultTextBox.AppendText(string.Format("Item count:  [{0}]/[{1}]" + Environment.NewLine, items1.Count, items2.Count));
				}

				while(enumerator1.MoveNext()){
					if(!enumerator2.MoveNext()){
						return false;
					}

					OtbItem item1 = enumerator1.Current;
					OtbItem item2 = enumerator2.Current;

					if (item1.spriteId != item2.spriteId)
					{
						resultTextBox.AppendText(string.Format("id: {0} Sprite changed [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.spriteId, item2.spriteId));
						continue;
					}

					if (item1.spriteHash != null && item2.spriteHash != null && !Utils.ByteArrayCompare(item1.spriteHash, item2.spriteHash))
					{
						resultTextBox.AppendText(string.Format("id: {0} Sprite updated" + Environment.NewLine, item1.id));
					}

					if (item1.type != item2.type)
					{
						resultTextBox.AppendText(string.Format("id: {0} type [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.type, item2.type));
					}

					if (item1.alwaysOnTop != item2.alwaysOnTop)
					{
						resultTextBox.AppendText(string.Format("id: {0} alwaysOnTop [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.alwaysOnTop, item2.alwaysOnTop));
					}

					if (item1.alwaysOnTopOrder != item2.alwaysOnTopOrder)
					{
						resultTextBox.AppendText(string.Format("id: {0} alwaysOnTopOrder [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.alwaysOnTopOrder, item2.alwaysOnTopOrder));
					}

					if (item1.blockObject != item2.blockObject)
					{
						resultTextBox.AppendText(string.Format("id: {0} blockObject [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.blockObject, item2.blockObject));
					}

					if (item1.blockPathFind != item2.blockPathFind)
					{
						resultTextBox.AppendText(string.Format("id: {0} blockPathFind [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.blockPathFind, item2.blockPathFind));
					}

					if (item1.blockProjectile != item2.blockProjectile)
					{
						resultTextBox.AppendText(string.Format("id: {0} blockProjectile [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.blockProjectile, item2.blockProjectile));
					}

					if (item1.groundSpeed != item2.groundSpeed)
					{
						resultTextBox.AppendText(string.Format("id: {0} groundSpeed [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.groundSpeed, item2.groundSpeed));
					}

					if (item1.hasHeight != item2.hasHeight)
					{
						resultTextBox.AppendText(string.Format("id: {0} hasHeight [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.hasHeight, item2.hasHeight));
					}

					if (item1.hasUseWith != item2.hasUseWith)
					{
						resultTextBox.AppendText(string.Format("id: {0} Useable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.hasUseWith, item2.hasUseWith));
					}

					if (item1.isHangable != item2.isHangable)
					{
						resultTextBox.AppendText(string.Format("id: {0} isHangable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isHangable, item2.isHangable));
					}

					if (item1.isHorizontal != item2.isHorizontal)
					{
						resultTextBox.AppendText(string.Format("id: {0} isHorizontal [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isHorizontal, item2.isHorizontal));
					}

					if (item1.isMoveable != item2.isMoveable)
					{
						resultTextBox.AppendText(string.Format("id: {0} isMoveable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isMoveable, item2.isMoveable));
					}

					if (item1.isPickupable != item2.isPickupable)
					{
						resultTextBox.AppendText(string.Format("id: {0} isPickupable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isPickupable, item2.isPickupable));
					}

					if (item1.isReadable != item2.isReadable)
					{
						resultTextBox.AppendText(string.Format("id: {0} isReadable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isReadable, item2.isReadable));
					}

					if (item1.isRotatable != item2.isRotatable)
					{
						resultTextBox.AppendText(string.Format("id: {0} isRotatable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isRotatable, item2.isRotatable));
					}

					if (item1.isStackable != item2.isStackable)
					{
						resultTextBox.AppendText(string.Format("id: {0} isStackable [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isStackable, item2.isStackable));
					}

					if (item1.isVertical != item2.isVertical)
					{
						resultTextBox.AppendText(string.Format("id: {0} isVertical [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.isVertical, item2.isVertical));
					}

					if (item1.lightColor != item2.lightColor)
					{
						resultTextBox.AppendText(string.Format("id: {0} lightColor [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.lightColor, item2.lightColor));
					}

					if (item1.lightLevel != item2.lightLevel)
					{
						resultTextBox.AppendText(string.Format("id: {0} lightLevel [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.lightLevel, item2.lightLevel));
					}

					if (item1.lookThrough != item2.lookThrough)
					{
						resultTextBox.AppendText(string.Format("id: {0} lookThrough [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.lookThrough, item2.lookThrough));
					}

					if (item1.maxReadChars != item2.maxReadChars)
					{
						resultTextBox.AppendText(string.Format("id: {0} maxReadChars [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.maxReadChars, item2.maxReadChars));
					}

					if (item1.maxReadWriteChars != item2.maxReadWriteChars)
					{
						resultTextBox.AppendText(string.Format("id: {0} maxReadWriteChars [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.maxReadWriteChars, item2.maxReadWriteChars));
					}

					if (item1.minimapColor != item2.minimapColor)
					{
						resultTextBox.AppendText(string.Format("id: {0} minimapColor [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.minimapColor, item2.minimapColor));
					}

                    if (item1.name != item2.name)
                    {
                        resultTextBox.AppendText(string.Format("id: {0} name [{1}]/[{2}]" + Environment.NewLine, item1.id, item1.name, item2.name));
                    }
				}

				if (resultTextBox.Text.Length == 0)
				{
					MessageBox.Show("No differences found!");
				}

				return true;
			}

			return false;
		}

		private void btnCompare_Click(object sender, EventArgs e)
		{
			resultTextBox.Clear();
			compareItems();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			FileDialog dialog = new OpenFileDialog();

			//Now set the file type
			dialog.Filter = "otb files (*.otb)|*.otb|All files (*.*)|*.*";

			dialog.Title = "Open otb file...";

			if (dialog.ShowDialog() == DialogResult.OK)
			{
				if (dialog.FileName.Length == 0)
				{
					return;
				}
			}

			file1Text.Text = dialog.FileName;
		}

		private void button2_Click(object sender, EventArgs e)
		{
			FileDialog dialog = new OpenFileDialog();

			//Now set the file type
			dialog.Filter = "otb files (*.otb)|*.otb|All files (*.*)|*.*";

			dialog.Title = "Open otb file...";

			if (dialog.ShowDialog() == DialogResult.OK)
			{
				if (dialog.FileName.Length == 0)
				{
					return;
				}
			}

			file2Text.Text = dialog.FileName;
		}
	}
}
