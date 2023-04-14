
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
	IloNumVarArray& Vars_MP) {

	//Lists.model_matrix.push_back(Lists.new_Y_col);

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;
	
	/*			      pattern columns
	---------------------------------------------
	|          P_num         |          K_num         |
	|  cut-stk-ptn cols  |  cut-stp-ptn cols  |
	--------------------------------------------------------
	|                             |                              |             |
	|               C            |              D              | J_num | strip_type cons >= 0
	|                             |                              |             |
	|-------------------------------------------------------
	|                             |                              |	            |
	|              0             |               B             | N_num | item_type cons >= item_type demand
	|                             |                              |             |
	---------------------------------------------------------
	*/

	if (Values.Y_col_flag == 1) {

		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++) {
			IloNum row_para = Lists.new_Y_col[row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		int cols_num = Lists.Y_cols_list.size();
		string var_name = "Y_" + to_string(cols_num + 1);
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity; // var UB
		IloNumVar Var_Y(CplexCol, var_min, var_max, ILOFLOAT, var_name.c_str()); // 
		(Vars_MP).add(Var_Y);

		CplexCol.end(); // must end this IloNumColumn object

		vector<double>new_col;
		double val;
		for (int row = 0; row < all_rows_num; row++) {
			val = Lists.new_Y_col[row];
			new_col.push_back(val);
		}
	
		Lists.Y_cols_list.push_back(new_col); // update this_strip cols
		Lists.model_matrix.insert(Lists.model_matrix.begin() + Lists.Y_cols_list.size(), new_col); // update model matrix
		Lists.new_Y_col.clear();
	}

	if (Values.Y_col_flag == 0) {

		int new_cols_num = Lists.new_X_cols_list.size();
		for (int col = 0; col < new_cols_num; col++) {

			IloNum obj_para = 0; // 
			IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

			for (int row = 0; row < all_rows_num; row++) {
				IloNum row_para = Lists.new_X_cols_list[col][row];
				CplexCol += (Cons_MP)[row](row_para); //
			}

			int old_item_cols_num = Lists.X_cols_list.size();
			string var_name = "X_" + to_string(old_item_cols_num + 1);
			IloNum var_min = 0; // var LB
			IloNum var_max = IloInfinity; // var UB
			IloNumVar Var_X(CplexCol, var_min, var_max, ILOFLOAT, var_name.c_str()); // 
			(Vars_MP).add(Var_X);

			CplexCol.end(); // must end this IloNumColumn object

			vector<double>temp_col;
			for (int row = 0; row < all_rows_num; row++) {
				double val = Lists.new_X_cols_list[col][row];
				temp_col.push_back(val);
			}

			Lists.X_cols_list.push_back(temp_col); // update item cols
			Lists.model_matrix.insert(Lists.model_matrix.end(), temp_col); // update model matrix
		}
		Lists.new_X_cols_list.clear();
	}
	
	OutputMasterProblem(Values, Lists);
	OutputDualMasterProblem(Values, Lists);

	printf("\n\n///////////////// MP-%d CPLEX solving START /////////////////\n", Values.iter);
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	//MP_cplex.exportModel("Update Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////// MP-%d CPLEX solving OVER /////////////////\n\n", Values.iter);
	printf("\n\t Obj = %f\n", MP_cplex.getValue(Obj_MP));

	double sum_vars = 0;
	int Y_fsb_num = 0;
	int X_fsb_num = 0;
	printf("\n\t Y Solns:\n\n");
	for (int col = 0; col < K_num; col++) {
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) {
			Y_fsb_num++;
			sum_vars = sum_vars + soln_val;
			printf("\t var_y_%d = %f\n", col + 1, soln_val);
		}
	}
	printf("\n\t X Soln:\n\n");
	for (int col = K_num; col < K_num + P_num; col++) {
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) {
			X_fsb_num++;
			sum_vars = sum_vars + soln_val;
			printf("\t var_x_%d = %f\n", col + 1 - K_num, soln_val);
		}
	}

	Lists.dual_prices_list.clear();

	printf("\n\t strip cons dual: \n\n");
	for (int row = 0; row < J_num; row++) {
		double dual_val = MP_cplex.getDual(Cons_MP[row]);
		if (dual_val == -0) {
			dual_val = 0;
		}
		Lists.dual_prices_list.push_back(dual_val);
		printf("\t dual_r_%d = %f\n", row + 1, dual_val);
	}
	printf("\n\t item cons dual: \n\n");
	for (int row = J_num; row < J_num + N_num; row++) {
		double dual_val = MP_cplex.getDual(Cons_MP[row]);
		if (dual_val == -0) {
			dual_val = 0;
		}
		Lists.dual_prices_list.push_back(dual_val);
		printf("\t dual_r_%d = %f\n", row + 1, dual_val);
	}

	printf("\n\t MP-%d:", Values.iter);
	printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
	printf("\n\t Sum of all solns = %f", sum_vars);
	printf("\n\t Number of all solns = %d", K_num + P_num);
	printf("\n\t Number of y fsb-solns = %d", Y_fsb_num);
	printf("\n\t Number of x fsb-solns = %d", X_fsb_num);
	printf("\n\t Number of all fsb-solns = %d", Y_fsb_num + X_fsb_num);

	cout << endl;
}

void SolveFinalMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP, 
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP) {

	int K_num = Lists.Y_cols_list.size();
	int P_num = Lists.X_cols_list.size();

	int J_num = Values.strip_types_num;
	int N_num = Values.item_types_num;

	int all_cols_num = K_num + P_num;
	int all_rows_num = J_num + N_num;

	IloNumArray  con_min(Env_MP);
	IloNumArray  con_max(Env_MP);

	for (int row = 0; row < J_num + N_num; row++) {
		if (row < J_num) {
			// con >= 0
			con_min.add(IloNum(0)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
		if (row >= J_num) {
			// con >= item_type demand
			int row_pos = row - J_num;
			double demand_val = Lists.all_item_types_list[row_pos].demand;
			con_min.add(IloNum(demand_val)); // con LB
			con_max.add(IloNum(IloInfinity)); // con UB
		}
	}
	Cons_MP = IloRangeArray(Env_MP, con_min, con_max);
	Model_MP.add(Cons_MP);
	con_min.end();
	con_max.end();

	OutputMasterProblem(Values, Lists);
	OutputDualMasterProblem(Values, Lists);

	// Matrix C & Matrix  0
	for (int col = 0; col < K_num; col++) { // col 1 -> col K_num
		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = Obj_MP(obj_para); // 
		for (int row = 0; row < J_num + N_num; row++) {  // row 1 -> row J_num+N_num // row 1 -> row J_num+N_num
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string var_name = "Y_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;
		IloNumVar Var_Y(CplexCol, var_min, var_max, ILOINT, var_name.c_str());
		//IloNumVar Var_Y(CplexCol, var_min, var_max, ILOINT, Y_name.c_str());
		Vars_MP.add(Var_Y);

		CplexCol.end();
	}

	// Matrix D & Matrix  B
	for (int col = K_num; col < K_num + P_num; col++) { // col K_num+1 -> col K_num+P_num
		IloNum obj_para = 0; // 
		IloNumColumn CplexCol = Obj_MP(obj_para); // 
		for (int row = 0; row < J_num + N_num; row++) { // row 1 -> row J_num+N_num
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string var_name = "X_" + to_string(col + 1 - K_num);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;
		IloNumVar Var_X(CplexCol, var_min, var_max, ILOINT, var_name.c_str());
		//IloNumVar Var_X(CplexCol, var_min, var_max, ILOINT, X_name.c_str());
		Vars_MP.add(Var_X);
		CplexCol.end();
	}

	printf("\n\n///////////////// MP_final CPLEX solving START /////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("Final Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////// MP_final CPLEX solving OVER /////////////////\n\n");

	double sum_vars = 0;
	int Y_fsb_num = 0;
	int X_fsb_num = 0;
	printf("\n\t Y Solns (stock cutting patterns):\n\n");
	for (int col = 0; col < K_num; col++) {
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) {
			Y_fsb_num++;
			sum_vars = sum_vars + soln_val;
			printf("\t var_Y_%d = %f\n", col + 1, soln_val);
		}
	}
	printf("\n\t X Solns (this_strip cutting patterns):\n\n");
	for (int col = K_num; col < K_num + P_num; col++) {
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) {
			X_fsb_num++;
			sum_vars = sum_vars + soln_val;
			printf("\t var_X_%d = %f\n", col + 1 - K_num, soln_val);

		}
	}

	printf("\n\t MP-final:\n");
	printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
	printf("\n\t Sum of all solns = %f", sum_vars);
	printf("\n\t Number of all solns = %d", K_num + P_num);
	printf("\n\t Number of Y fsb-solns = %d", Y_fsb_num);
	printf("\n\t Number of X fsb-solns = %d", X_fsb_num);
	printf("\n\t Number of all fsb-solns = %d", Y_fsb_num + X_fsb_num);

	cout << endl;
}