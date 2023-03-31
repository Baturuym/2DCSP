
// 2023-03-01

#include "2DBP.h"
using namespace std;

void NewNodeColumnGeneration(
	All_Values& Values,
	All_Lists& Lists,
	Node& this_node,
	Node& parent_node) {
	IloEnv Env_MP; // int environment
	IloModel Model_MP(Env_MP); // int model 
	IloObjective Obj_MP = IloAdd(Model_MP, IloMinimize(Env_MP)); // Init and set obj
	IloRangeArray Cons_MP(Env_MP); // Init cons
	IloNumVarArray Vars_MP(Env_MP); // Init vars

	this_node.iter = 0; // The firsth MP index ==0

	bool MP_flag = SolveNewNodeFirstMasterProblem(
		Values,
		Lists,
		Env_MP,
		Model_MP,
		Obj_MP,
		Cons_MP,
		Vars_MP,
		this_node,
		parent_node);

	cout << endl;

	if (MP_flag == 1) {
		while (1) {
			this_node.iter++;

			int SP_flag = SolveWidthSubProblem(Values, Lists, this_node);

			if (SP_flag == 0) {
				break;
			}
			if (SP_flag == 1) {
				SolveUpdateMasterProblem(
					Values,
					Lists,
					Env_MP,
					Model_MP,
					Obj_MP,
					Cons_MP,
					Vars_MP,
					this_node);
			}
		}

		SolveFinalMasterProblem(
			Values,
			Lists,
			Env_MP,
			Model_MP,
			Obj_MP,
			Cons_MP,
			Vars_MP,
			this_node);
	}

	Vars_MP.clear();
	Vars_MP.end();
	Cons_MP.clear();
	Cons_MP.end();
	Obj_MP.removeAllProperties();
	Obj_MP.end();
	Model_MP.removeAllProperties();
	Model_MP.end();
	Env_MP.removeAllProperties();
	Env_MP.end();

	cout << endl;
}
