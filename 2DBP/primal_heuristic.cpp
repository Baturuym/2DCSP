// Yuming Zhao: https://github.com/Baturuym
// 2023-03-10 CG for 2D-CSP

#include "2DCG.h"
using namespace std;

void InitModelMatrix(All_Values& Values, All_Lists& Lists) // 切断式切割启发式
{
	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	Values.Finish = false;
	Lists.items_occupied_list.clear();

	int stock_index = 0;
	int stock_pattern = 0;
	int strip_index = 0;
	int strip_pattern = 0;

#pragma region
	while (Values.Finish == false)
	{
		// 初始化新母板
		Lists.stock_pool_list.erase(Lists.stock_pool_list.begin()); // 母板池中除去母板0

		StockProperties new_stock;
		new_stock.length = Lists.stock_pool_list[0].length; // 母板
		new_stock.width = Lists.stock_pool_list[0].width; // 母板
		new_stock.area = new_stock.length * new_stock.width; // 母板
		new_stock.index = stock_index; // 母板
		new_stock.x_value = 0; // 母板
		new_stock.y_value = 0; // 母板

		for (int k = 0; k < strip_types_num; k++)
		{
			StripTypeProperties this_strip_type;
			this_strip_type.type_index = k + 1;
			new_stock.strip_types_list.push_back(this_strip_type);
		}

		// 每换一张新母板，就初始化被母板剩余版
		ItemProperties stock_remain;
		stock_remain.length = new_stock.length;
		stock_remain.width = new_stock.width;
		stock_remain.type_index = -1;
		stock_remain.area = new_stock.area;
		stock_remain.stock_index = new_stock.index;
		stock_remain.x_value = new_stock.x_value;
		stock_remain.y_value = new_stock.y_value;
		stock_remain.occupied = 0;

		int all_items_list_size1 = Lists.all_items_list.size();
		for (int j = 0; j < all_items_list_size1; j++)
		{
			// 子板 j 能不能放入被切母板剩下的区域
			if (Lists.all_items_list[j].length <= stock_remain.length
				&& Lists.all_items_list[j].width <= stock_remain.width
				&& Lists.all_items_list[j].occupied == 0)
			{
				// 确定第一个子板
				ItemProperties first_item;
				first_item.type_index = Lists.all_items_list[j].type_index;
				first_item.index = Lists.all_items_list[j].index;
				first_item.demand = Lists.all_items_list[j].demand;
				first_item.length = Lists.all_items_list[j].length;
				first_item.width = Lists.all_items_list[j].width;
				first_item.area = first_item.length * first_item.width;
				first_item.stock_index = stock_remain.stock_index;
				first_item.Strip_index = strip_index;
				first_item.x_value = stock_remain.x_value;
				first_item.y_value = stock_remain.y_value;
				first_item.occupied = 1;
				Lists.all_items_list[j].occupied = 1;
				Lists.items_occupied_list.push_back(first_item); // 确定的子板放入结果表

				StripProperties new_strip;
				new_strip.index = strip_index;
				new_strip.type_index = first_item.type_index;
				new_strip.items_list.push_back(first_item); // 初始化第一个元素
				new_strip.length = stock_remain.length;
				new_strip.width = first_item.width;
				new_strip.area = new_strip.length * new_strip.width;
				new_strip.x_value = first_item.x_value;
				new_strip.y_value = first_item.y_value;
				new_strip.stock_index = stock_remain.stock_index;

				for (int k = 0; k < item_types_num; k++)
				{
					ItemTypeProperties this_item_type;
					this_item_type.type_index = k + 1;
					new_strip.item_types_list.push_back(this_item_type);
				}
				new_strip.item_types_list[first_item.type_index - 1].count++;

				// 横向切断式切割后，第一个子板的右侧区域
				ItemProperties first_item_right_side;
				first_item_right_side.length = stock_remain.length - first_item.length; // 区域长度 = 母板长度 - 第一块子板长度
				first_item_right_side.width = first_item.width; // 区域宽度 = 第一块子板宽度
				first_item_right_side.area = first_item_right_side.length * first_item_right_side.width; // 区域面积
				first_item_right_side.stock_index = stock_remain.stock_index; // 区域所属母板
				first_item_right_side.type_index = -1;
				first_item_right_side.x_value = stock_remain.x_value + first_item.length;
				first_item_right_side.y_value = stock_remain.y_value;
				first_item_right_side.occupied = 0;

				// 在第一个子板的右侧区域，即中间板内部继续填充子板
				// 直到当前中间板再也无法放入子板为止
				int all_items_list_size2 = Lists.all_items_list.size();
				for (int m = 0; m < all_items_list_size2; m++)
				{
					// 如果当前子板能够放入第一个子板右侧区域：
					if (Lists.all_items_list[m].length <= first_item_right_side.length
						&& Lists.all_items_list[m].width <= first_item_right_side.width
						&& Lists.all_items_list[m].occupied == 0)
					{
						// 新的子板放入中间板
						ItemProperties new_item;
						new_item.type_index = Lists.all_items_list[m].type_index; // 子板编号
						new_item.index = Lists.all_items_list[m].index; // 子板编号
						new_item.demand = Lists.all_items_list[m].demand;
						new_item.length = Lists.all_items_list[m].length;
						new_item.width = Lists.all_items_list[m].width;
						new_item.area = new_item.length * new_item.width;
						new_item.x_value = first_item_right_side.x_value;
						new_item.y_value = first_item_right_side.y_value;
						new_item.stock_index = stock_remain.stock_index; // 所属母板编号
						new_item.Strip_index = strip_index;
						new_item.occupied = 1;
						Lists.all_items_list[m].occupied = 1;

						Lists.items_occupied_list.push_back(new_item); // 子板放入结果表

						new_strip.items_list.push_back(new_item);
						new_strip.item_types_list[new_item.type_index - 1].count++;

						first_item_right_side.length = first_item_right_side.length - new_item.length; // 更新中间板的剩余长度
						first_item_right_side.x_value = first_item_right_side.x_value + new_item.length;
					}
				}
				// 跟已经出现的所有中间板.的每种子管.的使用数量比较一下

				int Strip_Final_Cnt = 0;
				if (Lists.all_strips_list.size() == 0) // 第一个中间板
				{
					new_strip.pattern = strip_pattern;
					strip_pattern++; // 只有唯一的切割模式，才对应一个pattern
					Lists.item_col_ptns_list.push_back(new_strip);
				}
				if (Lists.all_strips_list.size() != 0) // 第一个中间板之后其他中间板
				{
					int all_strips_list_size = Lists.all_strips_list.size();
					for (int s = 0; s < all_strips_list_size; s++)
					{
						//printf("新中间板%d与中间板%d做对比\n", new_strip.index,s);
						int cnt01 = 0;
						int cnt02 = 0;
						int cnt03 = 0;
						for (int k = 0; k < item_types_num; k++) // 考虑所有子板种类 1-11
						{
							int cnt1 = Lists.all_strips_list[s].item_types_list[k].count; // 已有中间板s中子板种类k+1的使用次数
							int cnt2 = new_strip.item_types_list[k].count; // 新中间板new_strip中子板种类k+1的使用次数

							if (cnt1 == cnt2) // 二者使用次数相同
							{
								cnt01 = cnt01 + 1; // 使用次数相同的情况+1
							}
							if (cnt1 != cnt2) // 二者使用次数不同
							{
								cnt03 = cnt03 + 1;
							}
						}
						Strip_Final_Cnt = cnt01;
						//printf("相同使用次数 = %d\n不同使用次数 = %d\n",cnt01,cnt03);
						if (Strip_Final_Cnt == item_types_num) // 所有子板种类使用次数都相同
						{
							new_strip.pattern = Lists.all_strips_list[s].pattern;
							break;
						}
					}
					// 遍历所有中间板中所有种类子管的使用次数后
					if (Strip_Final_Cnt < item_types_num) // 确认是新的切割模式
					{
						new_strip.pattern = strip_pattern;
						strip_pattern++; // 只有唯一的切割模式，才对应一个pattern
						Lists.item_col_ptns_list.push_back(new_strip); // 第二阶段列
					}
				}
				Lists.all_strips_list.push_back(new_strip);
				strip_index++;
				new_stock.strips_list.push_back(new_strip);
				new_stock.strip_types_list[new_strip.type_index - 1].count++;

				// 横向切断式切割后，第一个子板的上方区域
				stock_remain.length = stock_remain.length;
				stock_remain.width = stock_remain.width - first_item.width;
				stock_remain.area = stock_remain.length * stock_remain.width;
				stock_remain.x_value = stock_remain.x_value;
				stock_remain.y_value = stock_remain.y_value + first_item.width;

				// 使用的子板总数
				int Used_sum = 0;
				int all_items_list_size3 = Lists.all_items_list.size();
				for (int k = 0; k < all_items_list_size3; k++)
				{
					Used_sum += Lists.all_items_list[k].occupied;
				}
				// 如果所有子板都被使用了
				if (Used_sum == all_items_list_size3)
				{
					Values.Finish = true;
				}
			}
		}
		//********计算母板总切割损耗成本********//
		int strip_total_cut_distane = 0;
		int new_stock_strips_list_size1 = new_stock.strips_list.size();
		for (int j = 0; j < new_stock_strips_list_size1; j++)
		{
			StripProperties strip = new_stock.strips_list[j];
			int item_total_cut_distance = 0;
			int strip_items_list_size = strip.items_list.size();
			for (int k = 0; k < strip_items_list_size; k++)
			{
				ItemProperties item = strip.items_list[k];
				if (item.width < strip.width)
				{
					item.cutting_distance = item.length + item.width;
				}
				if (item.width == strip.width)
				{
					item.cutting_distance = item.width;
				}
				item_total_cut_distance = item_total_cut_distance + item.cutting_distance;
			}
			if (strip.x_value + strip.width < new_stock.x_value + new_stock.width)
			{
				strip.cutting_distance = item_total_cut_distance + strip.length;
				strip.material_cutting_loss = strip.cutting_distance * Values.unit_cut_loss;
			}
			if (strip.x_value + strip.width == new_stock.x_value + new_stock.width)
			{
				strip.cutting_distance = item_total_cut_distance;
				strip.material_cutting_loss = strip.cutting_distance * Values.unit_cut_loss;
			}
			strip_total_cut_distane = strip_total_cut_distane + strip.cutting_distance;
		}
		new_stock.cutting_distance = strip_total_cut_distane;
		new_stock.material_cutting_loss = new_stock.cutting_distance * Values.unit_cut_loss;

		//********计算母板总面积浪费成本********//
		int strip_total_waste_area = 0;
		int new_stock_strips_list_size2 = new_stock.strips_list.size();
		for (int j = 0; j < new_stock_strips_list_size2; j++)
		{
			StripProperties strip = new_stock.strips_list[j];
			int item_total_used_area = 0;
			int strip_items_list_size = strip.items_list.size();
			for (int k = 0; k < strip_items_list_size; k++)
			{
				item_total_used_area = item_total_used_area + strip.items_list[k].area;
			}
			strip.wasted_area = strip.area - item_total_used_area;
			strip.material_area_loss = strip.wasted_area * Values.unit_area_loss;
			strip_total_waste_area = strip_total_waste_area + strip.wasted_area;
		}
		new_stock.wasted_area = new_stock.area - strip_total_waste_area;
		new_stock.material_area_loss = new_stock.wasted_area * Values.unit_area_loss;

		int Stock_Final_Cnt = 0;
		if (Lists.occupied_stocks_list.size() == 0) // 第一个母板
		{
			new_stock.pattern = stock_pattern;
			stock_pattern++; // 只有唯一的切割模式，才对应一个pattern
			Lists.strip_col_ptns_list.push_back(new_stock);
		}

		if (Lists.occupied_stocks_list.size() != 0) // 第一个中间板之后其他母板
		{
			for (int s = 0; s < Lists.occupied_stocks_list.size(); s++)
			{
				int cnt01 = 0;
				int cnt02 = 0;
				int cnt03 = 0;

				for (int k = 0; k < strip_types_num; k++) // 考虑所有中间板种类 1-11
				{
					int cnt1 = Lists.occupied_stocks_list[s].strip_types_list[k].count; // 已有母板s中中间板种类k的使用次数
					int cnt2 = new_stock.strip_types_list[k].count; // 新母板new_stock中中间板种类k的使用次数

					if (cnt1 == cnt2) // 二者使用次数相同
					{
						cnt01 = cnt01 + 1; // 使用次数相同的情况+1
					}
					if (cnt1 != cnt2) // 二者使用次数不同
					{
						cnt03 = cnt03 + 1;
					}
				}

				Stock_Final_Cnt = cnt01;
				if (Stock_Final_Cnt == strip_types_num) // 所有中间板种类使用次数都相同
				{
					break;
				}
			}

			if (Stock_Final_Cnt < strip_types_num) // 确认是新的中间板种类
			{
				new_stock.pattern = stock_pattern;
				stock_pattern++; // 只有唯一的切割模式，才对应一个pattern
				Lists.strip_col_ptns_list.push_back(new_stock); // 第一阶段列
			}
		}
		Lists.occupied_stocks_list.push_back(new_stock);
		stock_index = stock_index + 1;
	}


	/*			pattern columns
	-----------------------------------------
	|		P_num			|		K_num			|
	| stk-cut-ptn cols	| stp-cut-tpn cols	|
	-----------------------------------------------------
	|							|							|				|
	|			 C				|			D				|  J_num	|	strip_type rows >= 0
	|							|							|				|
	|----------------------------------------------------
	|							|							|				|
	|			 0				|			B				|  N_num	|	item_type rows >= item_type demand
	|							|							|				|
	-----------------------------------------------------
	*/

	int P_num = Lists.strip_col_ptns_list.size();
	int K_num = Lists.item_col_ptns_list.size();

	int J_num = strip_types_num;
	int N_num = item_types_num;


	// Init model matrix
	for (int col = 0; col < P_num + K_num; col++)
	{
		vector<double>temp_col;
		for (int row = 0; row < J_num + N_num; row++)
		{
			// Matrix C & Matrix 0
			if (col < P_num)
			{
				// 1. Matrix C
				if (row < J_num)
				{
					double temp_val =
						Lists.strip_col_ptns_list[col].strip_types_list[row].count; // 系数为中间板种类使用次数
					temp_col.push_back(temp_val);
				}
				// 2. Matrix 0
				if (row >= J_num)
				{
					double temp_val = 0; //
					temp_col.push_back(temp_val);
				}
			}
			// Matrix B & Matrix D
			if (col >= P_num)
			{
				// 3. Matrix D
				if (row < J_num)
				{
					int col_pos = col - P_num;
					int item_type_idx = row + 1;
					if (Lists.item_col_ptns_list[col_pos].type_index == item_type_idx) // 
					{
						double temp_val = -1; // 系数为-1
						temp_col.push_back(temp_val);
					}
					else // 中间板种类和子板种类不对应
					{
						double temp_val = 0; // 系数为0
						temp_col.push_back(temp_val);
					}

				}
				// 4.Matrix B
				if(row>=J_num)
				{
					int col_pos = col - P_num;
					int row_pos = row - J_num;
					double temp_val = Lists.item_col_ptns_list[col_pos].item_types_list[row_pos].count;
					temp_col.push_back(temp_val);
				}
			}
		}
		Lists.model_matrix.push_back(temp_col);
	}

	cout << endl;

	for (int col = 0; col < P_num; col++)
	{
		vector<double>temp_col;
		for (int row = 0; row < J_num + N_num; row++)
		{
			// 1. Matrix C
			if (row < J_num)
			{
				double temp_val = Lists.strip_col_ptns_list[col].strip_types_list[row].count; // 系数为中间板种类使用次数
				temp_col.push_back(temp_val);
			}
			// 2. Matrix 0
			if (row >= J_num)
			{
				double temp_val = 0; //
				temp_col.push_back(temp_val);
			}
		}
		Lists.stock_cut_cols.push_back(temp_col); // 第一阶段列
	}

	cout << endl;

	for (int col = P_num; col < P_num+K_num; col++)
	{
		vector<double>temp_col;
		for (int row = 0; row < J_num + N_num; row++)
		{
			// 3. Matrix D
			if (row < J_num)
			{
				int col_pos = col - P_num;
				int item_type_idx = row + 1;
				if (Lists.item_col_ptns_list[col_pos].type_index == item_type_idx) // 
				{
					double temp_val = -1; // 系数为-1
					temp_col.push_back(temp_val);
				}
				else // 中间板种类和子板种类不对应
				{
					double temp_val = 0; // 系数为0
					temp_col.push_back(temp_val);
				}
			}
			// 4.Matrix B
			if (row >= J_num)
			{
				int col_pos = col - P_num;
				int row_pos = row - J_num;
				double temp_val = Lists.item_col_ptns_list[col_pos].item_types_list[row_pos].count;
				temp_col.push_back(temp_val);
			}
		}
		Lists.strip_cut_cols.push_back(temp_col); // 第二阶段列
	}

	for (int k = 0; k < item_types_num; k++)
	{
		StripTypeProperties temp_stp;
		temp_stp.width = Lists.item_types_list[k].width;
		temp_stp.length = Values.stock_length;
		Lists.strip_types_list.push_back(temp_stp);
	}
	cout << endl;
}