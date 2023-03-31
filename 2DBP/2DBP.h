
// 2023-03-06 
// 
// CG -- col generation
// MP -- master  problem
// SP  -- sub problem
// LB  -- lower bound
// UB -- upper bound
//
// var -- variable
// con -- constraint
// para -- parameter
// col -- col
//
// fsb -- feasible
// int -- int-
// soln -- solution
// val -- val

#include<vector>
#include<queue>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <array>
#include <algorithm>
#include <stdio.h>
#include <ilcplex/ilocplex.h>

using namespace std;

#define RC_EPS 1.0e-6 // a num that is very close to 0

// stock --> strip --> item

struct Item_Type_Stc {
	int item_type_idx = -1;
	double demand = -1;

	int length = -1;
	int width = -1;
	int this_item_type_num = 0;
};

struct Strip_Type_Stc {
	int strip_type_idx = -1;

	int width = -1;
	int length = -1;
	int this_strip_type_num = 0;
};

struct Stock_Type_Stc {
	int stock_type_idx = -1;
	int this_stock_type_num = 0;
};

struct Item_Stc {
	int item_idx = -1;
	int item_type_idx = -1;
	int demand = -1;

	int length = -1;
	int width = -1;
	int area = -1;

	int pos_x = -1;
	int pos_y = -1;

	int strip_idx = -1;
	int stock_idx = -1;

	int occupied = 0;

	int cutting_distance = -1;
	int material_cutting_loss = -1;
};

struct Strip_Stc {
	int strip_idx = -1;
	int strip_type_idx = -1;
	int pattern = -1;

	vector<Item_Stc> items_in_strip_list;
	vector<Item_Type_Stc> item_types_list;

	int length = -1;
	int width = -1;
	int area = -1;

	int pos_x = -1;
	int pos_y = -1;

	int stock_idx = -1;

	int cutting_distance = -1;
	int material_cutting_loss = -1;

	int wasted_area = -1;
	int material_area_loss = -1;
};

struct Stock_Stc {
	int stock_idx = -1;
	int stock_type_idx = 0;
	int pattern = -1;

	vector<Strip_Stc> strips_list;
	vector<Strip_Type_Stc> strip_types_list;

	int length = -1;
	int width = -1;
	int area = -1;

	int pos_x = -1;
	int pos_y = -1;

	int cutting_distance = -1;
	int material_cutting_loss = -1;

	int wasted_area = -1;
	int material_area_loss = -1;
};

// Node
struct Node {
	int index = -1;

	// Values of the Parent Node of this Node
	int parent_index = -1;
	int parent_branching_flag = -1;
	double parent_var_to_branch_val = -1;

	// Values of Node status
	double node_lower_bound = -1; // LB of this Node
	int node_branched_flag = -1; // flag: this Node is the left or the Right Node of its Parent Node, 1 -- left, 2 -- right
	int node_pruned_flag = -1; // flag: this Node is pruned from Tree or not. 1 -- pruned, 0 -- not pruned

	// Values of final branching of this Node
	int var_to_branch_idx = -1; // var-to-branch's col-index of this Node
	double var_to_branch_soln_val = -1; // var-to-branch's soln-val of this Node
	double var_to_branch_int_val_floor = -1; // var-to-branch's floored int-val of this Node
	double var_to_branch_int_val_ceil = -1; // var-to-branch's ceiled int-val of this Node
	double var_to_branch_int_val_final = -1; // var-to-branch's final int-val (floored or ceiled)

	// Lists of final branching of this Node
	vector<int> branched_idx_list; // all branched-vars' col-index on the route from this Node to Root Node
	vector<double> branched_int_val_list; // all branched-vars' int-val (floored or ceiled) on the route from this Node to Root Node
	vector<double> branched_vars_soln_val_list; // all branched-vars' soln-val on the route from this Node to Root Node
	//vector<vector<int>>branched_cols_list;

	vector<double> all_solns_val_list; // final all (include 0) solns of this Node
	//vector<double> fsb_solns_val_list; // final feasible (i.e. non-0) solns of this Node
	//vector<int> fsb_solns_idx_list; // final col-index of feasible-solns of this Node
	//vector<double> int_solns_val_list; // final all int-solns of this Node
	//vector<int> int_solns_idx_list;  // final col-index of int-solns of this Node

	// Lists of one Column Generation iter of this Node
	int iter = -1;
	vector<vector<double>> model_matrix; // model matrix in this CG iter
	vector<double> dual_prices_list; // dual prices of Master Problem cons in this CG iter
	//vector<double> new_col; // one new col from Sub Problem in this CG iter
	//vector<vector<double>> new_cols_list; // new cols from Sub Problem in this CG iter

	/*-------------------------------------*/

	vector<Stock_Stc> cutting_stock_patterns_list; // 存储每种第一阶段方案（母板）的详细信息
	vector<Strip_Stc> cutting_strip_patterns_list; // 存储每种第二阶段方案（中间板）的详细信息

	vector<vector<double>> cutting_stock_cols; // 存储第一阶段方案的所有列
	vector<vector<double>> cutting_strip_cols; // 存储第二阶段方案的所有列

	vector<double> new_cutting_stock_col;
	vector<vector<double>> new_cutting_strip_cols;

	double LSP_obj_val = -1;
	//vector<double> LSP_one_new_col;
	vector<double> LSP_solns_list;

};

struct All_Values {
	bool Finish;

	int item_types_num = -1; // number of all item_types demanded
	int strip_types_num = -1;

	int stocks_num = -1;
	int stock_types_num = -1;

	int stock_length = -1;
	int stock_width = -1;

	int items_num = -1;
	int strips_num = -1;

	int level_num = -1; // number of all node-branch structurew
	int node_num = -1; // number of all Nodes

	int unit_cut_loss = -1;
	int unit_area_loss = -1;

	int final_cut_loss = -1;
	int final_area_loss = -1;

	double tree_optimal_lower_bound = -1; // current optimal lower bound of BP Tree

	// flag of left or right or searching
	// 1 -- new the Left Node,
	// 2 -- new the Right Node
	// 3 -- previously generated Node
	int branch_status = -1;

	// flag of branching or searching
	// 0 -- contiinue to branch generated Node
	// 1 -- stop at generated Node and search for a previously generated Node
	int search_flag = -1;

	// flag of fathoming
	// 1 -- fathom on the Left Node
	// 2 -- fathom on the Right Node
	int fathom_flag = -1;

	// int integerity_nodes_num = -1;
	int root_flag = -1;
};

struct All_Lists {
	vector<Node> all_nodes_list; // list of all Nodes generated

	vector<Stock_Stc> all_stocks_list;
	vector<Strip_Stc> all_strips_list;
	vector<Item_Stc> all_items_list;

	vector<Strip_Type_Stc> all_strip_types_list;
	vector<Item_Type_Stc> all_item_types_list;

	vector<Stock_Stc> occupied_stocks_list;
	vector<Item_Stc> occupied_items_list;
};

void SplitString(const string& s, vector<string>& v, const string& c);

void ReadData(All_Values& Values, All_Lists& Lists);

void PrimalHeuristic(All_Values& Values, All_Lists& Lists, Node& root_node);

void RootNodeColumnGeneration(All_Values& Values, All_Lists& Lists, Node& root_node);

bool SolveRootNodeFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_List_MP,
	IloNumVarArray& Vars_List_MP,
	Node& root_node);

//bool SolveSubProblem(All_Values& Values, All_Lists& Lists, Node& this_node);

int SolveWidthSubProblem(All_Values& Values, All_Lists& Lists, Node& this_node);

void SolveLengthSubProblem(All_Values& Values, All_Lists& Lists, Node& this_node);

void SolveUpdateMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP,
	Node& this_node);

void SolveFinalMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_List_MP,
	IloNumVarArray& Vars_List_MP,
	Node& this_node);

int FinishNode(All_Values& Values, All_Lists& Lists, Node& this_node);

int ChooseVarToBranch(All_Values& Values, All_Lists& Lists, Node& this_node);

int BranchAndPriceTree(All_Values& Values, All_Lists& Lists);

int ChooseNodeToBranch(All_Values& Values, All_Lists& Lists, Node& parent_node);

void GenerateNewNode(All_Values& Values, All_Lists& Lists, Node& new_node, Node& parent_node);

void NewNodeColumnGeneration(All_Values& Values, All_Lists& Lists, Node& this_node, Node& parent_node);

bool SolveNewNodeFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_List_MP,
	IloNumVarArray& Vars_List_MP,
	Node& this_node,
	Node& parent_node);

















