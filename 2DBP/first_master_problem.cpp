// 2023-03-10 CG for 2D-CSP
#include "2DCG.h"
using namespace std;

void SolveFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP)
{
	int stg0_cols_num = Lists.stg0_cols_list.size();
	int stg1_cols_num = Lists.stg1_cols_list.size();
	int stg2_cols_num = Lists.stg2_cols_list.size();

	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	int all_cols_num = stg1_cols_num + stg2_cols_num;
	int all_rows_num = item_types_num + strip_types_num;

	IloNumArray  con_min(Env_MP);
	IloNumArray  con_max(Env_MP);

	for (int row1 = 0; row1 < item_types_num; row1++)
	{
		double demand_val = Lists.item_types_list[row1].demand;

		// con >= demand
		con_min.add(IloNum(demand_val)); // con LB
		con_max.add(IloNum(IloInfinity)); // con UB
	}

	for (int row2 = 0; row2 < strip_types_num; row2++)
	{
		// con >= 0
		con_min.add(IloNum(0)); // con LB
		con_max.add(IloNum(IloInfinity)); // con UB
	}

	Cons_MP = IloRangeArray(Env_MP, con_min, con_max);
	Model_MP.add(Cons_MP);

	con_min.end();
	con_max.end();

	// 第1列 -> 第 K 列
	for (int col = 0; col < stg1_cols_num; col++)
	{
		IloNum obj_para = 1; // 目标函数中 y 对应的系数为 1
		IloNumColumn CplexCol = Obj_MP(obj_para); // 列建模

		// 第1行  -> 第 N+J 行 
		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.stg1_cols_list[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string Y_name = "Y_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, Y_name.c_str());
		Vars_MP.add(var);

		CplexCol.end();
	}

	// 第 K+1 列 -> 第 K + H 列
	for (int col = 0; col < stg2_cols_num; col++)
	{
		int obj_para = 0; // 目标函数中 x 对应的系数为 0
		IloNumColumn CplexCol = Obj_MP(obj_para); // 列建模

		// 第1行 -> 第 N+J 行 
		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.stg2_cols_list[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string X_name = "X_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str());
		Vars_MP.add(var);
		CplexCol.end();
	}

	printf("\n--------Start the CPLEX solving of the FIRST MP--------\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("The First Master Problem.lp");
	bool MP_flag = MP_cplex.solve();
	printf("---------Finish the CPLEX solving of the FIRST MP---------\n\n");

	int fsb_num = 0;
	int int_num = 0;

	if (MP_flag == 0)
	{
		printf("The FIRST MP has NO feasible solution\n");
	}
	else
	{
		printf("The FIRST MP has feasible solution\n");

		printf("\n	Solns of Y:\n\n");
		for (int col = 0; col < stg1_cols_num; col++)
		{
			IloNum soln_val = MP_cplex.getValue(Vars_MP[col]);
			if (soln_val > 0) // feasible soln > 0
			{
				fsb_num++;
				int soln_int_val = int(soln_val);
				if (soln_int_val == soln_val)
				{
					// ATTTENTION:  
					if (soln_int_val >= 1)
					{
						int_num++;
						printf("	var_y_%d = %f int\n", col + 1, soln_val);
					}
				}
				else
				{
					printf("	var_y_%d = %f\n", col + 1, soln_val);
				}
			}
		}

		printf("\n	Solns of X:\n\n");
		for (int col = stg1_cols_num; col < stg1_cols_num + stg2_cols_num; col++)
		{
			IloNum soln_val = MP_cplex.getValue(Vars_MP[col]);
			if (soln_val > 0) // feasible soln > 0
			{
				fsb_num++;
				int soln_int_val = int(soln_val);
				if (soln_int_val == soln_val)
				{
					// ATTTENTION:  
					if (soln_int_val >= 1)
					{
						int_num++;
						printf("	var_x_%d = %f int\n", col + 1, soln_val);
					}
				}
				else
				{
					printf("	var_x_%d = %f\n", col + 1, soln_val);
				}
			}
		}

		Lists.dual_prices_list.clear();
		for (int row = 0; row < all_rows_num; row++)
		{
			double dual_val = MP_cplex.getDual((Cons_MP)[row]);
			Lists.dual_prices_list.push_back(dual_val);
		}

		printf("\n	DUAL PRICES: \n\n");
		Lists.dual_prices_list.clear();
		for (int row = 0; row < all_rows_num; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("	dual_r_%d = %f\n", row + 1, dual_val);
			Lists.dual_prices_list.push_back(dual_val);
		}

		printf("\n	MP-1:\n");
		printf("\n	Lower Bound = %f", MP_cplex.getValue(Obj_MP));
		printf("\n	NUM of all solns = %d", all_cols_num);
		printf("\n	NUM of fsb solns = %d", fsb_num);
		printf("\n	NUM of int solns = %d", int_num);
	}
	cout << endl;
}