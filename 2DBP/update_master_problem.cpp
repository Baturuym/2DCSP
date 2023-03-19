// Yuming Zhao: https://github.com/Baturuym
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
	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;
	int J = strip_types_num;
	int N = item_types_num;

	int P = Lists.strip_cols.size();
	int K = Lists.item_cols.size();
	int all_cols_num = P + K;
	int all_rows_num = item_types_num + strip_types_num;

	while (1)
	{
		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.new_strip_col[row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		int old_strip_cols_num = Lists.strip_cols.size();
		string Y_name = "Y_" + to_string(old_strip_cols_num + 1);
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity;  // var UB

		IloNumVar Y_var(CplexCol, var_min, var_max, ILOFLOAT, Y_name.c_str()); // 
		(Vars_MP).add(Y_var);

		CplexCol.end(); // must end this IloNumColumn object

		// update vectors
		vector<double>temp_col;
		for (int row = 0; row < all_rows_num; row++)
		{
			double temp_val = Lists.new_strip_col[row];
			temp_col.push_back(temp_val);
		}

		Lists.strip_cols.push_back(temp_col); // update strip cols
		Lists.model_matrix.insert(Lists.model_matrix.begin() + Lists.strip_cols.size(), temp_col); // update model matrix

		break;
	}
	
	int new_item_cols_num = Lists.new_item_cols.size();
	for (int col = 0; col < new_item_cols_num; col++)
	{
		IloNum obj_para = 0; // 
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.new_item_cols[col][row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		int old_item_cols_num = Lists.item_cols.size();
		string X_name = "X_" + to_string(old_item_cols_num + 1);
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity;  // var UB

		IloNumVar X_var(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str()); // 非整数松弛变量
		(Vars_MP).add(X_var);

		CplexCol.end(); // must end this IloNumColumn object
		
		// update vectors
		vector<double>temp_col; 
		for (int row = 0; row < all_rows_num; row++)
		{
			double temp_val = Lists.new_item_cols[col][row];
			temp_col.push_back(temp_val);
		}
		
		Lists.item_cols.push_back(temp_col); // update item cols
		Lists.model_matrix.insert(Lists.model_matrix.end(), temp_col); // update model matrix
	}


	printf("\n\n///////////////////////////////// MP_%d CPLEX solving START /////////////////////////////////\n", Values.iter);
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("Update Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////////////////////// MP_%d CPLEX solving OVER /////////////////////////////////\n\n", Values.iter);

	printf("\n	Y Solns:\n\n");
	for (int col = 0; col < P; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		printf("	var_y_%d = %f\n", col + 1, soln_val);
	}

	printf("\n	X Solns:\n\n");
	for (int col = P; col < P + K; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
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
	int stg0_cols_num = Lists.model_matrix.size();
	int P = Lists.strip_cols.size();
	int K = Lists.item_cols.size();

	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	int all_rows_num = item_types_num + strip_types_num;
	int all_cols_num = P + K;

	printf("\n\n///////////////////////////////// Start the CPLEX solving of the NEW MP /////////////////////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel(" New MP.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////////////////////// Finish the CPLEX solving of the NEW MP /////////////////////////////////\n\n");


	printf("\n	Y Solns:\n\n");
	for (int col = 0; col < P; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		printf("	var_y_%d = %f\n", col + 1, soln_val);
	}

	printf("\n	X Solns:\n\n");
	for (int col = P; col < P + K; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		printf("	var_x_%d = %f\n", col + 1, soln_val);
	}

	printf("\n	MP-1:\n");
	printf("\n	Lower Bound = %f", MP_cplex.getValue(Obj_MP));
	printf("\n	NUM of all solns = %d", all_cols_num);

	cout << endl;
}