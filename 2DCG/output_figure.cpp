
// 2023-03-20: Column Generation for 2D-CSP

#include "2DCG.h"
using namespace std;

void OutputHeuristicResults(All_Values& Values, All_Lists& Lists) {

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	int stocks_num = Lists.all_stocks_list.size();
	int strips_num = Lists.all_strips_list.size();

	for (int k = 0; k < stocks_num; k++) {		
		for (int s = 0; s < strips_num; s++) {
			if (Lists.all_strips_list[s].occupied_flag != 1) {

				if (s == 0) {
					Lists.all_strips_list[s].pos_x = 0;
					Lists.all_strips_list[s].pos_y = 0;
					Lists.all_strips_list[s].stock_idx = Lists.all_stocks_list[k].stock_idx;
					Lists.all_strips_list[s].occupied_flag = 1;		

					int items_num = Lists.all_strips_list[s].items_list.size();
					for (int i = 0; i < items_num; i++) {
						if (Lists.all_strips_list[s].items_list[i].occupied_flag != 1) {

							if (i == 0) {
								Lists.all_strips_list[s].items_list[i].pos_x = 0;
								Lists.all_strips_list[s].items_list[i].pos_y = 0;
								Lists.all_strips_list[s].items_list[i].occupied_flag = 1;
								Lists.occupied_items_list.push_back(Lists.all_strips_list[s].items_list[i]);
							}
							else {
								Lists.all_strips_list[s].items_list[i].pos_x
									= Lists.all_strips_list[s].items_list[i - 1].pos_x
									+ Lists.all_strips_list[s].items_list[i - 1].length;
								Lists.all_strips_list[s].items_list[i].pos_y = 0;
								Lists.all_strips_list[s].items_list[i].occupied_flag = 1;
								Lists.occupied_items_list.push_back(Lists.all_strips_list[s].items_list[i]);
							}
						}
					}
				}

				else {
					Lists.all_strips_list[s].pos_x = 0;
					Lists.all_strips_list[s].pos_y
						= Lists.all_strips_list[s - 1].pos_y
						+ Lists.all_strips_list[s - 1].width;
					if (Lists.all_strips_list[s].pos_y >= Values.stock_width)
						break;
					else {
						Lists.all_strips_list[s].stock_idx = Lists.all_stocks_list[k].stock_idx;
						Lists.all_strips_list[s].occupied_flag = 1;
					}
				}
			}
		}
	}

	int items_num = Lists.occupied_items_list.size();

	for (int pos = 0; pos < stocks_num; pos++) {

		int LL = Lists.all_stocks_list[0].length;
		int WW = Lists.all_stocks_list[0].width;

		printf("\n\tSTOCK_%d ====================\n\n", Lists.all_stocks_list[pos].stock_idx+1);
		printf("\t0\t0\n");
		printf("\t0\t%d\n", WW);
		printf("\t%d\t%d\n", LL, WW);
		printf("\t%d\t0\n", LL);

		printf("\n\tSTOCK_%d, Stripes:\n", Lists.all_stocks_list[pos].stock_idx+1);
		for (size_t i = 0; i < strips_num; i++) {
			if (Lists.all_strips_list[i].stock_idx == Lists.all_stocks_list[pos].stock_idx) {

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

		printf("\n\tSTOCK_%d, Item:\n", Lists.all_stocks_list[pos].stock_idx+1);
		for (size_t i = 0; i < items_num; i++) {
			if (Lists.occupied_items_list[i].stock_idx == Lists.all_stocks_list[pos].stock_idx) {

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
		s_out << "C:/Users/YMZhao/Desktop/2DCSP/2DCG/Stock_" << pos << ".txt";
		out_str = s_out.str();
		f_out.open(out_str, ios::out);

		int LL = Lists.all_stocks_list[0].length;
		int WW = Lists.all_stocks_list[0].width;

		f_out << 0 << "\t" << 0 << "\t" << "x" << endl;
		f_out << 0 << "\t" << WW << "\t" << "x" << endl;
		f_out << LL << "\t" << WW << "\t" << "x" << endl;
		f_out << LL << "\t" << 0 << "\t" << "x" << endl;

		// 输出母板s中子板信息
		for (size_t i = 0; i < items_num; i++) {
			if (Lists.occupied_items_list[i].stock_idx == Lists.all_stocks_list[pos].stock_idx) {

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
			if (Lists.all_strips_list[i].stock_idx == Lists.all_stocks_list[pos].stock_idx) {
				int X = Lists.all_strips_list[i].pos_x;
				int Y = Lists.all_strips_list[i].pos_y;
				int L = Lists.all_strips_list[i].length;
				int W = Lists.all_strips_list[i].width;
				int strip_type_idx = Lists.all_strips_list[i].strip_type_idx+1;

				f_out << X << "\t" << Y << "\t" << "S" << strip_type_idx << endl;
				f_out << X << "\t" << Y + W << "\t" << "S" << strip_type_idx << endl;
				f_out << X + L << "\t" << Y + W << "\t" << "S" << strip_type_idx << endl;
				f_out << X + L << "\t" << Y << "\t" << "S" << strip_type_idx << endl;
			}
		}

		f_out.close();
	}
}
