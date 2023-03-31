// 2022-11-17

#include "2DBP.h"
using namespace std;

void SplitString(const string& line_string, vector<string>& string_list, const string& data_string) {
	string::size_type pos1, pos2;
	pos2 = line_string.find(data_string);
	pos1 = 0;
	string_list.clear();
	while (string::npos != pos2) {
		string_list.push_back(line_string.substr(pos1, pos2 - pos1));
		pos1 = pos2 + data_string.size();
		pos2 = line_string.find(data_string, pos1);
	}
	if (pos1 != line_string.length()) {
		string_list.push_back(line_string.substr(pos1));
	}
}

void ReadData(All_Values& Values, All_Lists& Lists) // 启发式读取数据
{
	ostringstream s_in, s_out;//IO文件
	string in_str, out_str;//IO文件名
	ofstream f_out;//输出文件
	string line;//读数据的一行
	vector<string> data_inline, data_inline1, data_inline2;//读取本地行中的数据

	s_in.str("");
	s_in << "C:/Users/YMZhao/Desktop/2DCG/2DBP/cutdata1207.txt";
	//s_in << "D:/CuttingTXT/cutdata11251.txt";
	in_str = s_in.str();

	ifstream fin(in_str.c_str());

	if (fin) {
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

		for (int i = 0; i < Values.stocks_num; i++) {
			Stock_Stc this_stock;
			this_stock.length = Values.stock_length;
			this_stock.width = Values.stock_width;
			this_stock.area = Values.stock_length * Values.stock_width;
			this_stock.pos_x = 0;
			this_stock.pos_y = 0;
			Lists.all_stocks_list.insert(Lists.all_stocks_list.begin(), this_stock);
		}

		int item_index = 1;
		int item_types_num = Values.item_types_num;

		for (int i = 0; i < item_types_num; i++) {  // 所有子板行
			getline(fin, line);
			SplitString(line, data_inline, "\t");

			int item_type_demand_num = atoi(data_inline[2].c_str());
			for (int k = 0; k < item_type_demand_num; k++) {   // 子板需求量
				Item_Stc this_item;
				this_item.item_type_idx = atoi(data_inline[3].c_str()); // 子板行第4位：子板种类
				this_item.item_idx = item_index; // 子板序号，从1开始
				this_item.demand = atoi(data_inline[2].c_str()); // 子板行第3位：子板需求		
				this_item.length = atoi(data_inline[0].c_str()); // 子板行第1位：子板长度
				this_item.width = atoi(data_inline[1].c_str()); // 子板行第2位：子板宽度
				this_item.area = this_item.length * this_item.width; // 子板面积
				this_item.pos_x = -1; // 子板左上角pos_x坐标
				this_item.pos_y = -1; // 子板左上角pos_y坐标
				this_item.stock_idx = -1; // 子板所属母板编号
				this_item.occupied = 0;
				Lists.all_items_list.push_back(this_item);

				item_index++;
				Values.items_num++;
			}

			Item_Type_Stc this_item_type;
			this_item_type.item_type_idx = atoi(data_inline[3].c_str());
			this_item_type.demand = atoi(data_inline[2].c_str());
			this_item_type.width = atoi(data_inline[1].c_str());
			this_item_type.length = atoi(data_inline[0].c_str());
			Lists.all_item_types_list.push_back(this_item_type);
		}
	}

	// 所有子板按照宽度从宽到窄排序
	Item_Stc  VP;
	int all_items_num = Lists.all_items_list.size();
	for (int i = 0; i < all_items_num - 1; i++) {
		for (int j = i + 1; j < all_items_num; j++) {
			if (Lists.all_items_list[i].width < Lists.all_items_list[j].width) {
				VP = Lists.all_items_list[i];
				Lists.all_items_list[i] = Lists.all_items_list[j];
				Lists.all_items_list[j] = VP;
			}
		}
	}
}
