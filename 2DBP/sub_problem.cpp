// Yuming Zhao: https://github.com/Baturuym
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

/*			pattern columns
-----------------------------------------
|		P_num			|		K_num			|
| stk-cut-ptn cols	| stp-cut-tpn cols	|
-----------------------------------------------------
|							|							|				|
|			 C				|			D				|  J_num	|	strip_type rows >= 0
|							|							|				|
|----------------------------------------------------
|							|							|				|
|			 0				|			B				|  N_num	|	item_type rows >= item_type demand
|							|							|				|
-----------------------------------------------------
*/

int SolveOuterSubProblem(All_Values& Values, All_Lists& Lists)
{
	int all_cols_num = Lists.model_matrix.size();

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int P_num = Lists.stock_cut_cols.size();
	int K_num = Lists.strip_cut_cols.size();

	int J_num = strip_types_num;
	int N_num = item_types_num;

	int loop_continue_flag = -1;

	IloEnv Env_OSP; // OSP环境
	IloModel Model_OSP(Env_OSP); // OSP模型
	IloNumVarArray Vars_Ga(Env_OSP); // OSP决策变量

	for (int j = 0; j < J_num; j++) // 
	{
		// var >= 0
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity; // var UB
		string Ga_name = "G_" + to_string(j + 1);

		IloNumVar var_ga(Env_OSP, var_min, var_max, ILOINT, Ga_name.c_str()); //
		Vars_Ga.add(var_ga); // 
	}

	// OSP obj
	IloExpr obj_sum(Env_OSP);
	for (int j = 0; j < J_num; j++) // vars num = strip_types num
	{
		double a_val = Lists.dual_prices_list[j];
		obj_sum += a_val * Vars_Ga[j]; //  obj: sum (a_j * G_j)
	}

	IloObjective Obj_OSP = IloMaximize(Env_OSP, obj_sum);
	Model_OSP.add(Obj_OSP); // 
	obj_sum.end();

	// OSP only one con
	IloExpr con_sum(Env_OSP);
	for (int j = 0; j < J_num; j++)
	{
		double ws_val = Lists.strip_types_list[j].width;
		con_sum += ws_val * Vars_Ga[j]; // con: sum (ws_j * G_j) <= W
	}

	Model_OSP.add(con_sum <= Values.stock_width);
	con_sum.end();

	printf("\n///////////////// OSP_%d CPLEX solving START /////////////////\n\n", Values.iter);
	IloCplex Cplex_OSP(Env_OSP);
	Cplex_OSP.extract(Model_OSP);
	Cplex_OSP.exportModel("Outer Sub Problem.lp"); // 
	bool OSP_flag = Cplex_OSP.solve(); // 
	printf("\n///////////////// OSP_%d CPLEX solving OVER /////////////////\n\n", Values.iter);

	if (OSP_flag == 0)
	{
		printf("\n	OSP_%d is NOT FEASIBLE\n",Values.iter);
	}
	else
	{
		printf("\n	OSP_%d is FEASIBLE\n",Values.iter);
		printf("\n	Obj = %f\n", Cplex_OSP.getValue(Obj_OSP));

		double OSP_obj_val = Cplex_OSP.getValue(Obj_OSP);
		vector<double> OSP_solns_list;

		// print OSP solns
		Lists.new_strip_col.clear();
		printf("\n	OSP_%d VARS:\n",Values.iter);
		for (int j = 0; j < J_num; j++) // strip rows
		{
			double soln_val = Cplex_OSP.getValue(Vars_Ga[j]);
			printf("	var_G_%d = %f\n", j + 1, soln_val);
			Lists.new_strip_col.push_back(soln_val); 
			OSP_solns_list.push_back(soln_val);
		}

		// print OSP new col
		printf("\n	OSP_%d new col:\n\n", Values.iter);
		for (int j = 0; j < J_num+N_num; j++)
		{
			if (j < J_num)
			{
				double soln_val = Cplex_OSP.getValue(Vars_Ga[j]);
				printf("	row_%d = %f\n", j + 1, soln_val);
			}
			else
			{
				printf("	row_%d = 0\n", j + 1);
				Lists.new_strip_col.push_back(0);
			}		
		}

		if (OSP_obj_val > 1) // 则求解OSP获得的新列加入当前MP，不用求解ISP
		{
			printf("\n\n	OSP reduced cost = %f > 1,  \n", OSP_obj_val);
			printf("\n	No need to solve Inner-SP\n");

			loop_continue_flag = 1;
		}
		else // 则继续求解这张中间板对应的ISP，看能否求出新列
		{
			printf("\n	OSP reduced cost = %f <=1 \n", OSP_obj_val);
			printf("\n	Continue to solve ISP\n");

			SolveInnerSubProblem(Values, Lists);

			Values.ISP_obj_val = -1;
			Lists.new_item_cols.clear();

			Lists.ISP_solns_list.clear();
			Lists.ISP_new_col.clear();

			int feasible_flag = 0;
			for (size_t k = 0; k < K_num; k++) // all current stk-cut-patterns
			{
				double G_soln_val = OSP_solns_list[k];
				if (G_soln_val > 0) // only current feasible stk-cut patterns
				{
					if (Values.ISP_obj_val > 1) // better stp-cut-patterns for stk-cut-patterns
					{
						feasible_flag = 1;

						vector<double> temp_col; // 
						for (int j = 0; j < J_num; j++) //  all current stp-cut-patterns 
						{
							if (k == j) // this stp-cut-pattern p is USED in stk-cut-pattern k
							{
								temp_col.push_back(-1); // used
							}
							else
							{
								temp_col.push_back(0); // not used
							}
						}

						for (int i = 0; i < N_num; i++) // 
						{
							double D_soln_val = Lists.ISP_solns_list[i];
							temp_col.push_back(D_soln_val); // 
						}

						// print ISP new col
						printf("\n	ISP new col:\n\n");
						for (int row = 0; row < J_num + N_num; row++) // 输出ISP的新列
						{
							printf("	row_%d = %f\n", row + 1, temp_col[row]);
						}

						Lists.new_item_cols.push_back(temp_col);

						printf("\n	Add OSP_%d ISP new col to MP\n\n", Values.iter);
					}
					else
					{
						feasible_flag = 0; // no better stp-cut-patterns for stk-cut-patterns

						printf("\n	OSP_%d has no ISP new col \n\n", Values.iter);
						printf("\n	Column Generation loop break\n");
					}
				}
			}

			loop_continue_flag = feasible_flag;
		}
	}

	Obj_OSP.removeAllProperties();
	Obj_OSP.end();
	Vars_Ga.clear();
	Vars_Ga.end();
	Model_OSP.removeAllProperties();
	Model_OSP.end();
	Env_OSP.removeAllProperties();
	Env_OSP.end();

	return loop_continue_flag; // 函数最终的返回值
}

void SolveInnerSubProblem(All_Values& Values, All_Lists& Lists)
{
	int all_cols_num = Lists.model_matrix.size();

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int P_num = Lists.stock_cut_cols.size();
	int K_num = Lists.strip_cut_cols.size();

	int J_num = strip_types_num;
	int N_num = item_types_num;

	int final_return = -1;

	IloEnv Env_ISP; // ISP环境
	IloModel Model_ISP(Env_ISP); // ISP模型
	IloNumVarArray Vars_De(Env_ISP); // ISP

	for (int i = 0; i < N_num; i++) // 一个子管种类，对应一个ISP决策变量
	{
		IloNum  var_min = 0; // 
		IloNum  var_max = IloInfinity; // 
		string De_name = "D_" + to_string(i + 1);

		IloNumVar var_de(Env_ISP, var_min, var_max, ILOINT, De_name.c_str()); // ISP决策变量，整数
		Vars_De.add(var_de); // ISP决策变量加入list
	}

	// Inner-SP's obj
	IloExpr obj_sum(Env_ISP);
	for (int i = 0; i < N_num; i++)  // 一个子管种类，对应一个ISP决策变量
	{
		int row_pos = i + N_num;
		double b_val = Lists.dual_prices_list[row_pos];
		obj_sum += b_val * Vars_De[i]; // 连加：对偶价格*决策变量
	}

	IloObjective Obj_ISP = IloMaximize(Env_ISP, obj_sum); //
	Model_ISP.add(Obj_ISP); //
	obj_sum.end();

	// Inner-SP's only one con
	IloExpr con_sum(Env_ISP);
	for (int i = 0; i < N_num; i++) // 一个子管种类，对应一个ISP决策变量
	{
		double li_val = Lists.item_types_list[i].length;
		con_sum += li_val * Vars_De[i];
	}

	Model_ISP.add(con_sum <= Values.stock_length);
	con_sum.end();

	printf("\n///////////////// OSP_%d ISP CPLEX solving START /////////////////\n", Values.iter);
	IloCplex Cplex_ISP(Env_ISP);
	Cplex_ISP.extract(Model_ISP);
	Cplex_ISP.exportModel("Inner Sub Problem.lp"); // 输出ISP的lp模型
	bool ISP_flag = Cplex_ISP.solve(); // 求解ISP
	printf("\n///////////////// OSP_%d ISP CPLEX solving OVER /////////////////\n\n", Values.iter);

	if (ISP_flag == 0)
	{
		printf("\n	OSP_%d ISP is NOT FEASIBLE\n", Values.iter);
	}
	else
	{
		printf("\n	OSP_%d ISP is FEASIBLE\n", Values.iter);

		printf("\n	Obj = %f\n", Cplex_ISP.getValue(Obj_ISP));
		Values.ISP_obj_val = Cplex_ISP.getValue(Obj_ISP);

		for (int j = 0; j < J_num; j++)
		{
			Lists.ISP_new_col.push_back(-1);
		}

		printf("\n	ISP VARS:\n");
		for (int i = 0; i < N_num; i++)
		{
			double soln_val = Cplex_ISP.getValue(Vars_De[i]);
			printf("	var_D_%d = %f\n", i + 1, soln_val);
			Lists.ISP_new_col.push_back(soln_val);
			Lists.ISP_solns_list.push_back(soln_val);
		}
	}

	Obj_ISP.removeAllProperties();
	Obj_ISP.end();
	Vars_De.clear();
	Vars_De.end();
	Model_ISP.removeAllProperties();
	Model_ISP.end();
	Env_ISP.removeAllProperties();
	Env_ISP.end();
}
