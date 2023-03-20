// Yuming Zhao: https://github.com/Baturuym
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

int main()
{
	All_Values Values;
	All_Lists Lists;

	ReadData(Values, Lists); // 读取数据

	InitModelMatrix(Values, Lists); // 初始启发式，获得一组初始的切割方案，作为初始MP的系数矩阵
	OutPutResults(Values, Lists);

	ColumnGeneration(Values, Lists);
	OutPutResults(Values, Lists);

	return 0;
}