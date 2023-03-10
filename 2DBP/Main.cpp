// 2023-03-10 BP for 2D-CSP
#include "2DBP.h"
using namespace std;

int main()
{
	All_Values Values;
	All_Lists Lists;
	ReadData(Values, Lists); // 读取数据
	CuttingHeuristic(Values, Lists); // 初始启发式，获得一组初始的切割方案，作为初始MP的系数矩阵



}