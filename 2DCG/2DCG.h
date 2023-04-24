
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

struct One_Item_Type
{
	int item_type_idx = -1;
	double demand = -1;

	int length = -1;
	int width = -1;
	int this_item_type_num = 0;
};

struct One_Strip_Type
{
	int strip_type_idx = -1;

	int width = -1;
	int length = -1;
	int this_strip_type_num = 0;
};

struct One_Stock_Type
{
	int stock_type_idx = -1;
	int this_stock_type_num = 0;
};

struct One_Item
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

	int occupied_flag = 0;

	int cutting_distance = -1;
	int material_cutting_loss = -1;
};

struct One_Strip
{
	int strip_idx = -1;
	int strip_type_idx = -1;
	int pattern = -1;

	vector<One_Item> items_list;
	vector<One_Item_Type> item_types_list;

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

	int occupied_flag = -1;
};

struct One_Stock
{
	int stock_idx = -1;
	int stock_type_idx = 0;
	int pattern = -1;

	vector<One_Strip> strips_list;
	vector<One_Strip_Type> strip_types_list;

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

struct  X_Plan {
	int strip_type_idx = -1;
	int plan_num = -1;
	int plan_idx = -1;
	vector<int> plan_col;
};

struct Y_Plan {
	int plan_num = -1;
	int plan_idx = -1;
	vector<int> plan_col;
};


struct All_Lists
{
	vector<One_Stock> Y_patterns_list; // 存储每种第一阶段方案（母板）的详细信息
	vector<One_Strip> X_patterns_list; // 存储每种第二阶段方案（中间板）的详细信息

	vector<One_Item_Type> all_item_types_list;
	vector<One_Strip_Type> all_strip_types_list;
	vector<One_Item> all_items_list;
	vector<One_Strip> all_strips_list;
	vector<One_Stock> occupied_stocks_list;
	vector<One_Item> occupied_items_list;
	vector<One_Stock> all_stocks_list;

	vector<vector<int>> model_matrix; // 存储系数矩阵的所有列
	vector<vector<int>> Y_cols_list; // 存储第一阶段方案的所有列
	vector<vector<int>> X_cols_list; // 存储第二阶段方案的所有列
	vector<double> dual_prices_list;
	vector<int> new_Y_col;
	vector<int> SP2_solns_list;
	vector<vector<int>> new_X_cols_list;

	vector<double> X_solns_list;
	vector<double> Y_solns_list;

	vector<Y_Plan> Y_plans_list;
	vector<X_Plan>X_plans_list;

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




