// Yuming Zhao: https://github.com/Baturuym
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

/*			     pattern columns
-----------------------------------------
|		K_num			|		P_num			|
| cut-stk-ptn cols	| cut-stp-ptn cols	|
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

		IloNumVar Var_Ga(Env_OSP, var_min, var_max, ILOINT, Ga_name.c_str()); //
		Vars_Ga.add(Var_Ga); // 
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
		double ws_val = Lists.all_strip_types_list[j].width;
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
		printf("\n\t OSP_%d is NOT FEASIBLE\n", Values.iter);
	}
	else
	{
		printf("\n\t OSP_%d is FEASIBLE\n", Values.iter);
		printf("\n\t Obj = %f\n", Cplex_OSP.getValue(Obj_OSP));

		double OSP_obj_val = Cplex_OSP.getValue(Obj_OSP);
		vector<double> OSP_solns_list;

		// print OSP solns
		Lists.new_cutting_stock_col.clear();
		printf("\n\t OSP_%d VARS:\n\n", Values.iter);
		for (int j = 0; j < J_num; j++) // this_strip rows
		{
			double soln_val = Cplex_OSP.getValue(Vars_Ga[j]);
			printf("\t var_G_%d = %f\n", j + 1, soln_val);
			Lists.new_cutting_stock_col.push_back(soln_val);
			OSP_solns_list.push_back(soln_val);
		}

		// print OSP new col
		printf("\n\t OSP_%d new col:\n\n", Values.iter);
		for (int j = 0; j < J_num + N_num; j++)
		{
			if (j < J_num)
			{
				double soln_val = Cplex_OSP.getValue(Vars_Ga[j]);
				printf("\t row_%d = %f\n", j + 1, soln_val);
			}
			else
			{
				printf("\t row_%d = 0\n", j + 1);
				Lists.new_cutting_stock_col.push_back(0);
			}
		}

		if (OSP_obj_val > 1 + RC_EPS) // 则求解OSP获得的新列加入当前MP，不用求解ISP
		{
			printf("\n\n\t OSP reduced cost = %f > 1,  \n", OSP_obj_val);
			printf("\n\t No need to solve Inner-SP\n");

			loop_continue_flag = 1;
		}
		else // 则继续求解这张中间板对应的ISP，看能否求出新列
		{
			printf("\n\t OSP reduced cost = %f <=1 \n", OSP_obj_val);
			printf("\n\t Continue to solve ISP\n");

			Values.ISP_obj_val = -1;
			Lists.new_cutting_strip_cols.clear();

			Lists.ISP_solns_list.clear();
			Lists.ISP_new_col.clear();

			SolveInnerSubProblem(Values, Lists);

			int feasible_flag = 0;

			int K_num = Lists.cutting_stock_cols.size();
			int P_num = Lists.cutting_strip_cols.size();

			for (int k = 0; k < J_num; k++) // all current stk-cut-patterns
			{
				double a_val = Lists.dual_prices_list[k];
				//SolveInnerSubProblem(Values, Lists);

				if (Values.ISP_obj_val > a_val + RC_EPS) //
				{
					feasible_flag = 1;

					printf("\n\t OSP_%d_ISP_%d reduced cost = %f > strip_type con_%d dual = %f:\n",
						Values.iter, k + 1, Values.ISP_obj_val, k + 1, a_val);

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

					printf("\n\t OSP_%d_ISP_%d new col:\n\n", Values.iter, k + 1);
					for (int row = 0; row < J_num + N_num; row++) // 输出ISP的新列
					{
						printf("\t row_%d = %f\n", row + 1, temp_col[row]);
					}
					printf("\n\t Add OSP_%d_ISP_%d new col to MP\n\n", Values.iter, k + 1);

					Lists.new_cutting_strip_cols.push_back(temp_col);

					cout << endl;
				}
				else
				{
					printf("\n\t OSP_%d_ISP_%d reduced cost = %f < strip_type con_%d dual = %f:\n",
						Values.iter, k + 1, Values.ISP_obj_val, k + 1, a_val);
				}
			}

			if (feasible_flag == 0)
			{
				printf("\n\t OSP_%d_ISP has no new col \n\n", Values.iter);
				printf("\n\t Column Generation loop break\n");

				cout << endl;
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

	int J_num = strip_types_num;
	int N_num = item_types_num;

	int final_return = -1;

	IloEnv Env_ISP; // ISP环境
	IloModel Model_ISP(Env_ISP); // ISP模型
	IloNumVarArray Vars_De(Env_ISP); // ISP

	for (int i = 0; i < N_num; i++) // 
	{
		IloNum  var_min = 0; // 
		IloNum  var_max = IloInfinity; // 
		string De_name = "D_" + to_string(i + 1);

		IloNumVar Var_De(Env_ISP, var_min, var_max, ILOINT, De_name.c_str()); // ISP决策变量，整数
		Vars_De.add(Var_De); // ISP决策变量加入list
	}

	// Inner-SP's obj
	IloExpr obj_sum(Env_ISP);
	for (int i = 0; i < N_num; i++)  // 
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
	for (int i = 0; i < N_num; i++) // 
	{
		double li_val = Lists.all_item_types_list[i].length;
		con_sum += li_val * Vars_De[i];
	}

	Model_ISP.add(con_sum <= Values.stock_length);
	con_sum.end();

	printf("\n///////////////// OSP_%d_ISP CPLEX solving START /////////////////\n\n", Values.iter);
	IloCplex Cplex_ISP(Env_ISP);
	Cplex_ISP.extract(Model_ISP);
	Cplex_ISP.exportModel("Inner Sub Problem.lp"); // 输出ISP的lp模型
	bool ISP_flag = Cplex_ISP.solve(); // 求解ISP
	printf("\n///////////////// OSP_%d_ISP CPLEX solving OVER /////////////////\n\n", Values.iter);

	if (ISP_flag == 0)
	{
		printf("\n\t OSP_%d_ISP is NOT FEASIBLE\n", Values.iter);
	}
	else
	{
		printf("\n\t OSP_%d_ISP is FEASIBLE\n", Values.iter);

		printf("\n\t Obj = %f\n", Cplex_ISP.getValue(Obj_ISP));
		Values.ISP_obj_val = Cplex_ISP.getValue(Obj_ISP);

		for (int j = 0; j < J_num; j++)
		{
			Lists.ISP_new_col.push_back(-1);
		}

		printf("\n\t OSP_%d_ISP VARS:\n\n", Values.iter);
		for (int i = 0; i < N_num; i++)
		{
			double soln_val = Cplex_ISP.getValue(Vars_De[i]);
			printf("\t var_D_%d = %f\n", i + 1, soln_val);
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
