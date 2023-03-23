
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

int main() {
	All_Values Values;
	All_Lists Lists;

	ReadData(Values, Lists); // 读取数据
	PrimalHeuristic(Values, Lists); // 初始启发式，获得一组初始的切割方案，作为初始MP的系数矩阵
	ColumnGeneration(Values, Lists);

	return 0;
}