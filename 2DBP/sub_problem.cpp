// 2023-03-10 CG for 2D-CSP
#include "2DCG.h"
using namespace std;

int SolveOuterSubProblem(All_Values& Values, All_Lists& Lists)
{
	int stg0_cols_num = Lists.stg0_cols_list.size();
	int stg1_cols_num = Lists.stg1_cols_list.size();
	int stg2_cols_num = Lists.stg2_cols_list.size();

	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	int final_return = 0;

	IloEnv Env_OSP; // Outer SP环境
	IloModel Mode_OSP(Env_OSP); // Outer SP模型
	IloNumVarArray Vars_OSP(Env_OSP); // Outer SP决策变量

	// var >= 0
	IloNum var_min = 0; // var LB
	IloNum var_max = IloInfinity; // var UB

	for (int k = 0; k < strip_types_num; k++) // 一列一列，各个中间板种类对应的Outer SP决策变量
	{
		IloNumVar var(Env_OSP, var_min, var_max, ILOINT); //
		Vars_OSP.add(var); // 
	}

	// Outer SP obj
	IloExpr obj_sum(Env_OSP);
	for (int k = 0; k < strip_types_num; k++) // 中间板种类
	{
		int pos = k + item_types_num;
		obj_sum += Lists.dual_prices_list[pos] * Vars_OSP[k]; // 连加 *决策变量 w_i * C_i
	}

	IloObjective Obj_OSP = IloMaximize(Env_OSP, obj_sum); // 
	Mode_OSP.add(Obj_OSP); // 

	// Outer SP cons
	IloExpr con_sum(Env_OSP);
	for (int i = 0; i < strip_types_num; i++)
	{
		con_sum += Lists.item_types_list[i].width * Vars_OSP[i]; // y_i * C_i
	}

	// con <= stock width
	Mode_OSP.add(con_sum <= Values.stock_width);
	con_sum.end();

	printf("\n///////////////////////////////// Start the CPLEX solving of Outer SP /////////////////////////////////\n");
	IloCplex Cplex_OSP(Env_OSP);
	Cplex_OSP.extract(Mode_OSP);
	Cplex_OSP.exportModel("Outer SP.lp"); // 
	bool OSP_flag = Cplex_OSP.solve(); // 
	printf("\n///////////////////////////////// Finish the CPLEX solving of Outer SP /////////////////////////////////\n\n");

	if (OSP_flag == 0)
	{
		printf("\n	Outer SP has NO feasible solution ......\n");
	}
	else
	{
		printf("\n	Outer SP has feasible solution !\n");

		printf("\n	obj_OSP = %f\n", Cplex_OSP.getValue(Obj_OSP));

		double OSP_obj_val = 0;
		vector<double> OSP_solns_list;
		vector<double> OSP_new_col; // 求解Outer SP获得的新列

		printf("\n	Outer SP vars:\n");
		for (int col = 0; col < item_types_num; col++) // 
		{
			double soln_val = Cplex_OSP.getValue(Vars_OSP[col]);
			OSP_obj_val = OSP_obj_val + soln_val; // 得到最终Outer SP目标函数值
			OSP_solns_list.push_back(soln_val);

			printf("\n	var_%d = %f", col+1,soln_val);
		}

		// 如果Outer SP求出了削减费用
		// 则求解Outer SP获得的新列加入当前MP，不用求解Inner SP
		if (OSP_obj_val > 1)
		{
			printf("\n\n	Outer SP has NEGATIVE reduced cost !\n");
			printf("\n	Outer SP New Column：\n\n");

			for (int k = 0; k < item_types_num; k++) // 一行一行，子板行
			{
				OSP_new_col.push_back(0); // 值为0
				printf("	%f\n", OSP_new_col[k]);
			}
			for (int k = 0; k < strip_types_num; k++) // 一行一行，中间板行
			{
				double soln_val = Cplex_OSP.getValue(Vars_OSP[k]);
				OSP_new_col.push_back(soln_val); // 值为第一阶段决策变量值
				printf("	%f\n", OSP_new_col[k + strip_types_num]);
			}

			// 插在当前第一阶段列的后面
			Lists.stg0_cols_list.insert(Lists.stg0_cols_list.begin() + Lists.stg1_cols_list.size(), OSP_new_col); // 新第一阶段列加入所有列vector
			Lists.stg1_cols_list.push_back(OSP_new_col); // 新第一阶段列加入第一阶段列vector
			Lists.new_cols_list.push_back(OSP_new_col); // 新第一阶段列加入当前新列vector

			OSP_new_col.clear();

			final_return = 0;
		}

		// 如果Outer SP未能求出削减费用
		// 则继续求解这张中间板对应的Inner SP，看能否求出新列
		if (OSP_obj_val <= 1)
		{
			printf("\n	Outer SP has POSITIVE reduced cost ......\n");
			printf("\n	Procceed to solve Inner SP\n");

			// 当前Outer SP对应的Inner SP
			// 两层嵌套子问题，内层
			int OSP_solns_num = Vars_OSP.getSize(); // 循环的次数：每个第一阶段列都对应一组Inner SP，数量是子板种类数量
			for (int OSP_iter = 0; OSP_iter < OSP_solns_num; OSP_iter++) // 
			{
				double soln_val = OSP_solns_list[OSP_iter];
				final_return = SolveInnerSubProblem(Values, Lists, soln_val, OSP_iter);
			}
		}
	}




	return final_return; // 函数最终的返回值
}

int SolveInnerSubProblem(All_Values& Values, All_Lists& Lists, double OSP_soln_val, int OSP_iter)
{
	int final_return = -1;

	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	IloEnv Env_ISP; // Inner SP环境
	IloModel Model_ISP(Env_ISP); // Inner SP模型
	IloNumVarArray Vars_ISP(Env_ISP); // Inner SP

	IloNum  var_min = 0; // Inner SP决策变量下界=0
	IloNum  var_max = IloInfinity; // Inner SP决策变量上界=正无穷

	for (int k = 0; k < item_types_num; k++) // 一个子管种类，对应一个Inner SP决策变量
	{
		IloNumVar var(Env_ISP, var_min, var_max, ILOINT); // Inner SP决策变量，整数
		Vars_ISP.add(var); // Inner SP决策变量加入list
	}

	// 构建Inner SP的目标函数
	IloExpr obj_sum(Env_ISP);
	for (int k = 0; k < item_types_num; k++)  // 一个子管种类，对应一个Inner SP决策变量
	{
		obj_sum += Lists.dual_prices_list[k] * Vars_ISP[k]; // 连加：对偶价格*决策变量
	}
	IloObjective Obj_ISP = IloMaximize(Env_ISP, obj_sum); // 生成目标函数（求最大值）
	Model_ISP.add(Obj_ISP); // 目标函数add()进模型

	// 构建Inner SP的约束
	IloExpr con_sum(Env_ISP);
	for (int k = 0; k < item_types_num; k++) // 一个子管种类，对应一个Inner SP决策变量
	{
		con_sum += Lists.item_types_list[k].length * Vars_ISP[k];
	}

	// 一个中间板中各个子板长度之和必须小于当前中间板，也就是当前母板的长度
	Model_ISP.add(con_sum <= Values.stock_length);
	con_sum.end();

	printf("\n///////////////////////////////// Start the CPLEX solving of SP2_%d /////////////////////////////////\n", OSP_iter + 1);
	IloCplex Cplex_ISP(Env_ISP);
	Cplex_ISP.extract(Model_ISP);
	Cplex_ISP.exportModel("Sub Problem 2.lp"); // 输出Inner SP的lp模型
	Cplex_ISP.solve(); // 求解Inner SP
	printf("\n///////////////////////////////// Finish the CPLEX solving of SP2_%d /////////////////////////////////\n\n", OSP_iter + 1);


	printf("\n	Inner SP vars:\n");
	vector<double> ISP_solns_list;
	double ISP_obj_val = 0;

	for (int k = 0; k < item_types_num; k++)
	{
		double soln_val = Cplex_ISP.getValue(Vars_ISP[k]);
		ISP_obj_val = ISP_obj_val + soln_val; // 输出最终的决策变量值
		ISP_solns_list.push_back(soln_val);

		printf("	var_%d = %f\n",k+1, soln_val);
	}

	vector<double> ISP_new_col; // 求解Inner SP获得的新列
	if (ISP_obj_val > OSP_soln_val) // Inner SP的目标函数值 > 对应的Outer SP决策变量值
	{
		printf("\n	Inner SP has NEGATIVE reduced cost !\n");

		// 一行一行确定Inner SP新列
		for (int ISP_iter = 0; ISP_iter < item_types_num; ISP_iter++) // 一行一行，子板行
		{
			double soln_val = Cplex_ISP.getValue(Vars_ISP[ISP_iter]);
			ISP_new_col.push_back(soln_val); // 值为Inner SP决策变量值
		}

		for (int ISP_iter = 0; ISP_iter < strip_types_num; ISP_iter++) //  一行一行，中间板行
		{
			if (OSP_iter == ISP_iter) // 子板和所属中间板对应
			{
				ISP_new_col.push_back(-1); // 对应上的话，值就为-1
			}

			if (OSP_iter != ISP_iter) // 子板和所属中间板不对应 
			{
				ISP_new_col.push_back(0); // 没对应上的话，值就为0
			}
		}

		printf("\n	SP2 %d New Column:\n", OSP_iter + 1);
		for (int k = 0; k < item_types_num + strip_types_num; k++) // 输出Inner SP的新列
		{
			printf("	%f\n", ISP_new_col[k]);
		}
		final_return = 0; // 求解Inner SP得到负费用列，返回0
	}

	// 如果Inner SP没有求出负费用列
	if (ISP_obj_val <= OSP_soln_val) // Inner SP目标函数小于对应的Outer SP决策变量值
	{
		printf("\n	Inner SP has POSITIVE reduced cost......\n");

		final_return = 1; // 求解Inner SP没有得到负费用列，返回1，整个算法求解结束
	}

	// Inner SP求得的新列，放到所有系数列的最后
	Lists.stg0_cols_list.insert(Lists.stg0_cols_list.end(), ISP_new_col); // Inner SP新列加入所有列的vector，放在最末尾
	Lists.stg2_cols_list.push_back(ISP_new_col); // Inner SP新列加入第二阶段列的vector
	Lists.new_cols_list.push_back(ISP_new_col); // Inner SP新列加入当前新生成列vector

	ISP_new_col.clear(); // 清空Inner SP新列

	return 0;
}
