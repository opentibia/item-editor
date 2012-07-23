using System;
using System.Collections.Generic;
using otitemeditor;

namespace PluginInterface
{
	public class SpriteItems : Dictionary<UInt16, SpriteItem>
	{
		public bool signatureCalculated = false;
	}

	public class SupportedClient
	{
		public SupportedClient(
			UInt32 version,
			string description,
			UInt32 otbVersion,
			UInt32 datSignature,
			UInt32 sprSignature)
		{
			this.version = version;
			this.description = description;
			this.otbVersion = otbVersion;
			this.datSignature = datSignature;
			this.sprSignature = sprSignature;
		}

		public UInt32 version;
		public string description;
		public UInt32 otbVersion;
		public UInt32 datSignature;
		public UInt32 sprSignature;
	}

	public interface IPlugin
	{
		IPluginHost Host { get; set; }

		SpriteItems Items { get; }
		List<SupportedClient> SupportedClients { get; }
		bool LoadClient(SupportedClient client, string datFullPath, string sprFullPath);

		void Initialize();
		void Dispose();
	}

	public interface IPluginHost
	{
	}
}
