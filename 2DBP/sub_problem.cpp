// Yuming Zhao: https://github.com/Baturuym
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

int SolveOuterSubProblem(All_Values& Values, All_Lists& Lists)
{
	int all_cols_num = Lists.model_matrix.size();

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int P = Lists.strip_cols_list.size();
	int K = Lists.item_cols_list.size();

	int J = strip_types_num;
	int N = item_types_num;

	int final_return = 0;

	IloEnv Env_OSP; // Outer-SP环境
	IloModel Model_OSP(Env_OSP); // Outer-SP模型
	IloNumVarArray Vars_Ga(Env_OSP); // Outer-SP决策变量

	// var >= 0
	IloNum var_min = 0; // var LB
	IloNum var_max = IloInfinity; // var UB

	for (int k = 0; k < J; k++) // 
	{
		string Ga_name = "G_" + to_string(k + 1);
		IloNumVar var_ga(Env_OSP, var_min, var_max, ILOINT,Ga_name.c_str()); //
		Vars_Ga.add(var_ga); // 
	}

	// Outer-SP obj
	IloExpr obj_sum(Env_OSP);
	for (int k = 0; k < J; k++) // vars num = strip_types num
	{
		double a_val = Lists.dual_prices_list[k];
		obj_sum += a_val * Vars_Ga[k]; //  obj: sum (a_j * G_j)
	}

	IloObjective Obj_OSP = IloMaximize(Env_OSP, obj_sum);
	Model_OSP.add(Obj_OSP); // 

	// Outer-SP only one con
	IloExpr con_sum(Env_OSP);
	for (int k = 0; k < J; k++)
	{
		double ws_val = Lists.strip_col_ptns_list[k].width;
		con_sum += ws_val * Vars_Ga[k]; // con: sum (ws_j * G_j) <= W
	}

	Model_OSP.add(con_sum <= Values.stock_width);
	con_sum.end();

	printf("\n///////////////////////////////// Start the CPLEX solving of Outer-SP /////////////////////////////////\n");
	IloCplex Cplex_OSP(Env_OSP);
	Cplex_OSP.extract(Model_OSP);
	Cplex_OSP.exportModel("Outer Sub Problem.lp"); // 
	bool OSP_flag = Cplex_OSP.solve(); // 
	printf("\n///////////////////////////////// Finish the CPLEX solving of Outer-SP /////////////////////////////////\n\n");

	if (OSP_flag == 0)
	{
		printf("\n	Outer-SP NOT FEASIBLE\n");
	}
	else
	{
		printf("\n	Outer-SP FEASIBLE\n");

		printf("\n	OBJ = %f\n", Cplex_OSP.getValue(Obj_OSP));

		double OSP_obj_val = Cplex_OSP.getValue(Obj_OSP);

		vector<double> OSP_solns_list;
		vector<double> OSP_new_col; // 求解Outer-SP获得的新列

		printf("\n	Outer-SP vars:\n");

		printf("\n	Outer-SP Vars：\n\n");
		for (int k = 0; k < J; k++) // strip rows
		{
			double soln_val = Cplex_OSP.getValue(Vars_Ga[k]);
			double var_val = soln_val;
			printf("	var_G_%d = %f\n", k + 1, var_val);

			OSP_new_col.push_back(soln_val); // 
		}

		printf("\n	Inner-SP Vars：\n\n");
		for (int k = 0; k < N; k++) // item rows
		{
			printf("	var_D_%d = 0\n", k + 1);
			OSP_new_col.push_back(0);
		}

		if (OSP_obj_val > 1) // 则求解Outer-SP获得的新列加入当前MP，不用求解Inner SP
		{
			printf("\n\n	Outer-SP reduced cost = %f > 1,  \n", Cplex_OSP.getValue(Obj_OSP));
			printf("\n	No need to solve Inner-SP\n");

			// 插在当前第一阶段列的后面
			Lists.model_matrix.insert(Lists.model_matrix.begin() + Lists.strip_cols_list.size(), OSP_new_col); // 新第一阶段列加入所有列vector
			Lists.strip_cols_list.push_back(OSP_new_col); // 新第一阶段列加入第一阶段列vector
			Lists.new_cols_list.push_back(OSP_new_col); // 新第一阶段列加入当前新列vector

			OSP_new_col.clear();
			final_return = 0;
		}

		// 如果Outer-SP未能求出削减费用
		// 则继续求解这张中间板对应的Inner SP，看能否求出新列
		if (OSP_obj_val <= 1)
		{
			printf("\n	Outer-SP reduced cost = %f <=1 \n", OSP_obj_val);
			printf("\n	Continue to solve Inner SP\n");

			// 当前Outer-SP对应的Inner SP
			// 两层嵌套子问题，内层
			int OSP_solns_num = Vars_Ga.getSize(); // 每个第一阶段列都对应一组Inner SP
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
	int all_cols_num = Lists.model_matrix.size();

	int strip_types_num = Values.strip_types_num;
	int item_types_num = Values.item_types_num;

	int P = Lists.strip_cols_list.size();
	int K = Lists.item_cols_list.size();

	int J = strip_types_num;
	int N = item_types_num;

	int final_return = -1;

	IloEnv Env_ISP; // Inner SP环境
	IloModel Model_ISP(Env_ISP); // Inner SP模型
	IloNumVarArray Vars_De(Env_ISP); // Inner SP

	IloNum  var_min = 0; // 
	IloNum  var_max = IloInfinity; // 

	for (int k = 0; k < N; k++) // 一个子管种类，对应一个Inner SP决策变量
	{
		string De_name = "D_" + to_string(k + 1);
		IloNumVar var_de(Env_ISP, var_min, var_max, ILOINT, De_name.c_str()); // Inner SP决策变量，整数
		Vars_De.add(var_de); // Inner SP决策变量加入list
	}

	// Inner-SP's obj
	IloExpr obj_sum(Env_ISP);
	for (int k = J; k < J+N; k++)  // 一个子管种类，对应一个Inner SP决策变量
	{
		double b_val = Lists.dual_prices_list[k];
		obj_sum += b_val * Vars_De[k]; // 连加：对偶价格*决策变量
	}

	IloObjective Obj_ISP = IloMaximize(Env_ISP, obj_sum); //
	Model_ISP.add(Obj_ISP); //

	// Inner-SP's only one con
	IloExpr con_sum(Env_ISP);
	for (int k = 0; k < N; k++) // 一个子管种类，对应一个Inner SP决策变量
	{
		double li_val = Lists.item_types_list[k].length;
		con_sum += li_val * Vars_De[k];
	}

	Model_ISP.add(con_sum <= Values.stock_length);
	con_sum.end();

	printf("\n///////////////////////////////// Start the CPLEX solving of SP2_%d /////////////////////////////////\n", OSP_iter + 1);
	IloCplex Cplex_ISP(Env_ISP);
	Cplex_ISP.extract(Model_ISP);
	Cplex_ISP.exportModel("Inner Sub Problem.lp"); // 输出Inner SP的lp模型
	bool ISP_flag = Cplex_ISP.solve(); // 求解Inner SP
	printf("\n///////////////////////////////// Finish the CPLEX solving of SP2_%d /////////////////////////////////\n\n", OSP_iter + 1);

	if (ISP_flag == 0)
	{
		printf("\n	Inner-SP NOT FEASIBLE\n");
	}
	else
	{
		printf("\n	Inner-SP FEASIBLE\n");

		printf("\n	OBJ = %f\n", Cplex_ISP.getValue(Obj_ISP));
		double ISP_obj_val = Cplex_ISP.getValue(Obj_ISP);

		printf("\n	Inner SP vars:\n");
		for (int k = 0; k < N; k++)
		{
			double soln_val = Cplex_ISP.getValue(Vars_De[k]);
			printf("	var_De_%d = %f\n", k + 1, soln_val);
		}

		vector<double> ISP_new_col; // 求解Inner SP获得的新列
		if (ISP_obj_val > OSP_soln_val) // Inner SP的目标函数值 > 对应的Outer-SP决策变量值
		{
			printf("\n	Inner SP reduced cost = %f > Outer SP var_Ga_%d = %f\n", 
				ISP_obj_val,OSP_iter+1, OSP_soln_val);

			for (int k = 0; k < J; k++) //  一行一行，中间板行
			{
				if (OSP_iter == k) // 子板和所属中间板对应
				{
					ISP_new_col.push_back(-1); // 对应上的话，值就为-1
				}

				if (OSP_iter != k) // 子板和所属中间板不对应 
				{
					ISP_new_col.push_back(0); // 没对应上的话，值就为0
				}
			}

			for (int k = 0; k < N; k++) // 一行一行，子板行
			{
				double soln_val = Cplex_ISP.getValue(Vars_De[k]);
				ISP_new_col.push_back(soln_val); // 值为Inner SP决策变量值
			}

			printf("\n	SP2 %d New Column:\n", OSP_iter + 1);
			for (int k = 0; k < item_types_num + strip_types_num; k++) // 输出Inner SP的新列
			{
				printf("	%f\n", ISP_new_col[k]);
			}
			final_return = 0; // 求解Inner SP得到负费用列，返回0
		}

		// 如果Inner SP没有求出负费用列
		if (ISP_obj_val <= OSP_soln_val) // Inner SP目标函数小于对应的Outer-SP决策变量值
		{
			printf("\n	Inner SP has POSITIVE reduced cost......\n");

			final_return = 1; // 求解Inner SP没有得到负费用列，返回1，整个算法求解结束
		}

		// Inner SP求得的新列，放到所有系数列的最后
		Lists.model_matrix.insert(Lists.model_matrix.end(), ISP_new_col); // Inner SP新列加入所有列的vector，放在最末尾
		Lists.item_cols_list.push_back(ISP_new_col); // Inner SP新列加入第二阶段列的vector
		Lists.new_cols_list.push_back(ISP_new_col); // Inner SP新列加入当前新生成列vector

		ISP_new_col.clear(); // 清空Inner SP新列
	}
	

	return 0;
}
