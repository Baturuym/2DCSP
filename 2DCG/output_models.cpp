#include"2DCG.h"
using namespace std;


void OutputMasterProblem(All_Values& Values, All_Lists& Lists) {

	ofstream dataFile;
	dataFile.open("Master Problem.txt", ios::app);

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	dataFile << endl;
	dataFile << "MP-" << Values.iter << endl;

	for (int col = 0; col < all_cols_num; col++) {
		if (col < K_num) {
			dataFile << "y" << col + 1 << "\t";
		}
		if (col >= K_num) {
			dataFile << "x" << col - K_num + 1 << "\t";
		}
	}
	dataFile << endl;


	for (int col = 0; col < all_cols_num; col++) {
		dataFile << ("-----------");
	}
	dataFile << endl;

	for (int col = 0; col < all_cols_num; col++) {
		if (col < K_num) {
			dataFile << 1 << "\t";
		}
		if (col >= K_num) {
			dataFile << 0 << "\t";
		}
	}
	dataFile << endl;

	for (int col = 0; col < all_cols_num; col++) {
		dataFile << ("-----------");
	}
	dataFile << endl;

	for (int row = 0; row < all_rows_num + 1; row++) {
		for (int col = 0; col < all_cols_num; col++) {
			if (col < K_num) {
				int col_pos = col;
				if (row < J_num) {
					dataFile << int(Lists.Y_cols_list[col_pos][row]) << "\t";
				}
				if (row == J_num) {
					dataFile << ("-----------");
				}
				if (row > J_num) {
					dataFile << int(Lists.Y_cols_list[col_pos][row - 1]) << "\t";
				}
			}
			if (col >= K_num) {
				int col_pos = col - K_num;
				if (row < J_num) {
					dataFile << int(Lists.X_cols_list[col_pos][row]) << "\t";
				}
				if (row == J_num) {
					dataFile << ("-----------");
				}
				if (row > J_num) {
					dataFile << int(Lists.X_cols_list[col_pos][row - 1]) << "\t";
				}
			}
		}

		if (row < J_num) {
			dataFile << ">=" << "\t" << "0";
			dataFile << endl;
		}
		if (row == J_num) {
			dataFile << endl;
		}
		if (row > J_num) {
			int row_pos = row - J_num - 1;
			dataFile << ">=" << "\t" << int(Lists.all_item_types_list[row_pos].demand);
			dataFile << endl;
		}
	}
	dataFile.close();
}

void DisplaySubProblem(All_Values& Values, All_Lists& Lists, int Name_SP) {

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	if (Name_SP == 1) {
		printf("\n\t SP1_%d obj MAX:\n\n", Values.iter);
		for (int k = 0; k < J_num - 1; k++) {
			printf("\t %f v_%d+\n", Lists.dual_prices_list[k], k + 1);
		}
		printf("\t %f v_%d\n", Lists.dual_prices_list[J_num - 1], J_num);

		printf("\n\t SP1_%d con:\n\n\t ", Values.iter);
		for (int k = 0; k < J_num - 1; k++) {
			printf("%d v_%d + ", Lists.all_item_types_list[k].length, k + 1);
		}
		printf("%d v_%d <= %d\n", Lists.all_item_types_list[J_num - 1].length, J_num, Values.stock_length);
	}

	if (Name_SP == 2) {
		printf("\n\t SP1_%d SP2 obj MAX:\n\n", Values.iter);
		for (int k = 0; k < N_num - 1; k++) {
			printf("\t %f w_%d+\n", Lists.dual_prices_list[k], k + 1);
		}
		printf("\t %f w_%d\n\n", Lists.dual_prices_list[N_num - 1], N_num);

		printf("\n\t SP1_%d SP2 con:\n\n\t ", Values.iter);
		for (int k = 0; k < J_num - 1; k++) {
			printf("%d w_%d + ", Lists.all_item_types_list[k].width, k + 1);
		}
		printf("%d w_%d <= %d", Lists.all_item_types_list[J_num - 1].width, J_num, Values.stock_width);
		printf("\n\n");
	}
}


void OutputDualMasterProblem(All_Values& Values, All_Lists& Lists) {

	ofstream dataFile;
	dataFile.open("Dual Master Problem.txt", ios::app);

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_rows_num = K_num + P_num;
	int all_cols_num = J_num + N_num;

	dataFile << endl;
	dataFile << "MP-" << Values.iter << endl;

	for (int col = 0; col < all_cols_num; col++) {
		if (col < J_num) {
			dataFile << "v" << col+1 << "\t";
		}
		if (col >= J_num) {
			dataFile << "w"<< col-J_num+1<<"\t";
		}
	}
	dataFile << endl;

	for (int col = 0; col < all_cols_num; col++) {
		dataFile << ("-----------");
	}
	dataFile << endl;

	for (int col = 0; col < all_cols_num; col++) {
		if (col < J_num) {
			dataFile << int(Lists.all_item_types_list[col].demand)<<"\t";
		}
		if (col >= J_num) {
			dataFile << "0"<<"\t";
		}
	}
	dataFile << endl;
	
	for (int col = 0; col < all_cols_num; col++) {
		dataFile << ("-----------");
	}
	dataFile << endl;

	for (int row = 0; row < all_rows_num; row++) {
		for (int col = 0; col < all_cols_num; col++) {
			if (row < K_num) {
				dataFile << int(Lists.Y_cols_list[row][col])<<"\t";
			}
			if (row >= K_num) {
				int row_pos = row - K_num;
				dataFile << int(Lists.X_cols_list[row_pos][col]) << "\t";
			}
		}

		if (row < K_num) {
			dataFile << ("<=\t1  y") << row + 1 << endl;;
		}
		if (row >= K_num) {
			dataFile << ("<=\t0  x") <<row-K_num+1<< endl;
		}
	}
	dataFile << endl;
}

