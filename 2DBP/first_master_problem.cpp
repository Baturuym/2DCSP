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


void SolveFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP)
{

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int J_num = strip_types_num;
	int N_num = item_types_num;

	int P_num = Lists.stock_cut_cols.size();
	int K_num = Lists.strip_cut_cols.size();

	int all_cols_num = P_num + K_num;
	int all_rows_num = item_types_num + strip_types_num;

	

	IloNumArray  con_min(Env_MP);
	IloNumArray  con_max(Env_MP);

	for (int row = 0; row < J_num + N_num; row++)
	{
		if (row < J_num)
		{
			// con >= 0
			con_min.add(IloNum(0)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
		if (row >= J_num)
		{
			// con >= item_type demand
			int row_pos = row - J_num;
			double demand_val = Lists.item_types_list[row_pos].demand;
			con_min.add(IloNum(demand_val)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
	}

	Cons_MP = IloRangeArray(Env_MP, con_min, con_max);
	Model_MP.add(Cons_MP);

	con_min.end();
	con_max.end();

	// Matrix C & Matrix  0
	for (int col = 0; col < P_num; col++) 	// col 1 -> col P_num
	{
		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = Obj_MP(obj_para); // 

		for (int row = 0; row < J_num + N_num; row++) // row 1 -> row J_num+N_num
		{
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string Y_name = "Y_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, Y_name.c_str());
		Vars_MP.add(var);

		CplexCol.end();
	}


	// Matrix D & Matrix  B
	for (int col = P_num; col < P_num + K_num; col++) // col P_num+1 -> col P_num+K_num
	{
		IloNum obj_para = 0; // 目标函数中 x 对应的系数为 0
		IloNumColumn CplexCol = Obj_MP(obj_para); // 列建模

		for (int row = 0; row < J_num + N_num; row++) // row 1 -> row J_num+N_num
		{
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string X_name = "X_" + to_string(col + 1-P_num);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str());
		Vars_MP.add(var);
		CplexCol.end();
	}

	printf("\n///////////////// MP_1 CPLEX solving START /////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("The First Master Problem.lp");
	bool MP_flag = MP_cplex.solve();
	printf("\n///////////////// MP_1 CPLEX solving OVER /////////////////\n\n");

	if (MP_flag == 0)
	{
		printf("\n	The FIRST MP has NO feasible solution\n");
	}
	else
	{
		printf("\n	The FIRST MP has feasible solution\n");

		printf("\n	Y Solns (stock cutting patterns):\n\n");
		for (int col = 0; col < P_num; col++)
		{
			double soln_val = MP_cplex.getValue(Vars_MP[col]);
			printf("	var_Y_%d = %f\n", col + 1, soln_val);
		}

		printf("\n	X Solns (strip cutting patterns):\n\n");
		for (int col = P_num; col < P_num + K_num; col++)
		{
			double soln_val = MP_cplex.getValue(Vars_MP[col]);
			printf("	var_X_%d = %f\n", col + 1 - P_num, soln_val);
		}

		Lists.dual_prices_list.clear();

		printf("\n	strip_type cons dual prices: \n\n");
		for (int row = 0; row < J_num; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("	dual_r_%d = %f\n", row + 1, dual_val);
			Lists.dual_prices_list.push_back(dual_val);
		}

		printf("\n	item_type cons dual prices: \n\n");
		for (int row = J_num; row < J_num + N_num; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("	dual_r_%d = %f\n", row + 1, dual_val);
			Lists.dual_prices_list.push_back(dual_val);
		}

		printf("\n	Lower Bound = %f", MP_cplex.getValue(Obj_MP));
		printf("\n	NUM of all solns = %d", all_cols_num);
	}
	cout << endl;
}