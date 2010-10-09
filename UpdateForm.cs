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
	public partial class UpdateForm : Form
	{
		public otitemeditor mainForm = null;
		public Host.Types.Plugin selectedPlugin = null;
		public SupportedClient updateClient = null;

		private UInt32 currentClientIndex = 0;
		private List<SupportedClient> clientList = new List<SupportedClient>();

		public UpdateForm()
		{
			InitializeComponent();
		}

		private void PluginForm_Load(object sender, EventArgs e)
		{
			foreach (Host.Types.Plugin plugin in Program.plugins.AvailablePlugins)
			{
				foreach (SupportedClient client in plugin.Instance.SupportedClients)
				{
					UInt32 index = (UInt32)pluginsListBox.Items.Add(client.description);
					clientList.Add(client);

					if (client.otbVersion == mainForm.currentOtbVersion)
					{
						currentClientIndex = index;
					}
				}
			}

			if (pluginsListBox.Items.Count > 0)
			{
				pluginsListBox.SelectedIndex = (int)currentClientIndex;
			}
		}

		private void selectBtn_Click(object sender, EventArgs e)
		{
			if(pluginsListBox.SelectedItem != null){
				selectedPlugin = Program.plugins.AvailablePlugins.Find(pluginsListBox.SelectedItem.ToString());
				updateClient = clientList[(Int32)pluginsListBox.SelectedIndex];

				this.DialogResult = DialogResult.OK;
				Close();
			}
		}

		private void pluginsListBox_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (pluginsListBox.SelectedItem != null && mainForm.currentPlugin != null)
			{
				selectBtn.Enabled = (pluginsListBox.SelectedIndex != currentClientIndex);
			}
		}

		private void btnChangeFolder_Click(object sender, EventArgs e)
		{

		}
	}
}
