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
using System;
using System.Collections.Generic;

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

		public override string ToString()
		{
			return this.description;
		}
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
