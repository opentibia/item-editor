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

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Windows.Forms;

namespace OTItemEditor
{
	public partial class CompareOtbForm : Form
	{
		#region Contructor
		public CompareOtbForm()
		{
			InitializeComponent();
		}
		#endregion

		#region General Methods

		private bool CompareItems()
		{
			if (System.IO.File.Exists(file1Text.Text) && System.IO.File.Exists(file2Text.Text))
			{
				OtbList items1 = new OtbList();
				OtbList items2 = new OtbList();

				bool result;
				result = Otb.open(file1Text.Text, ref items1, false);
				if (!result)
				{
					MessageBox.Show("Could not open {0}.", file1Text.Text);
					return false;
				}

				result = Otb.open(file2Text.Text, ref items2, false);
				if (!result)
				{
					MessageBox.Show("Could not open {0}.", file2Text.Text);
					return false;
				}

				IEnumerator<OtbItem> enumerator1 = items1.GetEnumerator();
				IEnumerator<OtbItem> enumerator2 = items2.GetEnumerator();

				if (items1.Count != items2.Count)
				{
					resultTextBox.AppendText(string.Format("Item count:  [ {0} / {1} ]" + Environment.NewLine, items1.Count, items2.Count));
				}

				while (enumerator1.MoveNext())
				{
					if (!enumerator2.MoveNext())
					{
						return false;
					}

					OtbItem item1 = enumerator1.Current;
					OtbItem item2 = enumerator2.Current;

					if (item1.spriteId != item2.spriteId)
					{
						resultTextBox.AppendText(string.Format("Id: {0}  -  Sprite changed  -  [ {1} / {2} ]" + Environment.NewLine, item1.id, item1.spriteId, item2.spriteId));
						continue;
					}

					if (item1.spriteHash != null && item2.spriteHash != null && !Utils.ByteArrayCompare(item1.spriteHash, item2.spriteHash))
					{
						resultTextBox.AppendText(string.Format("Id: {0}  -  Sprite updated." + Environment.NewLine, item1.id));
					}

					foreach (PropertyInfo property in item1.GetType().GetProperties())
					{
						if (property.Name != "spriteHash" && property.Name != "spriteId")
						{
							object value1 = property.GetValue(item1, null);
							object value2 = item2.GetType().GetProperty(property.Name).GetValue(item2, null);

							if (!value1.Equals(value2))
							{
								resultTextBox.AppendText(string.Format("Id: {0}  -  {1}  -  [ {2} / {3} ]{4}", item1.id, property.Name, value1, value2, Environment.NewLine));
							}
						}
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

		#endregion

		#region Event Handlers

		private void compareButton_Click(object sender, EventArgs e)
		{
			resultTextBox.Clear();
			CompareItems();
		}

		private void browseButton1_Click(object sender, EventArgs e)
		{
			FileDialog dialog = new OpenFileDialog();

			//Now set the file type
			dialog.Filter = "OTB files (*.otb)|*.otb|All files (*.*)|*.*";
			dialog.Title = "Open OTB File";

			if (dialog.ShowDialog() != DialogResult.OK || dialog.FileName.Length == 0)
			{
				return;
			}

			file1Text.Text = dialog.FileName;
		}

		private void browseButton2_Click(object sender, EventArgs e)
		{
			FileDialog dialog = new OpenFileDialog();

			//Now set the file type
			dialog.Filter = "OTB files (*.otb)|*.otb|All files (*.*)|*.*";
			dialog.Title = "Open OTB File";

			if (dialog.ShowDialog() != DialogResult.OK || dialog.FileName.Length == 0)
			{
				return;
			}

			file2Text.Text = dialog.FileName;
		}

		private void fileText_TextChanged(object sender, EventArgs e)
		{
			compareButton.Enabled = (file1Text.TextLength > 0 && file2Text.TextLength > 0);
		}

		#endregion
	}
}
