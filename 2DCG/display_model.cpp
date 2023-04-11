#include"2DCG.h"
using namespace std;

void DisplayMasterProblem(All_Values& Values, All_Lists& Lists) {

	int K_num = Lists.cutting_stock_cols.size();
	int P_num = Lists.cutting_strip_cols.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	for (int col = 0; col < all_cols_num + 1; col++) {
		if (col == 0) {
			printf("Min:\t");
		}
		if (col > 0) {
			if (col < K_num + 1) {
				printf("y%d\t", col);
			}
			if (col >= K_num + 1) {
				printf("x%d\t", col - K_num);
			}
		}
	}
	printf("\n");


	for (int col = 0; col < all_cols_num + 1; col++) {
		if (col == 0) {
			printf("  \t");
		}
		if (col > 0) {
			if (col == K_num + 1) {
				printf("--------");
			}
			if (col != K_num + 1) {
				printf("--------");
			}
		}
	}
	printf("\n");

	/*
	for (int col = 0; col < all_cols_num + 1; col++) {
		if (col == 0) {
			printf("  \t");
		}
		if (col > 0) {
			if (col < K_num + 1) {
				printf("1\t");
			}
			if (col == K_num + 1) {
				printf("0\t");
			}
			if (col > K_num + 1) {
				printf("0\t");
			}
		}
	}
	printf("\n");

	for (int col = 0; col < all_cols_num + 1; col++) {
		if (col == 0) {
			printf("  \t");
		}
		if (col > 0) {
			if (col == K_num + 1) {
				printf("--------");
			}
			if (col != K_num + 1) {
				printf("--------");
			}
		}
	}
	printf("\n");
	*/

	// 输出矩阵
	for (int row = 0; row < all_rows_num+1; row++) {
		for (int col = 0; col < all_cols_num+1; col++) {

			if (col == 0) {
				if (row < J_num) {
					printf("v%d:\t", row + 1);
				}
				if (row == J_num) {
					printf("  \t");
				}
				if (row > J_num) {
					printf("w%d:\t", row - J_num);
				}
			}

			// 输出矩阵元素和分割线
			if (col > 0) {
				if (col < K_num+1) {
					int col_pos = col - 1;
					if (row < J_num) {
						printf("%d\t", int(Lists.cutting_stock_cols[col_pos][row]));
					}
					if (row == J_num) {
						printf("--------");
					}
					if (row > J_num) {
						printf("%d\t", int(Lists.cutting_stock_cols[col_pos][row - 1]));
					}
				}
				if (col >= K_num+1) {
					int col_pos = col-K_num - 1;
					if (row < J_num) {
						printf("%d\t", int(Lists.cutting_strip_cols[col_pos][row]));
					}
					if (row == J_num) {
						printf("--------");
					}
					if (row > J_num) {
						printf("%d\t", int(Lists.cutting_strip_cols[col_pos][row - 1]));
					}
				}
			}
		}

		// 输出右端项
		if (row < J_num) {
			printf(">=\t0");
			printf("\n");
		}
		if (row == J_num) {
			printf("\n");
		}
		if (row > J_num) {
			int row_pos = row - J_num;
			printf(">=\t%d", int(Lists.all_item_types_list[row_pos].demand));
			printf("\n");
		}
	}
}

void DisplaySubProblem(All_Values& Values, All_Lists& Lists, int Name_SP) {

	int K_num = Lists.cutting_stock_cols.size();
	int P_num = Lists.cutting_strip_cols.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	if (Name_SP == 1) {
		printf("\t ");
		for (int k = 0; k < J_num; k++) {
			printf("v%d\t", k);
		}
		printf("\n");
		printf("\t ");
		for (int k = 0; k < J_num; k++) {
			printf("%d\t", Lists.all_item_types_list[k].length);
		}
		printf("<=\t%d", Values.stock_length);
	}
	if (Name_SP == 2) {
		printf("\t ");
		for (int k = 0; k < J_num; k++) {
			printf("w%d\t", k);
		}
		printf("\n");
		printf("\t ");
		for (int k = 0; k < J_num; k++) {
			printf("%d\t", Lists.all_item_types_list[k].width);
		}
		printf("<=\t%d", Values.stock_width);
		printf("\n\n");
	}
}

/*
void DisplayDualMasterProblem(All_Values& Values, All_Lists& Lists) {

	int K_num = Lists.cutting_stock_cols.size();
	int P_num = Lists.cutting_strip_cols.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	printf("\n\nThe dual model of current MP\n\n");
	for (int col = -1; col < item_types_num + strip_types_num; col++) {
		if (col == -1) {
			printf("Max:\t");
		}
		if (col != -1) {
			if (col < strip_types_num) {
				printf("v%d\t", col + 1);
			}
			if (col == strip_types_num) {
				printf("w%d\t", col + 1 - strip_types_num);
			}
			if (col > strip_types_num) {
				printf("w%d\t", col + 1 - strip_types_num);
			}
		}
	}
	printf("\n");
	for (int col = -1; col < item_types_num + strip_types_num; col++) {
		if (col == -1) {
			printf("  \t");
		}
		if (col > -1) {
			if (col < strip_types_num) {
				printf("--------");
			}
			if (col == strip_types_num) {
				printf("--------");
			}
			if (col > strip_types_num) {
				printf("--------");
			}
		}
	}
	printf("\n");
	for (int col = -1; col < item_types_num + strip_types_num; col++) {
		if (col == -1) {
			printf("  \t");
		}
		if (col != -1) {
			if (col < strip_types_num) {
				printf("%d\t", Lists.item_types_list[col].demand);
			}
			if (col == strip_types_num) {
				printf("0\t");
			}
			if (col > strip_types_num) {
				printf("0\t");
			}
		}
	}
	printf("\n");
	for (int col = -1; col < item_types_num + strip_types_num; col++) {
		if (col == -1) {
			printf("  \t");
		}
		if (col > -1) {
			if (col < strip_types_num) {
				printf("--------");
			}
			if (col == strip_types_num) {
				printf("--------");
			}
			if (col > strip_types_num) {
				printf("--------");
			}
		}
	}
	printf("\n");
	for (int row = 0; row < K_num + stg2_cols_num + 1; row++) {
		for (int col = -1; col < strip_types_num + item_types_num; col++) {
			int first_stage_patterns_list_size = Lists.stg1_patterns_list.size();
			if (row < first_stage_patterns_list_size) {
				if (col == -1) {
					printf("y%d:\t", row + 1);
				}
				if (col != -1) {
					if (col == strip_types_num) {
						printf("%d\t", Lists.stg0_cols_list[row][col]);
					}
					if (col != strip_types_num) {
						printf("%d\t", Lists.stg0_cols_list[row][col]);
					}
				}
			}

			if (row >= first_stage_patterns_list_size) {
				if (col == -1) {
					printf("x%d:\t", row - stg2_cols_num + 3);
				}
				if (col != -1) {
					if (col == strip_types_num) {
						printf("%d\t", Lists.stg0_cols_list[row - 1][col]);
					}
					if (col != strip_types_num) {
						printf("%d\t", Lists.stg0_cols_list[row - 1][col]);
					}
				}
			}
		}
		if (row < K_num) {
			printf("<=\t1\n");
		}
		if (row >= K_num) {
			printf("<=\t0\n");
		}
	}
	printf("\n\n");
}
*/
