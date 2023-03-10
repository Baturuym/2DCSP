// 2023-03-10 BP for 2D-CSP
#include "2DBP.h"
using namespace std;

void SolveUpdateMasterProblem(
	All_Values& Values, All_Lists& Lists,
	IloEnv& Env_MP, IloModel& Model_MP, IloObjective& Obj_MP,
	IloRangeArray& Cons_MP, IloNumVarArray& Vars_MP, int Final_Solve)
{
	int stg0_cols_num = Lists.stg0_cols_list.size();
	int stg1_cols_num = Lists.stg1_cols_list.size();
	int stg2_cols_num = Lists.stg2_cols_list.size();
	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;


	// 加求解Outer SP和Inner SP生成的新列
	int new_cols_num = Lists.new_cols_list.size();
	for (int col = 0; col < new_cols_num; col++) // 一列一列，Outer SP和Outer SP生成的所有新列
	{
		int Obj_Coeff = 1; // 新列对应的目标函数中决策变量的系数均为1
		IloNumColumn CplexCol = (Obj_MP)(Obj_Coeff); // 生成一个列对象

		for (int row = 0; row < item_types_num + strip_types_num; row++)
		{
			CplexCol += (Cons_MP)[row](Lists.new_cols_list[col][row]); // 赋值
		}

		float var_min = 0; // MP决策变量下界
		float var_max = IloInfinity;  // MP决策变量上界

		// 如果是最后一次求解MP
		if (Final_Solve == 0)
		{
			IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT); // 非整数松弛变量
			(Vars_MP).add(var);
		}
		// 如果不是最后一次求解MP
		if (Final_Solve == 1)
		{
			IloNumVar var(CplexCol, var_min, var_max, ILOINT); // 非整数松弛变量 
			(Vars_MP).add(var);
		}
		CplexCol.end();
	}
	Lists.new_cols_list.clear(); // 清空所有新列的vector

	printf("\n\n---------Start the CPLEX solving of the NEW MP---------\n");
	IloCplex Cplex_MP(Model_MP);
	Cplex_MP.extract(Model_MP);
	Cplex_MP.exportModel(" New MP.lp"); // 输出当前MP的模型 
	Cplex_MP.solve(); // 求解当前MP
	printf("---------Finish the CPLEX solving of the NEW MP---------\n\n");

	Lists.dual_prices_list.clear();
	for (int row = 0; row < item_types_num + strip_types_num; row++) // 一行一行，所有的子板列
	{
		Lists.dual_prices_list[row] = Cplex_MP.getDual((Cons_MP)[row]); // 对一行约束getDual()求对偶解
	}

	printf("The objective function is %f\n\n", Cplex_MP.getValue(Obj_MP));
	printf("\n");
}