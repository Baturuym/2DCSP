
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

int SolveStockSubProblem(All_Values& Values, All_Lists& Lists) {
	int K_num = Lists.cutting_stock_cols.size();
	int P_num = Lists.cutting_strip_cols.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	/*			    pattern columns
	-----------------------------------------
	|		 P_num			|		K_num			|
	| cut-stk-ptn cols	| cut-stp-ptn cols	|
	-----------------------------------------------------
	|							|							|				|
	|			 C				|			D				|  J_num	|	strip_type cons >= 0
	|							|							|				|
	|----------------------------------------------------
	|							|							|				|
	|			 0				|			B				|  N_num	|	item_type cons >= item_type demand
	|							|							|				|
	-----------------------------------------------------
	*/

	int loop_continue_flag = -1;

	IloEnv Env_KSP; // KSP环境
	IloModel Model_KSP(Env_KSP); // KSP模型
	IloNumVarArray Vars_Ga(Env_KSP); // KSP决策变量

	for (int j = 0; j < J_num; j++) // 
	{
		// var >= 0
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity; // var UB
		string Ga_name = "G_" + to_string(j + 1);

		IloNumVar Var_Ga(Env_KSP, var_min, var_max, ILOINT, Ga_name.c_str()); //
		Vars_Ga.add(Var_Ga); // 
	}

	// KSP obj
	IloExpr obj_sum(Env_KSP);
	for (int j = 0; j < J_num; j++) // vars num = strip_types num
	{
		double a_val = Lists.dual_prices_list[j];
		obj_sum += a_val * Vars_Ga[j]; //  obj: sum (a_j * G_j)
	}

	IloObjective Obj_KSP = IloMaximize(Env_KSP, obj_sum);
	Model_KSP.add(Obj_KSP); // 
	obj_sum.end();

	// KSP only one con
	IloExpr con_sum(Env_KSP);
	for (int j = 0; j < J_num; j++) {
		double ws_val = Lists.all_strip_types_list[j].width;
		con_sum += ws_val * Vars_Ga[j]; // con: sum (ws_j * G_j) <= W
	}

	Model_KSP.add(con_sum <= Values.stock_width);
	con_sum.end();

	printf("\n///////////////// KSP_%d CPLEX solving START /////////////////\n\n", Values.iter);
	IloCplex Cplex_KSP(Env_KSP);
	Cplex_KSP.extract(Model_KSP);
	Cplex_KSP.exportModel("Outer Sub Problem.lp"); // 
	bool KSP_flag = Cplex_KSP.solve(); // 
	printf("\n///////////////// KSP_%d CPLEX solving OVER /////////////////\n\n", Values.iter);

	if (KSP_flag == 0) {
		printf("\n\t KSP_%d is NOT FEASIBLE\n", Values.iter);
	}
	else {
		printf("\n\t KSP_%d is FEASIBLE\n", Values.iter);
		printf("\n\t Obj = %f\n", Cplex_KSP.getValue(Obj_KSP));

		double KSP_obj_val = Cplex_KSP.getValue(Obj_KSP);
		vector<double> KSP_solns_list;

		// print KSP solns
		Lists.new_cutting_stock_col.clear();
		printf("\n\t KSP_%d VARS:\n\n", Values.iter);
		for (int j = 0; j < J_num; j++) // this_strip rows
		{
			double soln_val = Cplex_KSP.getValue(Vars_Ga[j]);
			printf("\t var_G_%d = %f\n", j + 1, soln_val);
			Lists.new_cutting_stock_col.push_back(soln_val);
			KSP_solns_list.push_back(soln_val);
		}

		// print KSP new col
		printf("\n\t KSP_%d new col:\n\n", Values.iter);
		for (int j = 0; j < J_num + N_num; j++) {
			if (j < J_num) {
				double soln_val = Cplex_KSP.getValue(Vars_Ga[j]);
				printf("\t row_%d = %f\n", j + 1, soln_val);
			}
			else {
				printf("\t row_%d = 0\n", j + 1);
				Lists.new_cutting_stock_col.push_back(0);
			}
		}

		if (KSP_obj_val > 1 + RC_EPS) // 则求解KSP获得的新列加入当前MP，不用求解ISP
		{
			printf("\n\n\t KSP reduced cost = %f > 1,  \n", KSP_obj_val);
			printf("\n\t No need to solve Inner-SP\n");

			loop_continue_flag = 1;
		}
		else // 则继续求解这张中间板对应的ISP，看能否求出新列
		{
			printf("\n\t KSP reduced cost = %f <=1 \n", KSP_obj_val);
			printf("\n\t Continue to solve ISP\n");

			Values.ISP_obj_val = -1;
			Lists.new_cutting_strip_cols.clear();

			Lists.ISP_solns_list.clear();
			Lists.ISP_new_col.clear();

			SolveStripSubProblem(Values, Lists);

			int feasible_flag = 0;

			int K_num = Lists.cutting_stock_cols.size();
			int P_num = Lists.cutting_strip_cols.size();

			for (int k = 0; k < J_num; k++) // all current stk-cut-patterns
			{
				double a_val = Lists.dual_prices_list[k];
				//SolveStripSubProblem(Values, Lists);

				if (Values.ISP_obj_val > a_val + RC_EPS) //
				{
					feasible_flag = 1;

					printf("\n\t KSP_%d_PSP_%d reduced cost = %f > strip_type con_%d dual = %f:\n",
						Values.iter, k + 1, Values.ISP_obj_val, k + 1, a_val);

					vector<double> temp_col; // 
					for (int j = 0; j < J_num; j++) //  all current stp-cut-patterns 
					{
						if (k == j) // this stp-cut-pattern p is USED in stk-cut-pattern k
						{
							temp_col.push_back(-1); // used
						}
						else {
							temp_col.push_back(0); // not used
						}
					}

					for (int i = 0; i < N_num; i++) // 
					{
						double D_soln_val = Lists.ISP_solns_list[i];
						temp_col.push_back(D_soln_val); // 
					}

					printf("\n\t KSP_%d_PSP_%d new col:\n\n", Values.iter, k + 1);
					for (int row = 0; row < J_num + N_num; row++) // 输出ISP的新列
					{
						printf("\t row_%d = %f\n", row + 1, temp_col[row]);
					}
					printf("\n\t Add KSP_%d_PSP_%d new col to MP\n\n", Values.iter, k + 1);

					Lists.new_cutting_strip_cols.push_back(temp_col);

					cout << endl;
				}
				else {
					printf("\n\t KSP_%d_PSP_%d reduced cost = %f < strip_type con_%d dual = %f:\n",
						Values.iter, k + 1, Values.ISP_obj_val, k + 1, a_val);
				}
			}

			if (feasible_flag == 0) {
				printf("\n\t KSP_%d_PSP has no new col \n\n", Values.iter);
				printf("\n\t Column Generation loop break\n");

				cout << endl;
			}

			loop_continue_flag = feasible_flag;
		}
	}

	Obj_KSP.removeAllProperties();
	Obj_KSP.end();
	Vars_Ga.clear();
	Vars_Ga.end();
	Model_KSP.removeAllProperties();
	Model_KSP.end();
	Env_KSP.removeAllProperties();
	Env_KSP.end();

	return loop_continue_flag; // 函数最终的返回值
}

void SolveStripSubProblem(All_Values& Values, All_Lists& Lists) {
	int all_cols_num = Lists.model_matrix.size();

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int J_num = strip_types_num;
	int N_num = item_types_num;

	int final_return = -1;

	IloEnv Env_PSP; // ISP环境
	IloModel Model_PSP(Env_PSP); // ISP模型
	IloNumVarArray Vars_De(Env_PSP); // ISP

	for (int i = 0; i < N_num; i++) // 
	{
		IloNum  var_min = 0; // 
		IloNum  var_max = IloInfinity; // 
		string De_name = "D_" + to_string(i + 1);

		IloNumVar Var_De(Env_PSP, var_min, var_max, ILOINT, De_name.c_str()); // ISP决策变量，整数
		Vars_De.add(Var_De); // ISP决策变量加入list
	}

	// Inner-SP's obj
	IloExpr obj_sum(Env_PSP);
	for (int i = 0; i < N_num; i++)  // 
	{
		int row_pos = i + N_num;
		double b_val = Lists.dual_prices_list[row_pos];
		obj_sum += b_val * Vars_De[i]; // 连加：对偶价格*决策变量
	}

	IloObjective Obj_PSP = IloMaximize(Env_PSP, obj_sum); //
	Model_PSP.add(Obj_PSP); //
	obj_sum.end();

	// Inner-SP's only one con
	IloExpr con_sum(Env_PSP);
	for (int i = 0; i < N_num; i++) // 
	{
		double li_val = Lists.all_item_types_list[i].length;
		con_sum += li_val * Vars_De[i];
	}

	Model_PSP.add(con_sum <= Values.stock_length);
	con_sum.end();

	printf("\n///////////////// KSP_%d_PSP CPLEX solving START /////////////////\n\n", Values.iter);
	IloCplex Cplex_PSP(Env_PSP);
	Cplex_PSP.extract(Model_PSP);
	Cplex_PSP.exportModel("Inner Sub Problem.lp"); // 输出ISP的lp模型
	bool ISP_flag = Cplex_PSP.solve(); // 求解ISP
	printf("\n///////////////// KSP_%d_PSP CPLEX solving OVER /////////////////\n\n", Values.iter);

	if (ISP_flag == 0) {
		printf("\n\t KSP_%d_PSP is NOT FEASIBLE\n", Values.iter);
	}
	else {
		printf("\n\t KSP_%d_PSP is FEASIBLE\n", Values.iter);

		printf("\n\t Obj = %f\n", Cplex_PSP.getValue(Obj_PSP));
		Values.ISP_obj_val = Cplex_PSP.getValue(Obj_PSP);

		for (int j = 0; j < J_num; j++) {
			Lists.ISP_new_col.push_back(-1);
		}

		printf("\n\t KSP_%d_PSP VARS:\n\n", Values.iter);
		for (int i = 0; i < N_num; i++) {
			double soln_val = Cplex_PSP.getValue(Vars_De[i]);
			printf("\t var_D_%d = %f\n", i + 1, soln_val);
			Lists.ISP_new_col.push_back(soln_val);
			Lists.ISP_solns_list.push_back(soln_val);
		}
	}

	Obj_PSP.removeAllProperties();
	Obj_PSP.end();
	Vars_De.clear();
	Vars_De.end();
	Model_PSP.removeAllProperties();
	Model_PSP.end();
	Env_PSP.removeAllProperties();
	Env_PSP.end();
}
