// 2022-11-17
#include "2DBP.h"
using namespace std;

int ChooseNodeToBranch(All_Values& Values, All_Lists& Lists, Node& parent_node) {

	int parent_branch_flag = -1;
	int pos = -1;
	int all_nodes_num = Lists.all_nodes_list.size();

	if (Values.branch_status == 3) { // search for a previously generated unbranched unpruned Node
		for (int k = 0; k < all_nodes_num; k++) {
			if (Lists.all_nodes_list[k].node_branched_flag != 1 &&
				Lists.all_nodes_list[k].node_pruned_flag != 1) { // unbranched unpruned
				if (Lists.all_nodes_list[k].LB < Values.optimal_LB) {
					pos = k; // branch this previously generated Node_(k+1)
					cout << endl;
				}
				else {
					int temp_idx = Lists.all_nodes_list[k].index;
					printf("\n\t Node_%d has to be pruned\n", temp_idx);
					Lists.all_nodes_list[k].node_pruned_flag = 1; // prune this previously generated Node
				}
			}
		}
	}

	if (Values.branch_status != 3) { // continue to branch the Parent Node
		if (Values.root_flag == 1) { // the Parent Node is Root Node
			if (Values.branch_status == 1) { // the the Left Node of Root Node
				pos = all_nodes_num - 1; // sub left index = parent index + 1
			}
			if (Values.branch_status == 2) { // the the Right Node of Root Node
				pos = all_nodes_num - 2; // sub right index = parent index + 2
			}
		}

		if (Values.root_flag != 1) { // the Parent Node is not Root Node
			if (Values.fathom_flag == 1) { // the Parent Node is a Left Node
				if (Values.branch_status == 1) {
					pos = all_nodes_num - 2; // sub left index = parent index + 2
				}
				if (Values.branch_status == 2) {
					pos = all_nodes_num - 3; // sub right index = parent index + 3
				}
			}
			if (Values.fathom_flag == 2) { // the Parent Node is a Right Node
				if (Values.branch_status == 1) {
					pos = all_nodes_num - 1; // sub left index = parent index + 1
				}
				if (Values.branch_status == 2) {
					pos = all_nodes_num - 2; // sub right index = parent index + 2
				}
			}
		}
	}


	if (pos == -1) {
		parent_branch_flag = 0;
		printf("\n\t No Node to branch! \n");
	}
	else {
		parent_branch_flag = 1;
		parent_node = Lists.all_nodes_list[pos];
		parent_node.LB = 1;
		printf("\n\t The Node to branch is Node_%d\n", parent_node.index);
	}

	return parent_branch_flag;
}

void GenerateNewNode(All_Values& Values, All_Lists& Lists, Node& new_node, Node& parent_node) {
	
	int all_nodes_num = Lists.all_nodes_list.size();
	new_node.index = all_nodes_num + 1;
	new_node.LB = -1;

	if (Values.branch_status == 1) {
		printf("\n\t Node_%d is the LEFT branch of Node_%d	\n", new_node.index, parent_node.index);
	}
	if (Values.branch_status == 2) {
		printf("\n\t Node_%d is the RIGHT branch of Node_%d	\n", new_node.index, parent_node.index);
	}

	new_node.parent_index = parent_node.index;
	new_node.parent_branching_flag = Values.branch_status;
	new_node.parent_var_to_branch_val = parent_node.var_to_branch_soln;

	printf("\n###########################################\n");
	printf("###########################################\n");
	printf("################## NEW NODE_%d ##################\n", new_node.index);
	printf("###########################################\n");
	printf("###########################################\n\n");

	new_node.var_to_branch_idx = -1;
	new_node.var_to_branch_soln = -1;
	new_node.var_to_branch_floor = -1;
	new_node.var_to_branch_ceil = -1;
	new_node.var_to_branch_final = -1;

	// Init model matrix of the Node-to-branch
	int all_cols_num = parent_node.model_matrix.size();
	int all_rows_num = parent_node.model_matrix[0].size();
	for (int col = 0; col < all_cols_num; col++) {
		vector<double> temp_col;
		for (int row = 0; row < all_rows_num; row++) {
			double val = parent_node.model_matrix[col][row];
			temp_col.push_back(val);
		}
		new_node.model_matrix.push_back(temp_col); 
	}

	int K_num = parent_node.Y_cols_list.size();
	for (int col = 0; col < K_num; col++) {
		vector<double> temp_col;
		for (int row = 0; row < all_rows_num; row++) {
			double val = parent_node.Y_cols_list[col][row];
			temp_col.push_back(val);
		}
		new_node.Y_cols_list.push_back(temp_col);
	}

	int P_num = parent_node.X_cols_list.size();
	for (int col = 0; col < P_num; col++) {
		vector<double> temp_col;
		for (int row = 0; row < all_rows_num; row++) {
			double val = parent_node.X_cols_list[col][row];
			temp_col.push_back(val);
		}
		new_node.X_cols_list.push_back(temp_col);
	}

	// Init branched-vars list and their col-index list of the Node-to-branch
	int branched_vars_num = parent_node.branched_idx_list.size();
	for (int col = 0; col < branched_vars_num; col++) {
		int temp_idx = parent_node.branched_idx_list[col];
		new_node.branched_idx_list.push_back(temp_idx);
	}

	if (Values.branch_status == 1) {
		new_node.var_to_branch_final = parent_node.var_to_branch_floor;
	}
	if (Values.branch_status == 2) {
		new_node.var_to_branch_final = parent_node.var_to_branch_ceil;
	}

	double final_int_val = new_node.var_to_branch_final;
	if (branched_vars_num <= 1) // if new_node is the left or the Right Node of Root Node
	{
		new_node.branched_int_list.push_back(final_int_val);
	}
	else // other Nodes
	{
		for (int col = 0; col < branched_vars_num - 1; col++) {
			double val = parent_node.branched_int_list[col];
			new_node.branched_int_list.push_back(val);
		}
		new_node.branched_int_list.push_back(final_int_val);
	}

	// Clear all other lists to init them
	new_node.all_solns_val_list.clear();
	new_node.dual_prices_list.clear();

	new_node.new_Y_col.clear();
	new_node.new_X_cols_list.clear();

	/*
	new_node.LSP_one_new_col.clear();
	new_node.new_col.clear();
	new_node.new_cols_list.clear();
	new_node.fsb_solns_val_list.clear();
	new_node.fsb_solns_idx_list.clear();
	new_node.int_solns_idx_list.clear();
	new_node.int_solns_val_list.clear();
	*/

	cout << endl;
}


