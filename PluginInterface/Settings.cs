using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;

namespace PluginInterface
{
	public class Settings
	{
		public string SettingFilename = "";
		private XmlDocument xmlDocument = new XmlDocument();

		public Settings()
		{
		}

		public bool Load(string filename)
		{
			string path = System.IO.Path.Combine(System.IO.Directory.GetCurrentDirectory(), "plugins");
			try
			{
				xmlDocument.Load(System.IO.Path.Combine(path, filename));
				return true;
			}
			catch
			{
				xmlDocument.LoadXml("<settings></settings>");
				return false;
			}
		}

		public List<SupportedClient> GetSupportedClientList()
		{
			List<SupportedClient> list = new List<SupportedClient>();

			XmlNodeList nodes = xmlDocument.SelectNodes("/settings/clients/client");
			if (nodes != null)
			{
				foreach (XmlNode node in nodes)
				{
					try
					{
						UInt32 version = UInt32.Parse(node.Attributes["version"].Value);
						string description = node.Attributes["description"].Value;
						UInt32 otbVersion = UInt32.Parse(node.Attributes["otbversion"].Value);
						UInt32 datSignature = (UInt32)System.Int32.Parse(node.Attributes["datsignature"].Value, System.Globalization.NumberStyles.HexNumber);
						UInt32 sprSignature = (UInt32)System.Int32.Parse(node.Attributes["sprsignature"].Value, System.Globalization.NumberStyles.HexNumber);

						SupportedClient client = new SupportedClient(version, description, otbVersion, datSignature, sprSignature);
						list.Add(client);
					}
					catch
					{
						System.Windows.Forms.MessageBox.Show("Error");
					}
				}
			}

			return list;
		}
	}
}
