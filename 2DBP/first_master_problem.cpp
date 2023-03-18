// Yuming Zhao: https://github.com/Baturuym
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

	int P = Lists.strip_cols_list.size();
	int K = Lists.item_cols_list.size();

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int J = strip_types_num;
	int N = item_types_num;

	int all_cols_num = P + K;
	int all_rows_num = item_types_num + strip_types_num;

	IloNumArray  con_min(Env_MP);
	IloNumArray  con_max(Env_MP);

	for (int row = 0; row < J + N; row++)
	{
		if (row < J)
		{
			// con >= 0
			con_min.add(IloNum(0)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
		if (row >= J)
		{
			// con >= item_type demand
			int row_pos = row - J;
			double demand_val = Lists.item_types_list[row_pos].demand;
			con_min.add(IloNum(demand_val)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
	}

	Cons_MP = IloRangeArray(Env_MP, con_min, con_max);
	Model_MP.add(Cons_MP);

	con_min.end();
	con_max.end();

	/*
				P						K
	 strip_types cols  item_types cols
	----------------------------------
	|						|						|
	|			C		    |           D			|	J-strip_types rows >= 0
	|						|						|
	|---------------------------------
	|						|						|
	|			0			|			B			|	N-item_types rows >= item_type demand
	|						|						|
	----------------------------------

	*/

	// Matrix C & Matrix  0
	for (int col = 0; col < P; col++) 	// col 1 -> col P
	{
		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = Obj_MP(obj_para); // 

		for (int row = 0; row < J + N; row++) // row 1 -> row J+N
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
	for (int col = P; col < P + K; col++) // col P+1 -> col P+K
	{
		int obj_para = 0; // 目标函数中 x 对应的系数为 0
		IloNumColumn CplexCol = Obj_MP(obj_para); // 列建模

		for (int row = 0; row < J + N; row++) // row 1 -> row J+N
		{
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string X_name = "X_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str());
		Vars_MP.add(var);
		CplexCol.end();
	}

	printf("\n///////////////////////////////// Start the CPLEX solving of the FIRST MP /////////////////////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("The First Master Problem.lp");
	bool MP_flag = MP_cplex.solve();
	printf("\n///////////////////////////////// Finish the CPLEX solving of the FIRST MP /////////////////////////////////\n\n");

	if (MP_flag == 0)
	{
		printf("\n	The FIRST MP has NO feasible solution\n");
	}
	else
	{
		printf("\n	The FIRST MP has feasible solution\n");

		printf("\n	Y Solns:\n\n");
		for (int col = 0; col < P; col++)
		{
			IloNum soln_val = MP_cplex.getValue(Vars_MP[col]);
			printf("	var_y_%d = %f\n", col + 1, soln_val);
		}

		printf("\n	X Solns:\n\n");
		for (int col = P; col < P + K; col++)
		{
			IloNum soln_val = MP_cplex.getValue(Vars_MP[col]);
			printf("	var_x_%d = %f\n", col + 1, soln_val);
		}

		Lists.dual_prices_list.clear();

		printf("\n	Strip cons dual prices: \n\n");
		for (int row = 0; row < J; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("	dual_r_%d = %f\n", row + 1, dual_val);
			Lists.dual_prices_list.push_back(dual_val);
		}

		printf("\n	Item cons dual prices : \n\n");
		for (int row = J; row < J + N; row++)
		{
			double dual_val = MP_cplex.getDual(Cons_MP[row]);
			printf("	dual_r_%d = %f\n", row + 1, dual_val);
			Lists.dual_prices_list.push_back(dual_val);
		}

		printf("\n	MP-1:\n");
		printf("\n	Lower Bound = %f", MP_cplex.getValue(Obj_MP));
		printf("\n	NUM of all solns = %d", all_cols_num);
	}
	cout << endl;
}