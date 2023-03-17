// 2023-03-10 CG for 2D-CSP
#include "2DCG.h"
using namespace std;

void SolveUpdateMasterProblem(
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

	int all_rows_num = item_types_num + strip_types_num;
	int all_cols_num = stg1_cols_num + stg2_cols_num;

	// 加求解Outer SP和Inner SP生成的新列
	int new_cols_num = Lists.new_cols_list.size();
	for (int col = 0; col < new_cols_num; col++) // 
	{
		IloNum obj_para = 1; // 新列对应的目标函数中决策变量的系数均为1
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.new_cols_list[col][row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity;  // var UB

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT); // 非整数松弛变量
		(Vars_MP).add(var);

		CplexCol.end();
	}
	

	printf("\n\n///////////////////////////////// Start the CPLEX solving of the NEW MP /////////////////////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel(" New MP.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////////////////////// Finish the CPLEX solving of the NEW MP /////////////////////////////////\n\n");

	printf("\n	Solns of Y:\n\n");
	int fsb_num = 0;
	int int_num = 0;
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

	cout << endl;
}

void SolveFinalMasterProblem(
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

	int all_rows_num = item_types_num + strip_types_num;
	int all_cols_num = stg1_cols_num + stg2_cols_num;

	printf("\n\n///////////////////////////////// Start the CPLEX solving of the NEW MP /////////////////////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel(" New MP.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////////////////////// Finish the CPLEX solving of the NEW MP /////////////////////////////////\n\n");


	printf("\n	Solns of Y:\n\n");
	int fsb_num = 0;
	int int_num = 0;
	for (int col = 0; col < stg1_cols_num; col++)
	{
		IloNum soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) 
		{
			fsb_num++;
			int soln_int_val = int(soln_val);
			if (soln_int_val == soln_val)
			{
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
		if (soln_val > 0) 
		{
			fsb_num++;
			int soln_int_val = int(soln_val);
			if (soln_int_val == soln_val)
			{
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

	cout << endl;
}