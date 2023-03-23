
#include "2DBP.h"
using namespace std;

void OutPutResults(All_Values& Values, All_Lists& Lists) {
	int stocks_num = Lists.occupied_stocks_list.size();
	int items_num = Lists.occupied_items_list.size();
	int strips_num = Lists.all_strips_list.size();

	for (int pos = 0; pos < stocks_num; pos++) {
		int LL = Lists.occupied_stocks_list[0].length;
		int WW = Lists.occupied_stocks_list[0].width;

		printf("\n\tSTOCK_%d ====================\n\n", Lists.occupied_stocks_list[pos].stock_idx);
		printf("\t0\t0\n");
		printf("\t0\t%d\n", WW);
		printf("\t%d\t%d\n", LL, WW);
		printf("\t%d\t0\n", LL);

		printf("\n\tSTOCK_%d, Stripes:\n", Lists.occupied_stocks_list[pos].stock_idx);
		for (size_t i = 0; i < strips_num; i++) {
			if (Lists.all_strips_list[i].stock_idx == pos) {
				int X = Lists.all_strips_list[i].pos_x;
				int Y = Lists.all_strips_list[i].pos_y;
				int L = Lists.all_strips_list[i].length;
				int W = Lists.all_strips_list[i].width;
				int strip_type_idx = Lists.all_strips_list[i].strip_type_idx;

				printf("\n\tStrip_type_%d\n", strip_type_idx);
				printf("\t%d\t%d\n", X, Y);
				printf("\t%d\t%d\n", X, Y + W);
				printf("\t%d\t%d\n", X + L, Y + W);
				printf("\t%d\t%d\n", X + L, Y);
			}
		}

		printf("\n\tSTOCK_%d, Item:\n", Lists.occupied_stocks_list[pos].stock_idx);
		for (size_t i = 0; i < items_num; i++) {
			if (Lists.occupied_items_list[i].stock_idx == pos) {
				int X = Lists.occupied_items_list[i].pos_x;
				int Y = Lists.occupied_items_list[i].pos_y;
				int L = Lists.occupied_items_list[i].length;
				int W = Lists.occupied_items_list[i].width;
				int item_type_index = Lists.occupied_items_list[i].item_type_idx;

				printf("\n\tItem_type_%d\n", item_type_index);
				printf("\t%d\t%d\n", X, Y);
				printf("\t%d\t%d\n", X, Y + W);
				printf("\t%d\t%d\n", X + L, Y + W);
				printf("\t%d\t%d\n", X + L, Y);
			}
		}
	}

	ostringstream s_in, s_out;//IO文件
	string in_str, out_str;//IO文件名
	ofstream f_out;//输出文件

	// 输出结果信息到txt文件，用于画图
	for (int pos = 0; pos < stocks_num; pos++) {
		s_out.str("");
		s_out << "D:/CuttingTXT/Stock_" << pos << ".txt";
		out_str = s_out.str();
		f_out.open(out_str, ios::out);

		int LL = Lists.occupied_stocks_list[0].length;
		int WW = Lists.occupied_stocks_list[0].width;

		f_out << 0 << "\t" << 0 << "\t" << "x" << endl;
		f_out << 0 << "\t" << WW << "\t" << "x" << endl;
		f_out << LL << "\t" << WW << "\t" << "x" << endl;
		f_out << LL << "\t" << 0 << "\t" << "x" << endl;

		// 输出母板s中子板信息
		for (size_t i = 0; i < items_num; i++) {
			if (Lists.occupied_items_list[i].stock_idx == pos) {
				int X = Lists.occupied_items_list[i].pos_x;
				int Y = Lists.occupied_items_list[i].pos_y;
				int L = Lists.occupied_items_list[i].length;
				int W = Lists.occupied_items_list[i].width;
				int item_type_idx = Lists.occupied_items_list[i].item_type_idx;

				f_out << X << "\t" << Y << "\t" << "I" << item_type_idx << endl;
				f_out << X << "\t" << Y + W << "\t" << "I" << item_type_idx << endl;
				f_out << X + L << "\t" << Y + W << "\t" << "I" << item_type_idx << endl;
				f_out << X + L << "\t" << Y << "\t" << "I" << item_type_idx << endl;
			}
		}

		// 输出母板s中板条信息
		for (size_t i = 0; i < strips_num; i++) {
			if (Lists.all_strips_list[i].stock_idx == pos) {
				int X = Lists.all_strips_list[i].pos_x;
				int Y = Lists.all_strips_list[i].pos_y;
				int L = Lists.all_strips_list[i].length;
				int W = Lists.all_strips_list[i].width;
				int strip_type_idx = Lists.all_strips_list[i].strip_type_idx;

				f_out << X << "\t" << Y << "\t" << "S" << strip_type_idx << endl;
				f_out << X << "\t" << Y + W << "\t" << "S" << strip_type_idx << endl;
				f_out << X + L << "\t" << Y + W << "\t" << "S" << strip_type_idx << endl;
				f_out << X + L << "\t" << Y << "\t" << "S" << strip_type_idx << endl;
			}
		}

		f_out.close();
	}
}
