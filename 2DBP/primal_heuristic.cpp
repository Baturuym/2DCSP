
#include "2DBP.h"
using namespace std;

void PrimalHeuristic(All_Values& Values, All_Lists& Lists, Node& root_node) // 切断式切割启发式
{
	int item_types_num = Values.item_types_num;
	int strip_types_num = Values.strip_types_num;

	Values.Finish = false;
	Lists.occupied_items_list.clear();

	int stock_index = 0;
	int stock_pattern = 0;
	int strip_index = 0;
	int strip_pattern = 0;

#pragma region
	while (Values.Finish == false) { // 初始化新母板

		Lists.all_stocks_list.erase(Lists.all_stocks_list.begin()); // 母板池中除去母板0

		// Init one stock
		Stock_Stc new_stock;
		new_stock.length = Lists.all_stocks_list[0].length; // 
		new_stock.width = Lists.all_stocks_list[0].width; // 
		new_stock.area = new_stock.length * new_stock.width; // 
		new_stock.stock_idx = stock_index; // 
		new_stock.pos_x = 0; // 
		new_stock.pos_y = 0; // 

		// Init all strip_types in each stock
		for (int k = 0; k < strip_types_num; k++) {
			Strip_Type_Stc this_strip_type;
			this_strip_type.strip_type_idx = k + 1;
			new_stock.strip_types_list.push_back(this_strip_type);
		}

		// Init a new stock
		Item_Stc stock_remain;

		stock_remain.length = new_stock.length;
		stock_remain.width = new_stock.width;
		stock_remain.area = new_stock.area;
		stock_remain.pos_x = new_stock.pos_x;
		stock_remain.pos_y = new_stock.pos_y;

		stock_remain.stock_idx = new_stock.stock_idx;
		stock_remain.occupied = 0;

		int all_items_num = Lists.all_items_list.size();
		for (int j = 0; j < all_items_num; j++) {
			if (Lists.all_items_list[j].length <= stock_remain.length
				&& Lists.all_items_list[j].width <= stock_remain.width
				&& Lists.all_items_list[j].occupied == 0) { // 子板 j 能不能放入被切母板剩下的区域

				// Init first item in a this_strip, though may not use them all
				Item_Stc first_item;
				Lists.all_items_list[j].occupied = 1;

				first_item.item_type_idx = Lists.all_items_list[j].item_type_idx;
				first_item.item_idx = Lists.all_items_list[j].item_idx;
				first_item.demand = Lists.all_items_list[j].demand;

				first_item.length = Lists.all_items_list[j].length;
				first_item.width = Lists.all_items_list[j].width;
				first_item.area = first_item.length * first_item.width;
				first_item.pos_x = stock_remain.pos_x;
				first_item.pos_y = stock_remain.pos_y;

				first_item.stock_idx = stock_remain.stock_idx;
				first_item.strip_idx = strip_index;
				first_item.occupied = 1;

				Lists.occupied_items_list.push_back(first_item); // 确定的子板放入结果表

				// Init a this_strip acoording to its first item
				Strip_Stc new_strip;
				new_strip.strip_idx = strip_index;
				new_strip.strip_type_idx = first_item.item_type_idx;

				new_strip.length = stock_remain.length; // all this_strip's length == stock's length
				new_strip.width = first_item.width; // a strip_type is defined by its first item's width
				new_strip.area = new_strip.length * new_strip.width;
				new_strip.pos_x = first_item.pos_x;
				new_strip.pos_y = first_item.pos_y;

				new_strip.stock_idx = stock_remain.stock_idx;
				new_strip.items_in_strip_list.push_back(first_item); // a this_strip's first item defined this this_strip

				// Init all item_types in each this_strip, though may not use them all
				for (int k = 0; k < item_types_num; k++) {
					Item_Type_Stc this_item_type;
					this_item_type.item_type_idx = k + 1;
					new_strip.item_types_list.push_back(this_item_type);
				}

				int type_pos = first_item.item_type_idx - 1;
				new_strip.item_types_list[type_pos].this_item_type_num++;

				// 横向切断式切割后，第一个子板的右侧区域
				Item_Stc first_item_right_side;

				first_item_right_side.length = stock_remain.length - first_item.length; // 区域长度 = 母板长度 - 第一块子板长度
				first_item_right_side.width = first_item.width; // 区域宽度 = 第一块子板宽度
				first_item_right_side.area = first_item_right_side.length * first_item_right_side.width; // 区域面积
				first_item_right_side.pos_x = stock_remain.pos_x + first_item.length;
				first_item_right_side.pos_y = stock_remain.pos_y;

				first_item_right_side.stock_idx = stock_remain.stock_idx; // 区域所属母板
				first_item_right_side.item_type_idx = -1;
				first_item_right_side.occupied = 0;

				// 在第一个子板的右侧区域，即中间板内部继续填充子板
				// 直到当前中间板再也无法放入子板为止
				for (int m = 0; m < all_items_num; m++) {
					if (Lists.all_items_list[m].length <= first_item_right_side.length
						&& Lists.all_items_list[m].width <= first_item_right_side.width
						&& Lists.all_items_list[m].occupied == 0) { // 如果当前子板能够放入第一个子板右侧区域

						// 新的子板放入中间板
						Item_Stc new_item;
						Lists.all_items_list[m].occupied = 1;

						new_item.item_type_idx = Lists.all_items_list[m].item_type_idx; // 子板编号
						new_item.item_idx = Lists.all_items_list[m].item_idx; // 子板编号
						new_item.demand = Lists.all_items_list[m].demand;

						new_item.length = Lists.all_items_list[m].length;
						new_item.width = Lists.all_items_list[m].width;
						new_item.area = new_item.length * new_item.width;
						new_item.pos_x = first_item_right_side.pos_x;
						new_item.pos_y = first_item_right_side.pos_y;

						new_item.stock_idx = stock_remain.stock_idx; // 所属母板编号
						new_item.strip_idx = strip_index;
						new_item.occupied = 1;

						Lists.occupied_items_list.push_back(new_item); // 子板放入结果表
						new_strip.items_in_strip_list.push_back(new_item);

						int itm_pos = new_item.item_type_idx - 1;
						new_strip.item_types_list[itm_pos].this_item_type_num++;

						first_item_right_side.length = first_item_right_side.length - new_item.length; // 更新中间板的剩余长度
						first_item_right_side.pos_x = first_item_right_side.pos_x + new_item.length;
					}
				}
				// 跟已经出现的所有中间板.的每种子管.的使用数量比较一下

				int Strip_Final_Cnt = 0;
				int all_strips_num = Lists.all_strips_list.size();
				if (all_strips_num == 0) { // 第一个中间板
					new_strip.pattern = strip_pattern;
					strip_pattern++; // 只有唯一的切割模式，才对应一个pattern
					root_node.cutting_strip_patterns_list.push_back(new_strip);
				}
				if (all_strips_num != 0) { // 第一个中间板之后其他中间板
					for (int s = 0; s < all_strips_num; s++) {
						int cnt01 = 0;
						int cnt02 = 0;
						int cnt03 = 0;
						for (int k = 0; k < item_types_num; k++) { // 考虑所有子板种类 1-11
							int cnt1 = Lists.all_strips_list[s].item_types_list[k].this_item_type_num; // 已有中间板s中子板种类k+1的使用次数
							int cnt2 = new_strip.item_types_list[k].this_item_type_num; // 新中间板new_strip中子板种类k+1的使用次数
							if (cnt1 == cnt2) { // 二者使用次数相同
								cnt01 = cnt01 + 1; // 使用次数相同的情况+1
							}
							if (cnt1 != cnt2) { // 二者使用次数不同
								cnt03 = cnt03 + 1;
							}
						}
						Strip_Final_Cnt = cnt01;
						//printf("相同使用次数 = %d\n不同使用次数 = %d\n",cnt01,cnt03);
						if (Strip_Final_Cnt == item_types_num) { // 所有子板种类使用次数都相同
							new_strip.pattern = Lists.all_strips_list[s].pattern;
							break;
						}
					}
					// 遍历所有中间板中所有种类子管的使用次数后
					if (Strip_Final_Cnt < item_types_num) { // 确认是新的切割模式
						new_strip.pattern = strip_pattern;
						strip_pattern++; // 只有唯一的切割模式，才对应一个pattern
						root_node.cutting_strip_patterns_list.push_back(new_strip); // 第二阶段列
					}
				}

				strip_index++;
				Lists.all_strips_list.push_back(new_strip);
				new_stock.strips_list.push_back(new_strip);

				int stp_pos = new_strip.strip_type_idx - 1;
				new_stock.strip_types_list[stp_pos].this_strip_type_num++;

				// 横向切断式切割后，第一个子板的上方区域
				stock_remain.length = stock_remain.length;
				stock_remain.width = stock_remain.width - first_item.width;
				stock_remain.area = stock_remain.length * stock_remain.width;
				stock_remain.pos_x = stock_remain.pos_x;
				stock_remain.pos_y = stock_remain.pos_y + first_item.width;

				// 使用的子板总数
				int occupied_items_num = 0;
				int all_items_num = Lists.all_items_list.size();
				for (int k = 0; k < all_items_num; k++) {
					occupied_items_num += Lists.all_items_list[k].occupied;
				}
				if (occupied_items_num == all_items_num) { // 如果所有子板都被使用了
					Values.Finish = true;
				}
			}
		}

		//********计算母板总切割损耗成本********//
		int strip_total_cut_distance = 0;
		int strips_num_in_stock = new_stock.strips_list.size();
		for (int j = 0; j < strips_num_in_stock; j++) {

			Strip_Stc this_strip = new_stock.strips_list[j];
			int item_total_cut_distance = 0;
			int items_num_in_strip = this_strip.items_in_strip_list.size();
			for (int k = 0; k < items_num_in_strip; k++) {
				Item_Stc this_item = this_strip.items_in_strip_list[k];

				if (this_item.width < this_strip.width) {
					this_item.cutting_distance = this_item.length + this_item.width;
				}
				if (this_item.width == this_strip.width) {
					this_item.cutting_distance = this_item.width;
				}

				item_total_cut_distance = item_total_cut_distance + this_item.cutting_distance;
			}

			if (this_strip.pos_x + this_strip.width < new_stock.pos_x + new_stock.width) {
				this_strip.cutting_distance = item_total_cut_distance + this_strip.length;
				this_strip.material_cutting_loss = this_strip.cutting_distance * Values.unit_cut_loss;
			}
			if (this_strip.pos_x + this_strip.width == new_stock.pos_x + new_stock.width) {
				this_strip.cutting_distance = item_total_cut_distance;
				this_strip.material_cutting_loss = this_strip.cutting_distance * Values.unit_cut_loss;
			}
			strip_total_cut_distance = strip_total_cut_distance + this_strip.cutting_distance;
		}

		new_stock.cutting_distance = strip_total_cut_distance;
		new_stock.material_cutting_loss = new_stock.cutting_distance * Values.unit_cut_loss;

		//********计算母板总面积浪费成本********//
		int strip_total_waste_area = 0;
		for (int j = 0; j < strips_num_in_stock; j++) {
			Strip_Stc this_strip = new_stock.strips_list[j];
			int item_total_used_area = 0;
			int items_num_in_strip = this_strip.items_in_strip_list.size();
			for (int k = 0; k < items_num_in_strip; k++) {
				item_total_used_area = item_total_used_area + this_strip.items_in_strip_list[k].area;
			}

			this_strip.wasted_area = this_strip.area - item_total_used_area;
			this_strip.material_area_loss = this_strip.wasted_area * Values.unit_area_loss;
			strip_total_waste_area = strip_total_waste_area + this_strip.wasted_area;
		}

		new_stock.wasted_area = new_stock.area - strip_total_waste_area;
		new_stock.material_area_loss = new_stock.wasted_area * Values.unit_area_loss;

		int Stock_Final_Cnt = 0;
		int occupied_stocks_num = Lists.occupied_stocks_list.size();
		if (occupied_stocks_num == 0) { // 第一个母板
			new_stock.pattern = stock_pattern;
			stock_pattern++; // 只有唯一的切割模式，才对应一个pattern
			root_node.cutting_stock_patterns_list.push_back(new_stock);
		}

		if (occupied_stocks_num != 0) { // 第一个中间板之后其他母板
			for (int s = 0; s < occupied_stocks_num; s++) {
				int cnt01 = 0;
				int cnt02 = 0;
				int cnt03 = 0;
				for (int k = 0; k < strip_types_num; k++) { // 考虑所有中间板种类 1-11
					int cnt1 = Lists.occupied_stocks_list[s].strip_types_list[k].this_strip_type_num; // 已有母板s中中间板种类k的使用次数
					int cnt2 = new_stock.strip_types_list[k].this_strip_type_num; // 新母板new_stock中中间板种类k的使用次数

					if (cnt1 == cnt2) { // 二者使用次数相同
						cnt01 = cnt01 + 1; // 使用次数相同的情况+1
					}
					if (cnt1 != cnt2) { // 二者使用次数不同
						cnt03 = cnt03 + 1;
					}
				}

				Stock_Final_Cnt = cnt01;
				if (Stock_Final_Cnt == strip_types_num) { // 所有中间板种类使用次数都相同
					break;
				}
			}

			if (Stock_Final_Cnt < strip_types_num) { // 确认是新的中间板种类
				new_stock.pattern = stock_pattern;
				stock_pattern++; // 只有唯一的切割模式，才对应一个pattern
				root_node.cutting_stock_patterns_list.push_back(new_stock); // 第一阶段列
			}
		}

		Lists.occupied_stocks_list.push_back(new_stock);
		stock_index = stock_index + 1;
	}

	/*			    pattern columns
	-----------------------------------------
	|		 K_num			|		 P_num			|
	| cut-stk-ptn cols	| stp-cut-ptn cols	|
	-----------------------------------------------------
	|							|							|				|
	|			 C				|			D				|  J_num	|	strip_type cons >= 0
	|							|							|				|
	|----------------------------------------------------
	|							|							|				|
	|			 0				|			B				|  N_num	|	item_type cons >= item_type demands
	|							|							|				|
	-----------------------------------------------------
	*/

	int K_num = root_node.cutting_stock_patterns_list.size();
	int P_num = root_node.cutting_strip_patterns_list.size();

	int J_num = strip_types_num;
	int N_num = item_types_num;

	// Init model matrix
	for (int col = 0; col < K_num + P_num; col++) {
		vector<double>temp_col;
		for (int row = 0; row < J_num + N_num; row++) {

			// Matrix C & Matrix 0
			if (col < K_num) {

				// 1. Matrix C
				if (row < J_num) {
					double temp_val =
						root_node.cutting_stock_patterns_list[col].strip_types_list[row].this_strip_type_num; // 系数为中间板种类使用次数
					temp_col.push_back(temp_val);
				}

				// 2. Matrix 0
				if (row >= J_num) {
					double temp_val = 0; //
					temp_col.push_back(temp_val);
				}
			}

			// Matrix B & Matrix D
			if (col >= K_num) {

				// 3. Matrix D
				if (row < J_num) {
					int col_pos = col - K_num;
					int item_type_idx = row + 1;
					int strip_type_idx = root_node.cutting_strip_patterns_list[col_pos].strip_type_idx;
					if (strip_type_idx == item_type_idx) // 
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
				if (row >= J_num) {
					int col_pos = col - K_num;
					int row_pos = row - J_num;
					double temp_val =
						root_node.cutting_strip_patterns_list[col_pos].item_types_list[row_pos].this_item_type_num;
					temp_col.push_back(temp_val);
				}
			}
		}

		root_node.model_matrix.push_back(temp_col);
	}

	cout << endl;

	for (int col = 0; col < K_num; col++) {
		vector<double>temp_col;
		for (int row = 0; row < J_num + N_num; row++) {

			// 1. Matrix C
			if (row < J_num) {
				double temp_val =
					root_node.cutting_stock_patterns_list[col].strip_types_list[row].this_strip_type_num; // 系数为中间板种类使用次数
				temp_col.push_back(temp_val);
			}

			// 2. Matrix 0
			if (row >= J_num) {
				double temp_val = 0; //
				temp_col.push_back(temp_val);
			}
		}
		root_node.cutting_stock_cols.push_back(temp_col); // 第一阶段列
	}

	cout << endl;

	for (int col = K_num; col < K_num + P_num; col++) {
		vector<double>temp_col;
		for (int row = 0; row < J_num + N_num; row++) {

			// 3. Matrix D
			if (row < J_num) {
				int col_pos = col - K_num;
				int item_type_index = row + 1;
				int strip_type_index = root_node.cutting_strip_patterns_list[col_pos].strip_type_idx;
				if (strip_type_index == item_type_index) {
					double temp_val = -1; // 系数为-1
					temp_col.push_back(temp_val);
				}
				else { // 中间板种类和子板种类不对应
					double temp_val = 0; // 系数为0
					temp_col.push_back(temp_val);
				}
			}

			// 4.Matrix B
			if (row >= J_num) {
				int col_pos = col - K_num;
				int row_pos = row - J_num;
				double temp_val =
					root_node.cutting_strip_patterns_list[col_pos].item_types_list[row_pos].this_item_type_num;
				temp_col.push_back(temp_val);
			}
		}
		root_node.cutting_strip_cols.push_back(temp_col); // 第二阶段列
	}

	for (int k = 0; k < item_types_num; k++) {
		Strip_Type_Stc temp_stp;
		temp_stp.width = Lists.all_item_types_list[k].width;
		temp_stp.length = Values.stock_length;

		Lists.all_strip_types_list.push_back(temp_stp);
	}
	cout << endl;
}