
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
	IloNumVarArray& Vars_MP) {

	int K_num = Lists.cutting_stock_cols.size();
	int P_num = Lists.cutting_strip_cols.size();

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

	printf("\n\t MP-1 model:\n");
	DisplayMasterProblem(Values, Lists);

	// Matrix C & Matrix  0
	for (int col = 0; col < K_num; col++) {  // col 1 -> col K_num
		IloNum obj_para = 1; // 
		IloNumColumn CplexCol = Obj_MP(obj_para); // 
		for (int row = 0; row < J_num + N_num; row++) {   // row 1 -> row J_num+N_num // row 1 -> row J_num+N_num
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string var_name = "Y_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;
		IloNumVar Var_Y(CplexCol, var_min, var_max, ILOFLOAT, var_name.c_str());
		//IloNumVar Var_Y(CplexCol, var_min, var_max, ILOINT, Y_name.c_str());
		Vars_MP.add(Var_Y);

		CplexCol.end();
	}


	// Matrix D & Matrix  B
	for (int col = K_num; col < K_num + P_num; col++) {  // col K_num+1 -> col K_num+P_num
		IloNum obj_para = 0; // 
		IloNumColumn CplexCol = Obj_MP(obj_para); // 
		for (int row = 0; row < J_num + N_num; row++) {  // row 1 -> row J_num+N_num
			IloNum row_para = Lists.model_matrix[col][row];
			CplexCol += Cons_MP[row](row_para);
		}

		string var_name = "X_" + to_string(col + 1 - K_num);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;
		IloNumVar Var_X(CplexCol, var_min, var_max, ILOFLOAT, var_name.c_str());
		//IloNumVar Var_X(CplexCol, var_min, var_max, ILOINT, X_name.c_str());
		Vars_MP.add(Var_X);
		CplexCol.end();
	}

	printf("\n///////////////// MP_1 CPLEX solving START /////////////////\n");
	IloCplex MP_cplex(Model_MP);
	MP_cplex.extract(Model_MP);
	MP_cplex.exportModel("The First Master Problem.lp");
	bool MP_flag = MP_cplex.solve();
	printf("\n///////////////// MP_1 CPLEX solving OVER /////////////////\n\n");

	if (MP_flag == 0) {
		printf("\n\t MP_1 has NO feasible solution\n");
	}
	else {
		printf("\n\t MP_1 has feasible solution\n");
		printf("\n\t Obj = %f\n",MP_cplex.getValue(Obj_MP));

		int Y_fsb_num = 0;
		int X_fsb_num = 0;
		printf("\n\t Y solns:\n\n");
		for (int col = 0; col < K_num; col++) {
			double soln_val = MP_cplex.getValue(Vars_MP[col]);
			if (soln_val > 0) {
				Y_fsb_num++;
				printf("\t var_y_%d = %f\n", col + 1, soln_val);
			}
		}

		printf("\n\t X solns:\n\n");
		for (int col = K_num; col < K_num + P_num; col++) {
			double soln_val = MP_cplex.getValue(Vars_MP[col]);
			if (soln_val > 0) {
				X_fsb_num++;
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

		printf("\n\t MP-1:\n");
		printf("\n\t Lower Bound = %f", MP_cplex.getValue(Obj_MP));
		printf("\n\t Number of all solns = %d", K_num + P_num);
		printf("\n\t Number of Y fsb-solns = %d", Y_fsb_num);
		printf("\n\t Number of X fsb-solns = %d", X_fsb_num);
		printf("\n\t Number of all fsb-solns = %d", Y_fsb_num + X_fsb_num);
	}
	cout << endl;
}