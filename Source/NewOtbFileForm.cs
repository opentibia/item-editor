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

using PluginInterface;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace OTItemEditor
{
	public partial class NewOtbFileForm : Form
	{
		#region Private Properties

		private MainForm mainForm = null;
		private string filePath;
		private SupportedClient selectedClient;

		#endregion

		#region Public Properties

		public string FilePath
		{
			get { return filePath; }
		}

		public SupportedClient SelectedClient
		{
			get { return selectedClient; }
		}

		#endregion

		#region Constructor

		public NewOtbFileForm()
		{
			InitializeComponent();
		}

		#endregion

		#region Event Handlers

		private void NewOtbFileForm_Load(object sender, System.EventArgs e)
		{
			filePath = Path.Combine(Path.GetTempPath(), Path.GetTempFileName());

			List<SupportedClient> list = new List<SupportedClient>();

			foreach (Host.Types.Plugin plugin in Program.plugins.AvailablePlugins)
			{
				foreach (SupportedClient client in plugin.Instance.SupportedClients)
				{
					list.Add(client);
				}
			}

			if (list.Count > 0)
			{
				list = list.OrderBy(i => i.otbVersion).ToList();

				clientVersionComboBox.DataSource = list;
				clientVersionComboBox.SelectedIndex = list.Count - 1;
			}
		}

		private void ClientVersionComboBox_SelectedIndexChanged(object sender, EventArgs e)
		{
			this.createButton.Enabled = (!String.IsNullOrEmpty(filePath) && clientVersionComboBox.SelectedItem != null);
		}

		private void CreateButton_Click(object sender, EventArgs e)
		{
			if (clientVersionComboBox.SelectedItem != null)
			{
				this.selectedClient = clientVersionComboBox.SelectedItem as SupportedClient;
				this.DialogResult = DialogResult.OK;
				Close();
			}
		}

		private void CancelButton_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		#endregion

	}
}
