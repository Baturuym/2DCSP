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

	while (1)
	{
		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.new_cutting_stock_col[row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		int old_strip_cols_num = Lists.cutting_stock_cols.size();
		string Y_name = "Y_" + to_string(old_strip_cols_num + 1);
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity;  // var UB

		//IloNumVar Var_Y(CplexCol, var_min, var_max, ILOFLOAT, Y_name.c_str()); // 
		IloNumVar Var_Y(CplexCol, var_min, var_max, ILOINT, Y_name.c_str()); // 
		(Vars_MP).add(Var_Y);

		CplexCol.end(); // must end this IloNumColumn object

		// update vectors
		vector<double>temp_col;
		for (int row = 0; row < all_rows_num; row++)
		{
			double temp_val = Lists.new_cutting_stock_col[row];
			temp_col.push_back(temp_val);
		}

		Lists.cutting_stock_cols.push_back(temp_col); // update this_strip cols
		Lists.model_matrix.insert(Lists.model_matrix.begin() + Lists.cutting_stock_cols.size(), temp_col); // update model matrix

		break;
	}

	int new_item_cols_num = Lists.new_cutting_strip_cols.size();
	for (int col = 0; col < new_item_cols_num; col++)
	{
		IloNum obj_para = 0; // 
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++)
		{
			IloNum row_para = Lists.new_cutting_strip_cols[col][row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		int old_item_cols_num = Lists.cutting_strip_cols.size();
		string X_name = "X_" + to_string(old_item_cols_num + 1);
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity;  // var UB

		IloNumVar Var_X(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str()); // 
		//IloNumVar Var_X(CplexCol, var_min, var_max, ILOINT, X_name.c_str()); // 
		(Vars_MP).add(Var_X);

		CplexCol.end(); // must end this IloNumColumn object

		// update vectors
		vector<double>temp_col;
		for (int row = 0; row < all_rows_num; row++)
		{
			double temp_val = Lists.new_cutting_strip_cols[col][row];
			temp_col.push_back(temp_val);
		}

		Lists.cutting_strip_cols.push_back(temp_col); // update item cols
		Lists.model_matrix.insert(Lists.model_matrix.end(), temp_col); // update model matrix
	}


	printf("\n\n///////////////// MP_%d CPLEX solving START /////////////////\n", Values.iter);
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("Update Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////// MP_%d CPLEX solving OVER /////////////////\n\n", Values.iter);

	int Y_fsb_num = 0;
	int X_fsb_num = 0;
	printf("\n\t Y Solns (stock cutting patterns):\n\n");
	for (int col = 0; col < K_num; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0)
		{
			Y_fsb_num++;
			printf("\t var_Y_%d = %f\n", col + 1, soln_val);
		}
	}
	printf("\n\t X Solns (this_strip cutting patterns):\n\n");
	for (int col = K_num; col < K_num + P_num; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0)
		{
			X_fsb_num++;
			printf("\t var_X_%d = %f\n", col + 1 - K_num, soln_val);
		}
	}

	Lists.dual_prices_list.clear();

	printf("\n\t strip_type cons dual prices: \n\n");
	for (int row = 0; row < J_num; row++)
	{
		double dual_val = MP_cplex.getDual(Cons_MP[row]);
		printf("\t dual_r_%d = %f\n", row + 1, dual_val);
		Lists.dual_prices_list.push_back(dual_val);
	}
	printf("\n\t item_type cons dual prices: \n\n");
	for (int row = J_num; row < J_num + N_num; row++)
	{
		double dual_val = MP_cplex.getDual(Cons_MP[row]);
		printf("\t dual_r_%d = %f\n", row + 1, dual_val);
		Lists.dual_prices_list.push_back(dual_val);
	}

	printf("\n\t MP-%d:\n", Values.iter);
	printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
	printf("\n\t NUM of all solns = %d", K_num + P_num);
	printf("\n\t NUM of Y fsb solns = %d", Y_fsb_num);
	printf("\n\t NUM of X fsb solns = %d", X_fsb_num);
	printf("\n\t NUM of all fsb solns = %d", Y_fsb_num + X_fsb_num);

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
	int K_num = Lists.cutting_stock_cols.size();
	int P_num = Lists.cutting_strip_cols.size();

	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	int all_rows_num = item_types_num + strip_types_num;
	int all_cols_num = K_num + P_num;

	printf("\n\n///////////////// MP_final CPLEX solving START /////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("Final Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////// MP_final CPLEX solving OVER /////////////////\n\n");

	int Y_fsb_num = 0;
	int X_fsb_num = 0;
	printf("\n\t Y Solns (stock cutting patterns):\n\n");
	for (int col = 0; col < K_num; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0)
		{
			Y_fsb_num++;
			printf("\t var_Y_%d = %f\n", col + 1, soln_val);
		}
	}

	printf("\n\t X Solns (this_strip cutting patterns):\n\n");
	for (int col = K_num; col < K_num + P_num; col++)
	{
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0)
		{
			X_fsb_num++;
			printf("\t var_X_%d = %f\n", col + 1 - K_num, soln_val);

		}
	}

	printf("\n\t MP-final:\n");
	printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
	printf("\n\t NUM of all solns = %d", K_num + P_num);
	printf("\n\t NUM of Y fsb solns = %d", Y_fsb_num);
	printf("\n\t NUM of X fsb solns = %d", X_fsb_num);
	printf("\n\t NUM of all fsb solns = %d", Y_fsb_num + X_fsb_num);

	cout << endl;
}