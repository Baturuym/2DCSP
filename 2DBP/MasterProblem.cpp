// 2023-03-10 BP for 2D-CSP
#include "2DBP.h"
using namespace std;

void SolveFirstMasterProblem(
	All_Values& Values, All_Lists& Lists,
	IloEnv& Env_MP, IloModel& Model_MP, IloObjective& Obj_MP,
	IloRangeArray& Cons_MP, IloNumVarArray& Vars_MP)
{
	size_t stg0_cols_num = Lists.stg0_cols_list.size();
	size_t stg1_cols_num = Lists.stg1_cols_list.size();
	size_t stg2_cols_num = Lists.stg2_cols_list.size();

	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	IloNumArray  con_min(Env_MP);
	IloNumArray  con_max(Env_MP);

	for (int i = 0; i < item_types_num; i++)
	{
		// con >= demand
		con_min.add(IloNum(Lists.item_types_list[i].demand)); // cons LB
		con_max.add(IloNum(IloInfinity)); // cons UB
	}

	for (int i = 0; i < strip_types_num; i++)
	{
		// con >= 0
		con_min.add(IloNum(0)); // cons LB
		con_max.add(IloNum(IloInfinity)); // cons UB
	}

	Cons_MP = IloRangeArray(Env_MP, con_min, con_max);
	Model_MP.add(Cons_MP);


	// 第1列 -> 第 K 列
	for (int col = 0; col < stg1_cols_num; col++)
	{
		int obj_coeff_1 = 1; // 目标函数中 y 对应的系数为 1
		IloNumColumn CplexCol = (Obj_MP)(obj_coeff_1); // 列建模

		// 第1行  -> 第 N+J 行 
		for (int row = 0; row < item_types_num + strip_types_num; row++)
		{
			CplexCol += (Cons_MP)[row](Lists.stg1_cols_list[col][row]);
		}

		string Y_name = "Y_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, Y_name.c_str());
		(Vars_MP).add(var);
		CplexCol.end();
	}

	// 第 K+1 列 -> 第 K + H 列
	for (int col = 0; col < stg2_cols_num; col++)
	{
		int obj_coeff_2 = 0; // 目标函数中 x 对应的系数为 0
		IloNumColumn CplexCol = (Obj_MP)(obj_coeff_2); // 列建模

		// 第1行 -> 第 N+J 行 
		for (int row = 0; row < item_types_num + strip_types_num; row++)
		{
			CplexCol += (Cons_MP)[row](Lists.stg2_cols_list[col][row]);
		}

		string X_name = "X_" + to_string(col + 1);
		IloNum var_min = 0;
		IloNum var_max = IloInfinity;

		IloNumVar var(CplexCol, var_min, var_max, ILOFLOAT, X_name.c_str());
		Vars_MP.add(var);
		CplexCol.end();
	}

	printf("\n--------Start the CPLEX solving of the FIRST MP--------\n");
	IloCplex Cplex_MP(Model_MP);
	Cplex_MP.extract(Model_MP);
	Cplex_MP.exportModel("The First Master Problem.lp");
	bool MP_flag = Cplex_MP.solve();
	printf("---------Finish the CPLEX solving of the FIRST MP---------\n\n");

	if (MP_flag == 0)
		printf("The FIRST MP has NO feasible solution\n");
	if (MP_flag != 0)
		printf("The FIRST MP has feasible solution\n");
	cout << endl;

	Lists.dual_prices_list.clear();
	for (int row = 0; row < item_types_num + strip_types_num; row++)
	{
		float dual_val = Cplex_MP.getDual((Cons_MP)[row]);
		Lists.dual_prices_list.push_back(dual_val);
	}
}