// Yuming Zhao: https://github.com/Baturuym
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

void SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	v.clear();//删除原内容
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

void ReadData(All_Values& Values, All_Lists& Lists) // 启发式读取数据
{
	ostringstream s_in, s_out;//IO文件
	string in_str, out_str;//IO文件名
	ofstream f_out;//输出文件
	string line;//读数据的一行
	vector<string> data_inline, data_inline1, data_inline2;//读取本地行中的数据

	s_in.str("");
	s_in << "C:/Users/YMZhao/Desktop/2DBP/2DBP/cutdata1207.txt";
	//s_in << "D:/CuttingTXT/cutdata11251.txt";
	in_str = s_in.str();

	ifstream fin(in_str.c_str());

	if (fin)
	{
		getline(fin, line);
		SplitString(line, data_inline, "\t"); // 第1行
		Values.stocks_num = atoi(data_inline[0].c_str()); // 第1行第1位：母板数量

		getline(fin, line);
		SplitString(line, data_inline, "\t"); // 第2行
		Values.item_types_num = atoi(data_inline[0].c_str()); // 第2行第1位：子板种类数量
		Values.strip_types_num = Values.item_types_num;

		getline(fin, line);
		SplitString(line, data_inline, "\t"); // 第3+行 母板行

		Values.stock_length = atoi(data_inline[0].c_str()); // 母板长度
		Values.stock_width = atoi(data_inline[1].c_str()); // 母板宽度

		for (int i = 0; i < Values.stocks_num; i++)
		{
			StockProperties this_stock;
			this_stock.length = Values.stock_length;
			this_stock.width = Values.stock_width;
			this_stock.area = Values.stock_length * Values.stock_width;
			this_stock.x_value = 0;
			this_stock.y_value = 0;
			Lists.stock_pool_list.insert(Lists.stock_pool_list.begin(), this_stock);
		}

		int item_index = 1;
		int item_types_num = Values.item_types_num;

		for (int i = 0; i < item_types_num; i++) // 所有子板行
		{
			getline(fin, line);
			SplitString(line, data_inline, "\t");

			int item_demand = atoi(data_inline[2].c_str());
			for (int k = 0; k < item_demand; k++) // 子板需求量
			{
				ItemProperties this_item;
				this_item.type = atoi(data_inline[3].c_str()); // 子板行第4位：子板种类
				this_item.index = item_index; // 子板序号，从1开始
				this_item.demand = atoi(data_inline[2].c_str()); // 子板行第3位：子板需求		
				this_item.length = atoi(data_inline[0].c_str()); // 子板行第1位：子板长度
				this_item.width = atoi(data_inline[1].c_str()); // 子板行第2位：子板宽度
				this_item.area = this_item.length * this_item.width; // 子板面积
				this_item.x_value = -1; // 子板左上角x_value坐标
				this_item.y_value = -1; // 子板左上角y_value坐标
				this_item.stock_index = -1; // 子板所属母板编号
				this_item.occupied = 0;
				Lists.all_items_list.push_back(this_item);

				item_index++;
				Values.items_num++;
			}

			ItemTypeProperties this_item_type;
			this_item_type.type = atoi(data_inline[3].c_str());
			this_item_type.demand = atoi(data_inline[2].c_str());
			this_item_type.width = atoi(data_inline[1].c_str());
			this_item_type.length = atoi(data_inline[0].c_str());
			Lists.item_types_list.push_back(this_item_type);
		}
	}

	// 所有子板按照宽度从宽到窄排序
	ItemProperties  VP;
	int all_items_list_size = Lists.all_items_list.size();
	for (int i = 0; i < all_items_list_size - 1; i++)
	{
		for (int j = i + 1; j < all_items_list_size; j++)
		{
			if (Lists.all_items_list[i].width < Lists.all_items_list[j].width)
			{
				VP = Lists.all_items_list[i];
				Lists.all_items_list[i] = Lists.all_items_list[j];
				Lists.all_items_list[j] = VP;
			}
		}
	}
}