
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

int SolveStageOneSubProblem(All_Values& Values, All_Lists& Lists) {

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	int loop_continue_flag = -1;

	IloEnv Env_SP1; // SP1环境
	IloModel Model_SP1(Env_SP1); // SP1模型
	IloNumVarArray Ga_Vars(Env_SP1); // SP1决策变量

	for (int j = 0; j < J_num; j++) {
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity; // var UB, var >= 0
		string var_name = "G_" + to_string(j + 1);
		IloNumVar Var_Ga(Env_SP1, var_min, var_max, ILOINT, var_name.c_str()); //
		Ga_Vars.add(Var_Ga); // 
	}

	// SP1 obj
	IloExpr obj_sum(Env_SP1);
	for (int j = 0; j < J_num; j++) { // vars num = strip_types num
		double val = Lists.dual_prices_list[j];
		obj_sum += val * Ga_Vars[j]; //  obj: sum (a_j * G_j)
	}
	IloObjective Obj_SP1 = IloMaximize(Env_SP1, obj_sum);
	Model_SP1.add(Obj_SP1); // 
	obj_sum.end();

	// SP1 only one con
	IloExpr con_sum(Env_SP1);
	for (int j = 0; j < J_num; j++) {
		double val = Lists.all_strip_types_list[j].width;
		con_sum += val * Ga_Vars[j]; // con: sum (ws_j * G_j) <= W
	}
	Model_SP1.add(con_sum <= Values.stock_width);
	con_sum.end();

	printf("\n///////////////// SP_%d CPLEX solving START /////////////////\n\n", Values.iter);
	IloCplex Cplex_SP1(Env_SP1);
	Cplex_SP1.extract(Model_SP1);
	//Cplex_SP1.exportModel("Width Sub Problem.lp"); // 
	bool SP1_flag = Cplex_SP1.solve(); // 
	printf("\n///////////////// SP_%d CPLEX solving OVER /////////////////\n\n", Values.iter);

	if (SP1_flag == 0) {
		printf("\n\t SP_%d is NOT FEASIBLE\n", Values.iter);
	}
	else {
		printf("\n\n\t SP_%d is FEASIBLE\n", Values.iter);

		printf("\n\n\t Obj = %f\n", Cplex_SP1.getValue(Obj_SP1));
		double SP1_obj_val = Cplex_SP1.getValue(Obj_SP1);

		Lists.new_Y_col.clear();
		printf("\n\t SP_%d VARS:\n\n", Values.iter);
		for (int j = 0; j < J_num; j++) { // this_strip rows
			double soln_val = Cplex_SP1.getValue(Ga_Vars[j]);
			if (soln_val == -0) {
				soln_val = 0;
			}
			printf("\t var_G_%d = %f\n", j + 1, soln_val);
			Lists.new_Y_col.push_back(soln_val);
		}

		printf("\n\t SP_%d new col:\n\n", Values.iter);
		for (int j = 0; j < J_num + N_num; j++) {
			if (j < J_num) {
				double soln_val = Cplex_SP1.getValue(Ga_Vars[j]);
				if (soln_val == -0) {
					soln_val = 0;
				}
				printf("\t row_%d = %f\n", j + 1, soln_val);
			}
			else {
				printf("\t row_%d = 0\n", j + 1);
				Lists.new_Y_col.push_back(0);
			}
		}
		if (SP1_obj_val > 1 + RC_EPS) { // 则求解SP1获得的新列加入当前MP，不用求解SP2
			printf("\n\n\t SP reduced cost = %f > 1,  \n", SP1_obj_val);
			printf("\n\t No need to solve SP2\n");

			Values.Y_col_flag = 1;
			loop_continue_flag = 1;
		}
		else { // 则继续求解这张中间板对应的SP2，看能否求出新列
			printf("\n\t SP reduced cost = %f <=1 \n", SP1_obj_val);
			printf("\n\t Continue to solve SP2\n");
			
			Lists.new_Y_col.clear();
			Lists.new_X_cols_list.clear();
			Values.Y_col_flag = 0;
			Values.SP2_obj_val = -1;

			int feasible_flag = 0;
			int SP2_flag = -1;
			int K_num = Lists.Y_cols_list.size();
			int P_num = Lists.X_cols_list.size();

			for (int k = 0; k < J_num; k++) { // all current stk-cut-patterns

				SP2_flag = SolveStageTwoSubProblem(Values, Lists, k + 1);
				if (SP2_flag == 1) {
					double a_val = Lists.dual_prices_list[k];

					if (Values.SP2_obj_val > a_val + RC_EPS) {
						feasible_flag = 1;
						printf("\n\t SP_%d_%d obj = %f > strip con_%d dual = %f:\n",
							Values.iter, k + 1, Values.SP2_obj_val, k + 1, a_val);

						vector<double> temp_col;
						for (int j = 0; j < J_num; j++) { //  all current stp-cut-patterns 
							if (k == j) {  // this stp-cut-pattern p is USED in stk-cut-pattern k
								temp_col.push_back(-1); // used
							}
							else {
								temp_col.push_back(0); // not used
							}
						}

						for (int i = 0; i < N_num; i++) {
							double soln_val = Lists.SP2_solns_list[i];
							if (soln_val == - 0) {
								soln_val = 0;
							}
							temp_col.push_back(soln_val); // 
						}

						printf("\n\t SP_%d_%d new col:\n\n", Values.iter, k + 1);
						for (int row = 0; row < J_num + N_num; row++) {
							printf("\t row_%d = %f\n", row + 1, temp_col[row]); // 输出SP2的新列
						}
						printf("\n\t Add SP_%d_%d new col to MP\n\n", Values.iter, k + 1);

						Lists.new_X_cols_list.push_back(temp_col);
						cout << endl;
					}
					else {
						printf("\n\t SP_%d_%d Obj = %f < strip con_%d dual = %f:\n",
							Values.iter, k + 1, Values.SP2_obj_val, k + 1, a_val);
						cout << endl;
					}
				}
			}

			if (feasible_flag == 0) {
				printf("\n\t Every SP_%d_* has no new col \n\n", Values.iter);
				printf("\n\t Column Generation loop break\n");
				cout << endl;
			}

			loop_continue_flag = feasible_flag;
		}
	}

	Obj_SP1.removeAllProperties();
	Obj_SP1.end();
	Ga_Vars.clear();
	Ga_Vars.end();
	Model_SP1.removeAllProperties();
	Model_SP1.end();
	Env_SP1.removeAllProperties();
	Env_SP1.end();

	return loop_continue_flag; // 函数最终的返回值
}

int SolveStageTwoSubProblem(All_Values& Values, All_Lists& Lists, int strip_type_idx) {

	printf("\n\t SP_%d_%d strip width = %d\n", Values.iter, strip_type_idx, 
		Lists.all_strip_types_list[strip_type_idx-1].width);

	int all_cols_num = Lists.model_matrix.size();
	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;
	int J_num = strip_types_num;
	int N_num = item_types_num;

	int final_return = -1;

	IloEnv Env_SP2; // SP2环境
	IloModel Model_SP2(Env_SP2); // SP2模型
	IloNumVarArray De_Vars(Env_SP2); // SP2

	for (int i = 0; i < N_num; i++) {
		IloNum  var_min = 0; // 
		IloNum  var_max = IloInfinity; // 
		string var_name = "D_" + to_string(i + 1);
		IloNumVar De_Var(Env_SP2, var_min, var_max, ILOINT, var_name.c_str()); // SP2决策变量，整数
		De_Vars.add(De_Var); // SP2决策变量加入list
	}

	IloObjective Obj_SP2(Env_SP2); // obj
	IloExpr obj_sum(Env_SP2);
	IloExpr sum_1(Env_SP2); // con 1
	double sum_val = 0;
	vector<int> fsb_idx;

	for (int i = 0; i < N_num; i++) {
		if (Lists.all_item_types_list[i].width 
			<= Lists.all_strip_types_list[strip_type_idx-1].width) {
			int row_pos = i + N_num;
			double b_val = Lists.dual_prices_list[row_pos];
			if (b_val > 0) {
				obj_sum += b_val * De_Vars[i]; // 
				sum_val += b_val;
				double l_val = Lists.all_item_types_list[i].length;
				sum_1 += l_val * De_Vars[i];
				fsb_idx.push_back(i);
			}
		}
	}
	if (sum_val > 0) {
		Obj_SP2 = IloMaximize(Env_SP2, obj_sum);
		Model_SP2.add(Obj_SP2);
		Model_SP2.add(sum_1 <= Values.stock_length);
		obj_sum.end();
		sum_1.end();
		final_return = 1;
	}
	if (sum_val == 0) {
		printf("\n\t SP_%d_%d does NOT EXIST\n", Values.iter, strip_type_idx);
		sum_1.end();
		final_return = 1;
		Obj_SP2.removeAllProperties();
		Obj_SP2.end();
		De_Vars.clear();
		De_Vars.end();
		Model_SP2.removeAllProperties();
		Model_SP2.end();
		Env_SP2.removeAllProperties();
		Env_SP2.end();
		final_return = 0;
		return final_return;
	}

	printf("\n///////////////// SP_%d_%d CPLEX solving START /////////////////\n\n", Values.iter, strip_type_idx);
	IloCplex Cplex_SP2(Env_SP2);
	Cplex_SP2.extract(Model_SP2);
	//Cplex_SP2.exportModel("Length Sub Problem.lp"); // 输出SP2的lp模型
	bool SP2_flag = Cplex_SP2.solve(); // 求解SP2
	printf("\n///////////////// SP_%d_%d CPLEX solving OVER /////////////////\n\n", Values.iter, strip_type_idx);

	if (SP2_flag == 0) {
		printf("\n\t SP_%d_%d is NOT FEASIBLE\n", Values.iter, strip_type_idx);
	}
	else {
		printf("\n\t SP_%d_%d is FEASIBLE\n", Values.iter, strip_type_idx);

		printf("\n\t Obj = %f\n", Cplex_SP2.getValue(Obj_SP2));
		Values.SP2_obj_val = Cplex_SP2.getValue(Obj_SP2);

		printf("\n\t SP_%d_%d VARS:\n\n", Values.iter, strip_type_idx);
		Lists.SP2_solns_list.clear();
		int fsb_num = fsb_idx.size();

		for (int i = 0; i < N_num; i++) {
			int fsb_flag = -1;
			for (int k = 0; k < fsb_num; k++) {
				if (i == fsb_idx[k]) {
					fsb_flag = 1;
					double soln_val = Cplex_SP2.getValue(De_Vars[i]);
					if (soln_val == -0) {
						soln_val = 0;
					}
					printf("\t var_D_%d = %f\n", i + 1, soln_val);
					Lists.SP2_solns_list.push_back(soln_val);
				}
			}
			if (fsb_flag != 1) {
				Lists.SP2_solns_list.push_back(0);
			}
		}
	}

	Obj_SP2.removeAllProperties();
	Obj_SP2.end();
	De_Vars.clear();
	De_Vars.end();
	Model_SP2.removeAllProperties();
	Model_SP2.end();
	Env_SP2.removeAllProperties();
	Env_SP2.end();

	return final_return;
}
