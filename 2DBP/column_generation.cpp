// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

void ColumnGeneration(All_Values& Values, All_Lists& Lists)
{
	IloEnv Env_MP; // MP环境
	IloModel Model_MP(Env_MP); // MP模型
	IloNumVarArray Vars_MP(Env_MP); // MP所有决策变量
	IloRangeArray Cons_MP(Env_MP); // MP所有约束
	IloObjective Obj_MP = IloAdd(Model_MP, IloMinimize(Env_MP)); // MP目标函数

	SolveFirstMasterProblem(Values, Lists, Env_MP, Model_MP, Obj_MP, Cons_MP, Vars_MP); // 求解初始MP

	while (1)
	{
		int SP_flag;

		SP_flag = SolveOuterSubProblem(Values, Lists); // 求解子问题

		// 如果求解SP1和SP2都没有得到负费用列
		if (SP_flag == 1)
		{
			cout << endl;
			break;
		}

		// 如果求解SP1得到负费用列，
		// 或者求解SP1没有得到负费用列，但是求解SP2得到了
		if (SP_flag == 0)
		{
			SolveUpdateMasterProblem(
				Values,
				Lists,
				Env_MP,
				Model_MP,
				Obj_MP,
				Cons_MP,
				Vars_MP); // 继续求解加入SP生成列的更新MP
		}
	}

	SolveFinalMasterProblem(
		Values,
		Lists,
		Env_MP,
		Model_MP,
		Obj_MP,
		Cons_MP,
		Vars_MP); // 最后一次求解MP，得到最优整数解

	Obj_MP.removeAllProperties();
	Obj_MP.end();
	Vars_MP.clear();
	Vars_MP.end();
	Cons_MP.clear();
	Cons_MP.end();
	Model_MP.removeAllProperties();
	Model_MP.end();
	Env_MP.removeAllProperties();
	Env_MP.end();
}