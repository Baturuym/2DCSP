using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CuttingFig
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new Form1());
		}

		public static List<Stock> Stock_List = new List<Stock>();
		public class Stock
		{
			public string ID;
			public int A1;
			public int A2;
			public int B1;
			public int B2;
			public int C1;
			public int C2;
			public int D1;
			public int D2;
			public int X;
			public int Y;
		}
	}
}
