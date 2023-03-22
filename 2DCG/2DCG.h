
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

struct ItemTypeProperties
{
	int item_type_idx = -1;
	double demand = -1;

	int length = -1;
	int width = -1;
	int this_item_type_num = 0;
};

struct StripTypeProperties
{
	int strip_type_idx = -1;

	int width = -1;
	int length = -1;
	int this_strip_type_num = 0;
};

struct StockTypeProperties
{
	int stock_type_idx = -1;
	int this_stock_type_num = 0;
};

struct ItemProperties
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

struct StripProperties
{
	int strip_idx = -1;
	int strip_type_idx = -1;
	int pattern = -1;

	vector<ItemProperties> items_in_strip_list;
	vector<ItemTypeProperties> item_types_list;

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

struct StockProperties
{
	int stock_idx = -1;
	int stock_type_idx = 0;
	int pattern = -1;

	vector<StripProperties> strips_list;
	vector<StripTypeProperties> strip_types_list;

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
	double ISP_obj_val = -1;
};

struct All_Lists
{
	vector<StockProperties> cutting_stock_patterns_list; // 存储每种第一阶段方案（母板）的详细信息
	vector<StripProperties> cutting_strip_patterns_list; // 存储每种第二阶段方案（中间板）的详细信息

	vector<ItemTypeProperties> all_item_types_list;
	vector<StripTypeProperties> all_strip_types_list;

	vector<ItemProperties> all_items_list;
	vector<StripProperties> all_strips_list;

	vector<StockProperties> occupied_stocks_list;
	vector<ItemProperties> occupied_items_list;

	vector<StockProperties> stock_pool_list;

	vector<vector<double>> model_matrix; // 存储系数矩阵的所有列
	vector<vector<double>> cutting_stock_cols; // 存储第一阶段方案的所有列
	vector<vector<double>> cutting_strip_cols; // 存储第二阶段方案的所有列
	//vector<vector<double>> new_cols; // 存储要加入MP的新列

	vector<double> new_cutting_stock_col;
	vector<vector<double>> new_cutting_strip_cols;

	vector<double> dual_prices_list;

	vector<double> ISP_new_col;
	vector<double> ISP_solns_list;

};

///////////////////////////函数声明////////////////////////////////

void SplitString(const string& s, vector<string>& v, const string& c);

void ReadData(All_Values& Values, All_Lists& Lists);

void InitModelMatrix(All_Values& Values, All_Lists& Lists);

void ColumnGeneration(All_Values& Values, All_Lists& Lists);

void SolveFirstMasterProblem(
	All_Values& Values,
	All_Lists& Lists,
	IloEnv& Env_MP,
	IloModel& Model_MP,
	IloObjective& Obj_MP,
	IloRangeArray& Cons_MP,
	IloNumVarArray& Vars_MP);

int SolveOuterSubProblem(All_Values& Values, All_Lists& Lists);

void SolveInnerSubProblem(All_Values& Values, All_Lists& Lists);

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

void OutPutResults(All_Values& Values, All_Lists& Lists);




