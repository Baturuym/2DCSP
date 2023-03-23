
// 2022-11-17
#include "2DBP.h"
using namespace std;

// judge the integerity of the Node, and find the branch var
int FinishNode(All_Values& Values, All_Lists& Lists, Node& this_node) {
	// 0 -- some fsb-solns are not int; 1 -- all fsb-solns are int
	int node_int_flag = -1;

	// 0 -- continue to branch this Node; 1 -- search another generated Node
	int tree_search_flag = -1;

	node_int_flag = ChooseVarToBranch(Values, Lists, this_node); // branch this Node

	if (this_node.node_pruned_flag == 1) {
		tree_search_flag = 1;
	}
	else {
		if (node_int_flag == 0)  // NOT ALL non-zero-solns are int in this Node
		{
			int var_idx = this_node.var_to_branch_idx;
			this_node.branched_idx_list.push_back(var_idx);

			double var_val = this_node.var_to_branch_soln_val;
			this_node.branched_vars_soln_val_list.push_back(var_val);

			tree_search_flag = 0;
		}

		if (node_int_flag == 1)  // ALL non-zero-solns are int in this Node
		{
			if (this_node.index == 1) // this node is Root Node 
			{
				Values.tree_optimal_lower_bound = this_node.node_lower_bound;
				printf("\n\t Current Optimal Lower Bound = %f\n", Values.tree_optimal_lower_bound);
			}
			if (this_node.index > 1) // this Node is not Root Node
			{
				if (Values.tree_optimal_lower_bound == -1) // this Node is the first Node with all int-solns
				{
					Values.tree_optimal_lower_bound = this_node.node_lower_bound;
					printf("\n\t Current Optimal Lower Bound = %f\n", Values.tree_optimal_lower_bound);
				}
				else // other Nodes with all int-solns
				{
					if (this_node.node_lower_bound < Values.tree_optimal_lower_bound) {
						Values.tree_optimal_lower_bound = this_node.node_lower_bound;
						printf("\n\t Current Optimal Lower Bound = %f\n", Values.tree_optimal_lower_bound);
					}
					if (this_node.node_lower_bound >= Values.tree_optimal_lower_bound) {
						this_node.node_pruned_flag = 1;
						printf("\n\t Node_%d has to be pruned\n", this_node.index);
					}
				}
			}

			tree_search_flag = 1;
		}
	}

	Lists.occupied_stocks_list.clear();
	Lists.occupied_items_list.clear();
	Lists.all_strips_list.clear();

	// 0 -- continue to branch this Node; 1 -- search another generated Node
	return tree_search_flag;
}


int ChooseVarToBranch(All_Values& Values, All_Lists& Lists, Node& this_node) {
	int node_int_flag = 1; // 0 -- some fsb-solns are not int; 1 -- all fsb-solns are int
	double soln_val;

	// find the var-to-branch of this Node
	int all_solns_num = this_node.all_solns_val_list.size();
	for (int col = 0; col < all_solns_num; col++) {
		soln_val = this_node.all_solns_val_list[col];
		if (soln_val > 0) {
			int soln_int_val = int(soln_val); // judge the integerity
			if (soln_int_val != soln_val) // not an integer
			{
				printf("\n\t Node_%d var_x_%d = %f is NOT an integer\n", this_node.index, col + 1, soln_val);

				this_node.var_to_branch_idx = col; // set the var-to-branch-col index
				this_node.var_to_branch_soln_val = soln_val; // set the var-to-branch	
				this_node.var_to_branch_int_val_floor = floor(soln_val);
				this_node.var_to_branch_int_val_ceil = ceil(soln_val);

				node_int_flag = 0; // continue BP algorithm

				break; // break the loop			
			}
		}
	}

	return node_int_flag;
}



