
// 2023-03-20: Branch and Price for 2DCSP

#include "2DBP.h"
using namespace std;

void SolveUpdateMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP,
	Node& this_node){

	//Lists.model_matrix.push_back(Lists.new_Y_col);

	int K_num = this_node.Y_cols_list.size();
	int P_num = this_node.X_cols_list.size();

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

	if (this_node.Y_col_flag == 1) {

		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

		for (int row = 0; row < all_rows_num; row++) {
			IloNum row_para = this_node.new_Y_col[row];
			CplexCol += (Cons_MP)[row](row_para); //
		}

		int cols_num = this_node.Y_cols_list.size();
		string var_name = "Y_" + to_string(cols_num + 1);
		IloNum var_min = 0; // var LB
		IloNum var_max = IloInfinity; // var UB
		IloNumVar Var_Y(CplexCol, var_min, var_max, ILOFLOAT, var_name.c_str()); // 
		(Vars_MP).add(Var_Y);

		CplexCol.end(); // must end this IloNumColumn object

		vector<double>new_col;
		double val;
		for (int row = 0; row < all_rows_num; row++) {
			val = this_node.new_Y_col[row];
			new_col.push_back(val);
		}

		this_node.Y_cols_list.push_back(new_col); // update this_strip cols
		this_node.model_matrix.insert(this_node.model_matrix.begin() + this_node.Y_cols_list.size(), new_col); // update model matrix
		this_node.new_Y_col.clear();
	}

	if (this_node.Y_col_flag == 0) {

		int new_cols_num = this_node.new_X_cols_list.size();
		for (int col = 0; col < new_cols_num; col++) {

			IloNum obj_para = 0; // 
			IloNumColumn CplexCol = (Obj_MP)(obj_para); // 

			for (int row = 0; row < all_rows_num; row++) {
				IloNum row_para = this_node.new_X_cols_list[col][row];
				CplexCol += (Cons_MP)[row](row_para); //
			}

			int old_item_cols_num = this_node.X_cols_list.size();
			string var_name = "X_" + to_string(old_item_cols_num + 1);
			IloNum var_min = 0; // var LB
			IloNum var_max = IloInfinity; // var UB
			IloNumVar Var_X(CplexCol, var_min, var_max, ILOFLOAT, var_name.c_str()); // 
			(Vars_MP).add(Var_X);

			CplexCol.end(); // must end this IloNumColumn object

			vector<double>temp_col;
			for (int row = 0; row < all_rows_num; row++) {
				double val = this_node.new_X_cols_list[col][row];
				temp_col.push_back(val);
			}

			this_node.X_cols_list.push_back(temp_col); // update item cols
			this_node.model_matrix.insert(this_node.model_matrix.end(), temp_col); // update model matrix
		}
		this_node.new_X_cols_list.clear();
	}

	//OutputMasterProblem(Values, Lists,this_node);
	//OutputDualMasterProblem(Values, Lists, this_node);

	printf("\n\n///////////////// MP-%d CPLEX solving START /////////////////\n", this_node.iter);
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	//MP_cplex.exportModel("Update Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////// MP-%d CPLEX solving OVER /////////////////\n\n", this_node.iter);
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

	this_node.dual_prices_list.clear();

	printf("\n\t strip cons dual: \n\n");
	for (int row = 0; row < J_num; row++) {
		double dual_val = MP_cplex.getDual(Cons_MP[row]);
		if (dual_val == -0) {
			dual_val = 0;
		}
		this_node.dual_prices_list.push_back(dual_val);
		printf("\t dual_r_%d = %f\n", row + 1, dual_val);
	}
	printf("\n\t item cons dual: \n\n");
	for (int row = J_num; row < J_num + N_num; row++) {
		double dual_val = MP_cplex.getDual(Cons_MP[row]);
		if (dual_val == -0) {
			dual_val = 0;
		}
		this_node.dual_prices_list.push_back(dual_val);
		printf("\t dual_r_%d = %f\n", row + 1, dual_val);
	}

	printf("\n\t MP-%d:", this_node.iter);
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
	IloNumVarArray& Vars_MP,
	Node& this_node) {

	int K_num = this_node.Y_cols_list.size();
	int P_num = this_node.X_cols_list.size();
	int N_num = Values.item_types_num;
	int J_num = Values.strip_types_num;
	int all_rows_num = N_num + J_num;
	int all_cols_num = K_num + P_num;

	printf("\n\n///////////////// MP_final CPLEX solving START /////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("Final Master Problem.lp"); // 输出当前MP的模型 
	MP_cplex.solve(); // 求解当前MP
	printf("\n///////////////// MP_final CPLEX solving OVER /////////////////\n\n");

	this_node.LB = MP_cplex.getValue(Obj_MP); // set Node LB in the last MP
	printf("\n\t OBJ of Node_%d MP-final is %f \n\n", this_node.index, MP_cplex.getValue(Obj_MP));

	
	for (int col = 0; col < all_cols_num; col++)
	{
		IloNum soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val == -0) {
			soln_val = 0;
		}
		this_node.all_solns_val_list.push_back(soln_val); // Node all solns (including zero-solns)
	}
	
	int sum_vars = 0;
	int Y_fsb_num = 0;
	int X_fsb_num = 0;
	printf("\n\t Y Solns (stock cutting patterns):\n\n");
	for (int col = 0; col < K_num; col++) {
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) {
			Y_fsb_num++;
			sum_vars++;
			printf("\t var_Y_%d = %f\n", col + 1, soln_val);
		}
	}
	printf("\n\t X Solns (this_strip cutting patterns):\n\n");
	for (int col = K_num; col < K_num + P_num; col++) {
		double soln_val = MP_cplex.getValue(Vars_MP[col]);
		if (soln_val > 0) {
			X_fsb_num++;
			sum_vars++;
			printf("\t var_X_%d = %f\n", col + 1 - K_num, soln_val);
		}
	}

	printf("\n\t BRANCHED VARS: \n\n");
	int branched_cols_num = this_node.branched_int_list.size();
	int var_idx = -1;
	double var_int_val = -1;
	for (int k = 0; k < branched_cols_num; k++) {
		var_idx = this_node.branched_idx_list[k] + 1;
		var_int_val = this_node.branched_int_list[k];
		printf("\t var_x_%d = %f branched \n", var_idx, var_int_val);
	}

	printf("\n\t Node_%d MP-1:\n", this_node.index);
	printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
	printf("\n\t Sum of all solns = %d", sum_vars);
	printf("\n\t Number of all solns = %d", K_num + P_num);
	printf("\n\t Number of Y fsb-solns = %d", Y_fsb_num);
	printf("\n\t Number of X fsb-solns = %d", X_fsb_num);
	printf("\n\t Number of all fsb-solns = %d\n", Y_fsb_num + X_fsb_num);

	cout << endl;
}