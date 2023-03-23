
// 2022-11-23 Branch and Price
// 2023-03-20: Branch and Price for 2DCSP 

#include "2DBP.h"
using namespace std;

int main() {
	clock_t start, finish;
	start = clock();

	All_Lists Lists;
	All_Values Values;

	ReadData(Values, Lists);

	Node_Stc root_node; // Init Root Node
	root_node.index = 1; // Node_Stc index
	Values.branch_status = 0;

	PrimalHeuristic(Values, Lists, root_node); // generate Root Node matrix
	RootNodeColumnGeneration(Values, Lists, root_node);
	Values.search_flag = FinishNode(Values, Lists, root_node); // find the branch var of Root Node
	Lists.all_nodes_list.push_back(root_node);
	Values.root_flag = 1;

	if (Values.search_flag == 0) // continue to BP
	{
		Values.branch_status = 1;
		BranchAndPriceTree(Values, Lists); // Branch and Price loop
	}

	finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("\n\t Process Time = %f seconds\n", duration);

	return 0;
}

