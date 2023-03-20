/*
2019-03
面向沈飞工程的板材切割可视化
2021-10
二位切割算法的结果可视化
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

		public bool status = false;//2.作为按钮启动的中介好用
		string file;

		private void button2_Click(object sender, EventArgs e)
		{
			Graphics g = panel1.CreateGraphics();
			g.Clear(panel1.BackColor);
			g.Dispose();

			OpenFileDialog dialog = new OpenFileDialog();
			dialog.Multiselect = true;

			//dialog.InitialDirectory = @"D:\CuttingTXT";

			dialog.Filter = "txt文件(*.*)|*.txt*";
			if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				file = dialog.FileName;
				// MessageBox.Show(file);
			}
		}

		private void button1_Click(object sender, EventArgs e)
		{
			panel1.Refresh();
			string strReadFilePath = @file;
			StreamReader srReadFile = new StreamReader(strReadFilePath);

			int index = 0;
			string[] res;

			while (!srReadFile.EndOfStream)
			{
				string str = srReadFile.ReadLine();
				char[] chs = { '	' }; //****数据中用的是tab还是空格得整明白
				res = str.Split(chs, StringSplitOptions.RemoveEmptyEntries);
				int XR = Convert.ToInt32(res[0]);
				int YR = Convert.ToInt32(res[1]);
				string IDR = Convert.ToString(res[2]);

				Program.Stock_List.Add(new Program.Stock());
				Program.Stock_List[index].X = XR;
				Program.Stock_List[index].Y = YR;
				Program.Stock_List[index].ID = IDR;
				index++;
			}
			srReadFile.Close();
			//Console.ReadKey();

			status = true;//2.1把按钮作为中介
			panel1.Refresh();//
		}

		private void button3_Click(object sender, EventArgs e)
		{
			Graphics g = panel1.CreateGraphics();
			g.Clear(panel1.BackColor);
			g.Dispose();
		}

		private void panel1_Paint_1(object sender, PaintEventArgs e)
		{
			if (status==true)//之前status 状态，要警惕或者利用重复。一个功能分配单独一个bool。
			{
				base.OnPaint(e);
				Graphics g;
				g = e.Graphics;//画图必备

				int Rows = Program.Stock_List.Count;//上次数ecxel的行数，这次数list的行数
				int loop_num = Rows / 4;
				for (int j = 0; j < loop_num; j++)//隔四个一循环的小技巧
				{
					int AX = (Program.Stock_List[4 * j].X); 
					int AY = panel1.Height - (Program.Stock_List[4 * j].Y);
					int BX = (Program.Stock_List[4 * j + 1].X); 
					int BY = panel1.Height - (Program.Stock_List[4 * j + 1].Y);
					int CX = (Program.Stock_List[4 * j + 2].X);
					int CY = panel1.Height - (Program.Stock_List[4 * j + 2].Y);
					int DX = (Program.Stock_List[4 * j + 3].X);
					int DY = panel1.Height - (Program.Stock_List[4 * j + 3].Y);

					Pen myPen = new Pen(Color.Blue);
					myPen.Width = 2;

					g.DrawLine(myPen, AX, AY, BX, BY);
					g.DrawLine(myPen, BX, BY, CX, CY);
					g.DrawLine(myPen, CX, CY, DX, DY);
					g.DrawLine(myPen, DX, DY, AX, AY);

					g.DrawString(Convert.ToString(Program.Stock_List[4 * j].ID), new Font("Verdana", 6), new SolidBrush(Color.Blue), new PointF(CX - 18, BY + 1));
				}

				Program.Stock_List.Clear(); // ATTENTION
			}
		}
	}
}

// 20211014
// 问题1：画图之后刷新panel
// 问题2：启发式生成txt编号
// 问题3：为什么画完的图没有消失？是字符串的问题？还是控件的问题？还是Stock_List的问题？

// 现象
// 第一次选0，显示0的图形，第二次选1，0消失显示1的图形，第三次选0，1和0的图形同时出现
// 从此只要是选0，0和1同时出现；只要是选1，0消失1出现

// 最终发现确实是List的问题，每一次画完图需要Clear();