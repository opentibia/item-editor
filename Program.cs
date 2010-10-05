using System;
using System.Collections.Generic;
using System.Windows.Forms;
using PluginInterface;

namespace otitemeditor
{
    static class Program
    {
        public static Host.PluginServices plugins = new Host.PluginServices();

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new otitemeditor());
        }
    }
}
