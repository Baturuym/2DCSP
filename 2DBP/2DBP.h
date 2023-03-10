
// 2022-10-21
// 头文件，包含所有的结构体和函数的声明

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
///////////////////////////结构体声明+定义////////////////////////////////

struct ItemTypeProperties
{
	int type = -1;
	int count = 0;
	int demand = -1;
	int length = -1;
	int width = -1;
};

struct StripTypeProperties
{
	int type = -1;
	int count = 0;
};

struct StockTypeProperties
{
	int type = -1;
	int count = 0;
};

struct ItemProperties
{
	int type = -1;
	int demand = -1;
	int length = -1;
	int width = -1;
	int area = -1;
	int x_value = -1;
	int y_value = -1;
	int index = -1;
	int Strip_index = -1;
	int stock_index = -1;
	int occupied = 0;

	int cutting_distance = -1;
	int material_cutting_loss = -1;
};

struct StripProperties
{
	int type = -1;
	int pattern = -1;
	vector<ItemProperties> items_list;
	vector<ItemTypeProperties> item_types_list;
	int length = -1;
	int width = -1;
	int area = -1;
	int x_value = -1;
	int y_value = -1;
	int index = -1;
	int stock_index = -1;

	int cutting_distance = -1;
	int material_cutting_loss = -1;

	int wasted_area = -1;
	int material_area_loss = -1;
};

struct StockProperties
{
	int type = 0;
	int pattern = -1;
	vector<StripProperties> strips_list;
	vector<StripTypeProperties> strip_types_list;

	int length = -1;
	int width = -1;
	int area = -1;
	int x_value = -1;
	int y_value = -1;
	int index = -1;

	int cutting_distance = -1;
	int material_cutting_loss = -1;

	int wasted_area = -1;
	int material_area_loss = -1;
};

struct All_Values
{
	int stocks_num = -1;
	int stock_types_num = -1;
	int items_num = -1;
	int strips_num = -1;

	int item_types_num = -1;
	int strip_types_num = -1;

	int stock_length = -1;
	int stock_width = -1;

	int unit_cut_loss = -1;
	int unit_area_loss = -1;

	int final_cut_loss = -1;
	int final_area_loss = -1;

	bool Finish;
};

struct All_Lists
{
	vector<StockProperties> stock_pool_list;
	vector<ItemProperties> items_occupied_list;
	vector<ItemProperties> all_items_list;

	vector<StripProperties> all_strips_list;
	vector<StockProperties> occupied_stocks_list;

	vector<StockProperties> stg1_patterns_list; // 存储每种第一阶段方案（母板）的详细信息
	vector<StripProperties> stg2_patterns_list; // 存储每种第二阶段方案（中间板）的详细信息

	vector<ItemTypeProperties> item_types_list;

	vector<vector<int>> stg0_cols_list; // 存储系数矩阵的所有列
	vector<vector<int>> stg1_cols_list; // 存储第一阶段方案的所有列
	vector<vector<int>> stg2_cols_list; // 存储第二阶段方案的所有列
	vector<vector<int>> new_cols_list; // 存储要加入MP的新列

	vector<float>dual_prices_list;

};

///////////////////////////函数声明////////////////////////////////

// 分割字符串
void SplitString(const string& s, vector<string>& v, const string& c);

// 读取数据
void ReadData(All_Values& Values, All_Lists& Lists);

// 终端输出主问题模型
void DisplayMasterProblem(All_Values& Values, All_Lists& Lists);

// 终端输出对偶主问题模型
void DisplayDualMasterProblem(All_Values& Values, All_Lists& Lists);

// 终端输出主问题求解结果
void DisplayMasterProblemResult(
	All_Values& Values, All_Lists& Lists,
	IloEnv& Env_MP, IloObjective& Obj_MP, IloCplex& Cplex_MP,
	IloRangeArray& Cons_List_MP, IloNumVarArray& Vars_List_MP);

// 终端输出子问题模型
void DisplaySubProblem(All_Values& Values, All_Lists& Lists, int Name_SP);

// 终端输出子问题求解结果
void DisplaySubProblemResult(
	All_Values& Values, All_Lists& Lists,
	IloEnv& Env_SP, IloObjective& Obj_SP, IloCplex& Cplex_MP,
	IloNumVarArray& List_of_Vars_SP, int Name_SP);

// 初始启发式
void CuttingHeuristic(All_Values& Values, All_Lists& Lists);

// 生成+求解初始主问题
void SolveFirstMasterProblem(
	All_Values& Values, All_Lists& Lists,
	IloEnv& Env_MP, IloModel& Model_MP, IloObjective& Obj_MP,
	IloRangeArray& Cons_List_MP, IloNumVarArray& Vars_List_MP);

// 生成+求解子问题
int SolveSubProblem(All_Values& Values, All_Lists& Lists);

// 生成+求解新的主问题
void SolveNewMasterProblem(
	All_Values& Values, All_Lists& Lists,
	IloEnv& Env_MP, IloModel& Model_MP, IloObjective& Obj_MP,
	IloRangeArray& Cons_List_MP, IloNumVarArray& Vars_List_MP, int Final_Solve);


