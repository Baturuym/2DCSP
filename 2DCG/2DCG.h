
// 
// 2022-10-21

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
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iomanip>

#include <ilcplex/ilocplex.h>

using namespace std;

#define RC_EPS 1.0e-6 // a num that is very close to 0

struct Item_Type_Stc
{
	int item_type_idx = -1;
	double demand = -1;

	int length = -1;
	int width = -1;
	int this_item_type_num = 0;
};

struct Strip_Type_Stc
{
	int strip_type_idx = -1;

	int width = -1;
	int length = -1;
	int this_strip_type_num = 0;
};

struct Stock_Type_Stc
{
	int stock_type_idx = -1;
	int this_stock_type_num = 0;
};

struct Item_Stc
{
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

struct Strip_Stc
{
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

struct Stock_Stc
{
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

struct All_Values
{
	int strip_types_num = -1;
	int item_types_num = -1;

	int stocks_num = -1;
	int stock_types_num = -1;

	int stock_length = -1;
	int stock_width = -1;

	int items_num = -1;
	int strips_num = -1;

	int unit_cut_loss = -1;
	int unit_area_loss = -1;

	int final_cut_loss = -1;
	int final_area_loss = -1;

	bool Finish;

	int iter = -1;
	double SP2_obj_val = -1;
	int Y_col_flag = -1;
};

struct All_Lists
{
	vector<Stock_Stc> Y_patterns_list; // 存储每种第一阶段方案（母板）的详细信息
	vector<Strip_Stc> X_patterns_list; // 存储每种第二阶段方案（中间板）的详细信息

	vector<Item_Type_Stc> all_item_types_list;
	vector<Strip_Type_Stc> all_strip_types_list;
	vector<Item_Stc> all_items_list;
	vector<Strip_Stc> all_strips_list;
	vector<Stock_Stc> occupied_stocks_list;
	vector<Item_Stc> occupied_items_list;
	vector<Stock_Stc> all_stocks_list;

	vector<vector<double>> model_matrix; // 存储系数矩阵的所有列
	vector<vector<double>> Y_cols_list; // 存储第一阶段方案的所有列
	vector<vector<double>> X_cols_list; // 存储第二阶段方案的所有列
	vector<double> dual_prices_list;
	vector<double> new_Y_col;
	vector<double> SP2_solns_list;
	vector<vector<double>> new_X_cols_list;

};

///////////////////////////函数声明////////////////////////////////

void SplitString(const string& s, vector<string>& v, const string& c);

void ReadData(All_Values& Values, All_Lists& Lists);

void PrimalHeuristic(All_Values& Values, All_Lists& Lists);

void ColumnGeneration(All_Values& Values, All_Lists& Lists);

void SolveFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP);

int SolveStageOneSubProblem(All_Values& Values, All_Lists& Lists);

int SolveStageTwoSubProblem(All_Values& Values, All_Lists& Lists, int strip_type_idx);

// 生成+求解新的主问题
void SolveUpdateMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP);

void SolveFinalMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP);

void OutputMasterProblem(All_Values& Values, All_Lists& Lists);

void OutputDualMasterProblem(All_Values& Values, All_Lists& Lists);

void DisplaySubProblem(All_Values& Values, All_Lists& Lists, int Name_SP);

void OutputHeuristicResults(All_Values& Values, All_Lists& Lists);

void OutputFinalResults(All_Values& Values, All_Lists& Lists);




