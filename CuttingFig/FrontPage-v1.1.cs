/*
2019-03: SAC
2021-10: 2D-CSP
 */

using System;
using System.Windows.Forms;
using System.IO;
using System.Drawing;

namespace CuttingFig
{
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
		}

		public bool status = false; //ATTENTION: button-click as a flag
		string file;

		private void button2_Click(object sender, EventArgs e)
		{
			Graphics g = panel1.CreateGraphics();
			g.Clear(panel1.BackColor);
			g.DLSPose();

			OpenFileDialog dialog = new OpenFileDialog();
			dialog.Multiselect = true;

			dialog.InitialDirectory = @"D:\CuttingTXT";

			dialog.Filter = "txt文件(*.*)|*.txt*";
			if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				file = dialog.FileName;
				//MessageBox.Show(file);
			}
		}

		private void button1_Click(object sender, EventArgs e)
		{
			panel1.Refresh();
			string strReadFilePath = @file;
			StreamReader srReadFile = new StreamReader(strReadFilePath);

			int pos = 0;
			string[] res;

			while (!srReadFile.EndOfStream)
			{
				string str = srReadFile.ReadLine();
				char[] chs = { '	' }; // ATTENTION:Tab or Space?
				res = str.Split(chs, StringSplitOptions.RemoveEmptyEntries);

				int X = Convert.ToInt32(res[0]); // X
				int Y = Convert.ToInt32(res[1]); // Y
				string ID = Convert.ToString(res[2]); // name

				Program.results_list.Add(new Program.Stock());
				Program.results_list[pos].X = X;
				Program.results_list[pos].Y = Y;
				Program.results_list[pos].ID = ID;
				pos++;
			}
			srReadFile.Close();
			//Console.ReadKey();

			status = true;//ATTENTION: button-click as a flag
			panel1.Refresh();//
		}

		private void button3_Click(object sender, EventArgs e)
		{
			Graphics g = panel1.CreateGraphics();
			g.Clear(panel1.BackColor);
			g.DLSPose();
		}

		private void panel1_Paint_1(object sender, PaintEventArgs e)
		{
			if (status == true) // ATTENTION: 之前status 状态，要警惕或者利用重复。一个功能分配单独一个bool。
			{
				base.OnPaint(e);
				Graphics draw;
				draw = e.Graphics;

				int Rows = Program.results_list.Count;//上次数ecxel的行数，这次数list的行数
				int loop_num = Rows / 4;
				for (int j = 0; j < loop_num; j++)
				{
					// ATTENTION: as for C#, upper left point is the (0,0) point

					// A: lower left point
					// B: upper left point
					// C: upper right point
					// D: lower right point

					int para = 2;
					int AX = para*(Program.results_list[4 * j].X);
					int AY = panel1.Height - para*(Program.results_list[4 * j].Y);
					int BX =para* (Program.results_list[4 * j + 1].X);
					int BY = panel1.Height - para*(Program.results_list[4 * j + 1].Y);
					int CX = para*(Program.results_list[4 * j + 2].X);
					int CY = panel1.Height - para*(Program.results_list[4 * j + 2].Y);
					int DX = para*(Program.results_list[4 * j + 3].X);
					int DY = panel1.Height - para*(Program.results_list[4 * j + 3].Y);

					Pen myPen = new Pen(Color.Blue);
					myPen.Width = 2;

					draw.DrawLine(myPen, AX, AY, BX, BY); // A->B
					draw.DrawLine(myPen, BX, BY, CX, CY); // B->C
					draw.DrawLine(myPen, CX, CY, DX, DY); // C->D
					draw.DrawLine(myPen, DX, DY, AX, AY); // D->A

					draw.DrawString(
						Convert.ToString(Program.results_list[4 * j].ID), 
						new Font("Courier", 10), 
						new SolidBrush(Color.Blue), 
						new PointF(CX - 22, BY + 2));
				}

				Program.results_list.Clear(); // ATTENTION: must be cleared
			}
		}
	}
}

// 20211014
// 问题1：画图之后刷新panel
// 问题2：启发式生成txt编号
// 问题3：为什么画完的图没有消失？是字符串的问题？还是控件的问题？还是results_list的问题？

// 现象
// 第一次选0，显示0的图形，第二次选1，0消失显示1的图形，第三次选0，1和0的图形同时出现
// 从此只要是选0，0和1同时出现；只要是选1，0消失1出现

// 最终发现确实是List的问题，每一次画完图需要Clear();