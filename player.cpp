#include"communication.h"
#include<vector>
#include<iostream>
#include <queue>
#include<algorithm>

using std::cout;
using std::endl;
using std::priority_queue;
using std::vector;
using std::find;

extern State* state;
extern std::vector<State* > all_state;
extern int** ts19_map;
extern bool ts19_flag;
/*************************符号**************************/
enum map_identify {			//地图标识
	blank,
	my_building,
	en_building,
	road1,
	road2,
	road3,
	road4,
	road5,
	road6,
	road7,
	road8,
	legal_area,
	base
};

enum _atrributes {			//评估函数包含的属性
	_type,
	_attack,
	my_resource,
	_range,
	_hp,
	_pos,
	_special,
	_preference,
	_attribute
};
/*********************自定义变量声明*********************/
const int all_type = 28;									//总共27种单位与建筑加上敌方Age
const int all_buildings_num = Building_Type;				//所有建筑数量
const int all_unit_num = Soldier_Type;						//所有单位数量
const int max_command_num = 50;								//单回合最多打指令数
															//常量

int ins_num = 50;											//每回合自动刷新，留存的指令数
int update_resource = 0;									//保留给升级的资源
int road_num = 0;											//路的数目
int lim_building_num;										//建筑数目上限，初始和升级时维护 
int lim_building_force;										//建造力上限 ，初始和升级时维护 
Position my_base_pos, enemy_base_pos;
int map[MAP_SIZE][MAP_SIZE] = { 0 };						//这个括号内得有0，不然有bug
int map2[MAP_SIZE][MAP_SIZE] = { 0 };
 vector<vector<float>> mysoldier_heal ;           //每条路上的我方血量
vector<vector<float>> mysoldier_heal_last ;           //每条路上的我方血量_上回合
vector<vector<int>> ensoldier_num;			//每条路上的敌方兵力
vector<vector<int>> enbuilding_num ;           //每条路附近的敌方建筑 


/***********************参数表************************/
const float utility_weight1[Soldier_Type][_attribute + 1] = {
	//单位				攻击		资源		攻击距离		生命		位置		特殊加成		偏好
	{ BIT_STREAM,		0.4,	0,		0.05,		0.4,	0,		0,			1 },
{ VOLTAGE_SOURCE,	0.35,	0,		0.05,		0.45,	0,		0,			1 },
{ CURRENT_SOURCE,	0.4,	0,		0,			0.4,	0.5,	0,			1 },
{ ENIAC,			0.4,	0,		0.05,		0.4,	0,		0,			1 },
{ PACKET,			0.4,	0,		0,			0.4,	0.5,	0,			1 },
{ OPTICAL_FIBER,	0.35,	0,		0.05,		0.45,	0,		0,			1 },
{ TURING_MACHINE,	0.4,	0,		0.05,		0.4,	0,		0,			1 },
{ ULTRON,			0.4,	0,		0,			0.4,	0.5,	0,			1 }
};

const float utility_weight2[Building_Type][_attribute + 1] = {
	//建筑				攻击		资源		攻击距离		生命		位置		特殊加成		偏好
	{ __Base,			0,		0,		0,			0,		0,		0,			0 },
{ Shannon,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Thevenin,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Norton,			0.1,	0,		0,			0.4,	0.3,	0,			1 },
{ Von_Neumann,		0.1,	0,		0,			0.4,	0,		0,			1 },
{ Berners_Lee,		0.1,	0,		0,			0.4,	0.33,	0,			1 },
{ Kuen_Kao,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Turing,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Tony_Stark,		0.1,	0,		0,			0.4,	0.4,	0,			1 },
{ Bool,				0.2,	0,		0.05,		0.5,	0,		0,			1 },
{ Ohm,				0.4,	0,		0.05,		0.5,	0,		4,			1 },//可以发动特效时，加成特殊*加成函数
{ Mole,				0.3,	0,		0.07,		0.5,	0,		2,			1 },
{ Monte_Carlo,		0.3,	0,		0.05,		0.5,	0,		0,			1 },
{ Larry_Roberts,	0.4,	0,		0.05,		0.5,	0,		3,			1 },
{ Robert_Kahn,		0.4,	0,		0.05,		0.5,	0,		1,			1 },
{ Musk,				0,		0,		0.2,		0.8,	0,		20,			1 },
{ Hawkin,			0,		0,		0.3,		0.8,	0,		30,			1 },
{ Programmer,		0,		1,		0,			0,		0.1,	0,			1 }
};

const float restrain_table[Building_Type][Building_Type] = {   //建筑相克表，未修改 
															   //建筑			__Base	Shannon	Thevenin    Norton	Von_Neumann		Berners_Lee	Kuen_Kao Turing  Tony_Stark  Bool  Ohm Mole Monte_Carlo  Larry_Roberts Robert_Kahn 	Musk Hawkin Programmer	
	{ __Base,			0,		0,		0,			0,		0,		0,			0 },
{ Shannon,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Thevenin,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Norton,			0.1,	0,		0,			0.4,	0.3,	0,			1 },
{ Von_Neumann,		0.1,	0,		0,			0.4,	0,		0,			1 },
{ Berners_Lee,		0.1,	0,		0,			0.4,	0.33,	0,			1 },
{ Kuen_Kao,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Turing,			0.1,	0,		0,			0.4,	0,		0,			1 },
{ Tony_Stark,		0.1,	0,		0,			0.4,	0.4,	0,			1 },
{ Bool,				0.2,	0,		0.05,		0.5,	0,		0,			1 },
{ Ohm,				0.4,	0,		0.05,		0.5,	0,		4,			1 },
{ Mole,				0.3,	0,		0.07,		0.5,	0,		2,			1 },
{ Monte_Carlo,		0.3,	0,		0.05,		0.5,	0,		0,			1 },
{ Larry_Roberts,	0.4,	0,		0.05,		0.5,	0,		3,			1 },
{ Robert_Kahn,		0.4,	0,		0.05,		0.5,	0,		1,			1 },
{ Musk,				0,		0,		0.2,		0.8,	0,		20,			1 },
{ Hawkin,			0,		0,		0.3,		0.8,	0,		30,			1 },
{ Programmer,		0,		1,		0,			0,		0.1,	0,			1 }
};

/**************************各种辅助函数**********************/
int abs(int x, int y) { return x > y ? x - y : y - x; }
int dist(Position a, Position b) { return abs(a.x, b.x) + abs(a.y, b.y); }
bool outofRange(int x, int y) {
	if (x < 0 || x >= 200 || y < 0 || y >= 200)
		return true;
	else
		return false;
}
/*bool construct_legal(Position pos) {
}*/
float calculate_special(BuildingType b, int flag) {
	return 0;
}
float calculate_attack(Soldier a) {
	return (1 + 0.5*a.level) * OriginalSoldierAttribute[a.soldier_name][SOLDIER_ORIGINAL_ATTACK];
}
float calculate_attackRange(Soldier a) {
	return (1 + 0.5*a.level) * OriginalSoldierAttribute[a.soldier_name][ATTACK_RANGE];
}
float calculate_attackRange(BuildingType b, int level) {
	return (1 + 0.5*level) * OriginalBuildingAttribute[b][ORIGINAL_RANGE];
}
float calculate_attack(Building b) {
	return (1 + 0.5*b.level)*OriginalBuildingAttribute[b.building_type][ORIGINAL_ATTACK];
}
float calculate_hp(Building b) {
	return (1 + 0.5*b.level) * OriginalBuildingAttribute[b.building_type][ORIGINAL_HEAL];
}
float calculate_hp(Soldier a, int level) {//提供等级，用来计算为建造的兵的生命
	return (1 + 0.5*level)* OriginalSoldierAttribute[a.soldier_name][SOLDIER_ORIGINAL_HEAL];
}
float calculate_attackRange(Building b) {
	return (1 + 0.5*b.level) * OriginalBuildingAttribute[b.building_type][ORIGINAL_RANGE];
}
float calculate_CD(Building b) {
	return (1 + 0.5*b.level) * OriginalBuildingAttribute[b.building_type][ORIGINAL_RANGE];
}
int calculate_resource_cost(BuildingType b, int level) {
	return  OriginalBuildingAttribute[b][ORIGINAL_RESOURCE];
}
int calculate_building_force(BuildingType b, int level) {
	return  OriginalBuildingAttribute[b][ORIGINAL_BUILDING_POINT];
}
float calculate_utility(Soldier a) {   //计算一个已经存在的兵的效用 
	                                                                                                                       
	int distance = (a.flag == ts19_flag) ? dist(a.pos, enemy_base_pos) : dist(a.pos, my_base_pos);
	return	a.heal * utility_weight1[a.soldier_name][_hp] +
		distance * utility_weight1[a.soldier_name][_pos] +
		calculate_attack(a) * utility_weight1[a.soldier_name][_attack] +
		calculate_attackRange(a) * utility_weight1[a.soldier_name][_range];

}
float calculate_utility(Building b) {	//	对所有建筑都适用
	if (b.building_type == Programmer) return calculate_attack(b);
	else if (b.building_type == Base) return 0;
	int distance = (b.flag == ts19_flag) ? dist(b.pos, enemy_base_pos) : dist(b.pos, my_base_pos);
	return b.heal * utility_weight2[b.building_type][_hp] +
		calculate_attack(b) * utility_weight2[b.building_type][_attack] +
		calculate_attackRange(b.building_type, b.level) * utility_weight2[b.building_type][_range] +
		calculate_special(b.building_type, b.flag) * utility_weight2[b.building_type][_special];
}
float calculate_utility(BuildingType b, int flag) {//计算所有己方b类建筑的总效用 
	float u = 0;
	for (auto iter = state->building[flag].cbegin(); iter != state->building[flag].cend(); iter++) {
		if (iter->building_type == b)
			u += calculate_utility(*iter);
	}
	return u;
}
float calculate_utility_change(Building b) {//返回升级的效用增量
	return 0.5 * (
		b.heal / calculate_hp(b) * utility_weight2[b.building_type][_hp] * OriginalBuildingAttribute[b.building_type][ORIGINAL_HEAL] +
		OriginalBuildingAttribute[b.building_type][ORIGINAL_ATTACK] * utility_weight2[b.building_type][_attack] +
		OriginalBuildingAttribute[b.building_type][ORIGINAL_RANGE] * utility_weight2[b.building_type][_range]);

}
float calculate_upgradeB_Rcost(Building b) {//升级建筑的资源消耗
	return 0.5 * OriginalBuildingAttribute[b.building_type][ORIGINAL_RESOURCE];
}
float calculate_upgradeB_Fcost(Building b) {//升级建筑的建造点消耗
	return 0.5 * OriginalBuildingAttribute[b.building_type][ORIGINAL_BUILDING_POINT];
}

/********************类的定义和函数实现******************/

void tranverse_r(Position pos, int r, void(*p)(Position, int, Position)) {//遍历pos周围汉米敦距离r的点，并做操作
	if (r <= 0) return;
	int upBound = 200;
	int x = pos.x - r;
	int y = pos.y;
	//cout << "xy:" << x << " " << y << endl;
	while (x < pos.x) {
		//cout << "while1in" << endl;
		if (!outofRange(x, y))
		{
			//cout << "while1" << endl;
			p(Position(x,y), r, pos);
		}
		x++;
		y++;
	}
	while (y > pos.y) {
		if (!outofRange(x, y))
		 {
			p(Position(x, y), r, pos);
		}
		x++;
		y--;
	}
	while (x > pos.x) {
		if (!outofRange(x, y))
		 {
			p(Position(x, y), r, pos);
		}
		x--;
		y--;
	}
	while (y < pos.y) {
		if (!outofRange(x, y))
		 {
			p(Position(x, y), r, pos);
		}
		x--;
		y++;
	}

}
void tranverse_r(Position pos, int rmin, int rmax, void(*p)(Position, int, Position)) {
	while (rmin <= rmax) {
		tranverse_r(pos, rmin, p);
		rmin++;
	}
}
void tranverse_r(Position pos, int r, vector<Position> s_pos) {	//帮助找该点最近的路的位置
	int roadNO = map2[pos.x][pos.y] / 100;
	if (r <= 0) return;
	int upBound = 200;
	int x = pos.x - r;
	int y = pos.y;
	while (x <= pos.x) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[pos.x][pos.y] == roadNO) {
				s_pos.push_back(Position(x, y));
				return;
			}
		}
		x++;
		y++;
	}
	while (y >= pos.y) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[pos.x][pos.y] == roadNO) {
				s_pos.push_back(Position(x, y));
				return;
			}
		}
		x++;
		y--;
	}
	while (x >= pos.x) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[pos.x][pos.y] == roadNO) {
				s_pos.push_back(Position(x, y));
				return;
			}
		}
		x--;
		y--;
	}
	while (y <= pos.y) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[pos.x][pos.y] == roadNO) {
				s_pos.push_back(Position(x, y));
				return;
			}
		}
		x--;
		y++;
	}
}
void tranverse_pro(Position pos, int r, vector<Position> poses) {//给码农用的
	if (r <= 0) return;
	int upBound = 200;
	int x = pos.x - r;
	int y = pos.y;
	while (x <= pos.x) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[x][y] == legal_area)
				poses.push_back(Position(x, y));
		}
		x++;
		y++;
	}
	while (y >= pos.y) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[x][y] == legal_area)
				poses.push_back(Position(x, y));
		}
		x++;
		y--;
	}
	while (x >= pos.x) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[x][y] == legal_area)
				poses.push_back(Position(x, y));
		}
		x--;
		y--;
	}
	while (y <= pos.y) {
		if (outofRange(x, y))
			continue;
		else {
			if (map[x][y] == legal_area)
				poses.push_back(Position(x, y));
		}
		x--;
		y++;
	}

}
																																																									//int s = 0;
void paint_map2(Position pos, int r, Position center) { //可能没有8条路 
	if (map[pos.x][pos.y] <= road8 && map[pos.x][pos.y] >= road1) {
		map2[center.x][center.y] = r + 100 * (map[pos.x][pos.y] - road1+1);
		
	}
   // cout << "callpaint" << endl;
}
void paint_maplegal(Position pos, int r, Position center) {
	if (ts19_map[pos.x][pos.y] != 1
		&& ts19_map[pos.x][pos.y] != 2
		&& map[pos.x][pos.y] != en_building
		&& map[pos.x][pos.y] != my_building)
		map[pos.x][pos.y] = legal_area;
}
void paint_mapblank(Position pos, int r, Position center) {
	if (ts19_map[pos.x][pos.y] != 1
		&& ts19_map[pos.x][pos.y] != 2
		&& map[pos.x][pos.y] != en_building
		&& map[pos.x][pos.y] != my_building)
		map[pos.x][pos.y] = blank;
}
void init_map() {		//初始化地图并将路进行标号 ,第0回合不需要对建筑标记吗？ 
	if (state->turn == 0) {    //如果路不往回拐的话，似乎可以简单地解决，如果往回拐我就不会了 
		my_base_pos = state->building[ts19_flag][0].pos;
		enemy_base_pos = state->building[1 - ts19_flag][0].pos;
		//先处理和左边主基地不在同一行的路 
		for (int i = 7; i < MAP_SIZE ; i++) {      
			int road_count = 0;
			for (int j = 0; j < MAP_SIZE; j++) {
				switch (ts19_map[i][j])
				{
				case 1: {
					map[i][j] = road1 + road_count;
					if (j + 1 != MAP_SIZE && ts19_map[i][j + 1] != 1)  //考虑到同一条路可能横在同一行中的情况 
						road_count++;
				}break;
				case 2: {
					map[i][j] = base;
				}break;
				case 0: {
					map[i][j] = blank;
				}break;
				default:
					break;
				}
			}
		}
		//再处理和左边主基地在同一行的情况 
		{
			int road_left = 0;  
			int i = 7;
			for (int j = 0; j <= 7; j++) if (ts19_map[i][j] == 1) road_left += 1;
			for (int i = 6; i >=0; i--) {
				int road_count = road_left;
				for (int j = 0; j <= MAP_SIZE - 1; j++) {
					switch (ts19_map[i][j])
					{
					case 1: {
						map[i][j] = road1 + road_count;
						
						if (j + 1 != MAP_SIZE && ts19_map[i][j + 1] != 1)  //考虑到同一条路可能横在同一行中的情况 
							road_count++;
					}break;
					case 2: {
						map[i][j] = base;
					}break;
					case 0: {
						map[i][j] = blank;
					}break;
					default:
						break;
					}
				}
				if (ts19_map[i][7] == 1)  road_left++;
			}
		}

	} 
	cout << "map is OK" << endl;
	//以下为map2的初始化   
	
	if (state->turn == 0){
																																		int point = 0; int p = 0;
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			map2[i][j] = -1;
			//  cout << "h";
			if (ts19_map[i][j] == 1) {
				map2[i][j] = 0 + 100 * (map[i][j] - road1 + 1);
				//cout << "ij" << i << " " << j << endl;
				continue;
			}
			if (map[i][j] == blank) {
				p++;
				for (int k = 1; k <= 50; k++) {
					//cout << "k:" << k<<endl;
					tranverse_r(Position(i, j), k, paint_map2);
					if (map2[i][j] != -1) {
						point++;
						break;
					}
				}
			}
		}
		}
	cout << "p" << p << endl;
	cout << "point" << point;
	cout << "map2 is OK" << endl;
	}
	
}
void refresh_unit() {		//将敌方unit统计表清零，并重新统计,实际上还没用到
	if (state->turn == 0) {
		ensoldier_num.resize(road_num);
		for (int i = 0; i < road_num; i++)
			ensoldier_num[i].resize(all_unit_num);

		mysoldier_heal.resize(road_num);
		for (int i = 0; i < road_num; i++)
			mysoldier_heal[i].resize(all_unit_num);

		mysoldier_heal_last.resize(road_num);
		for (int i = 0; i < road_num; i++)
			mysoldier_heal_last[i].resize(all_unit_num);
	}

	for (int i = 0; i < road_num; i++)
		for (int j = 0; j <all_unit_num; j++) {
			ensoldier_num[i][j] = 0;
		}
	vector<Soldier>& soldier_set1 = state->soldier[1 - ts19_flag];
	for (auto iter = soldier_set1.cbegin(); iter != soldier_set1.cend(); iter++) {
		ensoldier_num[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] ++;
	}
	//将我方unit统计表清零，并重新统计
	for (int i = 0; i < road_num; i++)
		for (int j = 0; j < all_unit_num; j++) {
			mysoldier_heal[i][j] = 0;
		}
	vector<Soldier>& soldier_set2 = state->soldier[ts19_flag];
	for (auto iter = soldier_set2.cbegin(); iter != soldier_set2.cend(); iter++) {
		mysoldier_heal[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] += iter->heal;
	}

	for (int i = 0; i < road_num; i++)   //将我方上回合unit血量统计表清零并重新统计 
		for (int j = 0; j <all_unit_num; j++) {
			mysoldier_heal_last[i][j] = 0;
		}
	vector<Soldier>& soldier_set3 = all_state.back()->soldier[ts19_flag];
	for (auto iter = soldier_set3.cbegin(); iter != soldier_set3.cend(); iter++) {
		mysoldier_heal_last[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] += iter->heal;
	}
}
void refresh_map() {	//更新地图
	if (state->turn >= 1) {   //直接全部设blank? 
		vector<Building>* building_set = state->building;   
		vector<Building>* building_set_last = all_state.back()->building;
		/*for (int index = 0; index <= 1; index++) {
		for (auto iter = building_set_last[index].cbegin();
		iter != building_set_last[index].cend(); iter++) {
		bool can_find = false;
		for (auto iter2 = building_set[index].begin();
		iter2 != building_set[index].end(); iter2++) {
		if (iter->unit_id == iter2->unit_id) {
		can_find = true;
		}
		if (can_find) break;
		}
		if (!can_find) map[iter->pos.x][iter->pos.y] = blank;
		}
		}*/
		for(int i=0; i<MAP_SIZE; i++)
			for (int j = 0; j < MAP_SIZE; j++)
			{
				if (map[i][j] == en_building || map[i][j] == my_building || map[i][j] == legal_area)
					map[i][j] == blank;
			}
		for (auto iter = building_set_last[1 - ts19_flag].cbegin();
			iter != building_set_last[1 - ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = en_building;
		}
		for (auto iter = building_set[ts19_flag].cbegin();
			iter != building_set[ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = my_building;

		}
		my_base_pos = state->building[ts19_flag][0].pos;               //将基地周围设为可建造区域
		if (my_base_pos.x == 6 && my_base_pos.y == 6) {
			for (int i = 7; i < 7 + 8; i++)
				for (int j = 0; j < 7; j++)
					paint_maplegal(Position(i, j), 0,Position(0,0));       //在这里后两个参数没有什么用
			for (int i = 0; i < 7; i++)
				for (int j = 7; j < 7 + 8; j++)
					paint_maplegal(Position(i, j), 0,Position(0,0));
		}
		else {
			for (int i = 193; i < 200; i++)
				for (int j = 193 - 8; j < 193; j++)
					paint_maplegal(Position(i, j), 0,Position(0,0));
			for (int i = 193 - 8; i < 193; i++)
				for (int j = 193; j < 200; j++)
					paint_maplegal(Position(i, j), 0,Position(0,0));
		}
		for (auto iter = building_set[ts19_flag].cbegin();
			iter != building_set[ts19_flag].cend();
			iter++) {
			tranverse_r(iter->pos,1,8, paint_maplegal);                 //在自己建筑周围第一轮遍历，将8以内设为可建造区域 	   
		}
		for (auto iter = building_set[ts19_flag].cbegin();
			iter != building_set[ts19_flag].cend();
			iter++) {
			tranverse_r(iter->pos, 1, 1, paint_mapblank);                 //在自己建筑周围第二轮遍历，将2以内设为不可建造区域 

		}
	}
}
void refresh() {
	ins_num = 50;
	update_resource += 0.1 * state->resource[ts19_flag].resource;
	state->resource[ts19_flag].resource *= 0.9;
	//每回合自动将资源0.1保留给update_resource
	init_map();
	refresh_map();
	refresh_unit();
}


/**************************行为树部分****************/

class Node
{
public:
	//函数成员 
	Node(){}
	~Node();
	void tick(int f_power, int f_resource, int f_utility);   //参数为父节点分配到的建造力、资源; 兄弟节点总效用 T
	virtual int assess() {
		std::cout << "assess" << endl;
		return 0;
	}
	virtual void execute() {
		std::cout << "execute" << endl;
	}

	//数据成员 
	std::vector<Node*> children;
	int utility;     //自己的效用值，在调用assess函数后更新 
	int max_power;   //分配到的建造力 
	int max_resource;   //分配到的资源 

};

/*******************************各种子类*******************************/
class _UpgradeAGE :public Node			//升级时代，每回合预留最低权重的资源，若可升级，就升级
{
public:
	_UpgradeAGE(){}
	int assess();
	void execute();
};
class _Resource :public Node			//获取资源，一般不需要维修，但过时建筑需要清理或者升级，每回合资源获取量要定一个函数来判断
{										//考虑敌方资源获取量
public:
	_Resource(){}
	int assess();

};
class _Development :public Node			//建造 进攻或防守 
										//计算攻击和防御需要达到的效用值，再加上原有的攻击防御偏好，分配每个行为的值
{
public:
	_Development(){}
	int assess();
};

class _Attack :public Node				//根据权值表来，附带考虑对方的防御建筑效用表，进行对应策略
{
public:
	_Attack(){}
	int assess();
};
class _Defend :public Node				//根据每条路每个单位的威胁值进行分配，按照需要程度分配，某些单位有对应克制单位，这些单位加大投入，
										//其余就按照权值表来
{
public:
	_Defend(){}
	int assess();
	float table[8] = {0};				//防御建筑权值表
	//void fresh_table();					//防御建筑权值更新，不能建的建筑权值为0，特殊克制建筑权值上升，其余简直按效用值排序，总和归一化
};
class _Programmar :public Node			//码农的维修，升级，建造，单独有个效用函数，根据每回合获得的需要效用值进行
{										//感觉码农不需要考虑太复杂，建造时生产到远一点的位置就行，基本不需要考虑维修
public:
	_Programmar(){}
	int assess();
	void execute();
	void sell_programmer();				//卖掉码农
	vector<Position> find_best_place(int n);	//在基地周围建码农
};
class _Sell :public Node				//通过卖房子获取资源
{
public:
	_Sell(){}
	int assess();
	void execute();
	vector<Building> sell_list();		//若效用小于某一个阈值（可以定为平均值75%），则淘汰，有数量上限
										//顺便再考虑当建筑数量较多时，出售效用较低的一批建筑
};

class _BuildingNode :public Node		//代表行为树中建筑节点部分
{										//决策说明：假设有评估效用的函数，该节点需要达到某效用值，
										//此外还有成本限制数
										//遍历全部建筑得到 每个建筑，每种操作(f,c)
										//目标：选取某建筑下的某操作，使得sum(ci)最小且在成本预算内，达到效用最大化；
										//可以先写个简化版

public:
	_BuildingNode(){}
	int assess();
	//void execute();
	//void attribute();				//决策如何分配建筑，按整数分配到下面的节点，由成本决定

	BuildingType buildingtype;
	_BuildingNode(BuildingType b) :buildingtype(b) {}

};

class _Construct :public Node
{
public:
	_Construct(){}
	int assess();
	void execute();
	BuildingType buildingtype;
	_Construct(BuildingType b) :buildingtype(b) {}

	vector<Position> find_soldier_pos(vector<Position>& b_pos);	//根据建筑位置列表，选择最佳的生产地点
	vector<Position> find_best_place();			//寻找到最佳的位置来建造
private:
	int construct_num = 0;
};

class _Maintain :public Node
{
public:
	_Maintain(){}
	int assess();
	void execute();
	BuildingType buildingtype;
	_Maintain(BuildingType b) :buildingtype(b) {}

	void fresh_num(int num) { maintain_num = (num >= 0) ? num : 0; }
	vector<int> min_cost();	//返回维修费用最少的，限定数量的建筑，切记每回合维修最多20%的血量
private:
	int maintain_num = 0;
};

class _Upgrade :public Node
{
public:
	_Upgrade(){}
	int assess();
	void execute();
	BuildingType buildingtype;
	_Upgrade(BuildingType b) :buildingtype(b) {}

	void fresh_num(int num) { upgrade_num = (num >= 0) ? num : 0; }
	vector<int> min_cost();		//返回升级费用最小且最优的建筑id
private:
	int upgrade_num = 0;
};
/***************************子节点声明结束*************************/


void Node::tick(int f_power, int f_resource, int f_utility)
{
	//参数为父节点分配到的建造力、资源; 兄弟节点总效用 
	max_power = f_power * utility / f_utility;
	max_resource = f_resource * utility / f_utility;  // 应该有更好的分配方案 
	if (this->children.size() != 0)
	{
		int all_utility = 0;
		for (int i = 0; i < this->children.size(); i++) {
			all_utility += children[i]->assess();    //调用assess的时候需要计算并保存节点的utility 
		}

		/*for (int i = 0; i< this->children.size(); i++) {
			children[i]->tick(max_power, max_resource, all_utility);
		}*/
	}
	else
		execute();
}



class Tree
{
public:
	Node * root;
	Tree();	     //建树，在init中调用 
	//void evaluate();		//评估，并对第一层做出分配
	//void init_map();		//初始化地图
	void tranverse();
private:
	//参数表
	float threaten_buliding[all_buildings_num] = {};				//敌方各种类型单位的威胁值
																	//float threaten_soldier[road_num][all_unit_num] = {};					//每条路上每种兵的威胁
																	//float weight[all_operation] = {	1, 1, 0.1, 0.1, 0.9 };			//己方各种操作权重
	float building_weight[Building_Type] = {};						//各种建筑的权重，建筑标识参照api文件
	float soldier_weight[Soldier_Type] =
	{};															//对各种士兵的基础权重
	float evaluate_table[3] = {};									//对三个节点做出评估
																	//每回合需更新的信息
																	
																	
};

Tree::Tree()
{
	root = new Node();
	
	//第一层 
	root->children.push_back(new _UpgradeAGE());
	root->children.push_back(new _Development());
	root->children.push_back(new _Resource());

	//第二层
	root->children[1]->children.push_back(new _Attack());
	root->children[1]->children.push_back(new _Defend());
	root->children[2]->children.push_back(new _Programmar());
	root->children[2]->children.push_back(new _Sell());

	//第三层 第一行为attack的孩子，第二行为defend的孩子，决定建筑种类 
	for (int i = 1; i < 8; i++)    root->children[1]->children[0]->children.push_back(new _BuildingNode(BuildingType(i)));
	for (int i = 9; i < 16; i++)   root->children[1]->children[1]->children.push_back(new _BuildingNode(BuildingType(i)));

	//第四层  前三行为进攻建筑，后三行为防守建筑，决定是升级、维修还是建造 
	for (int i = 0; i < 7; i++)
		for (int j = 0; j <= 1; j++) {
			vector<Node*> temp = root->children[1]->children[j]->children[i]->children;
			temp.push_back(new _Construct(BuildingType(i)));
			temp.push_back(new _Maintain(BuildingType(i)));
			temp.push_back(new _Upgrade(BuildingType(i)));

		}

};

void Tree::tranverse() {
	refresh();
	//evaluate();
	/*for (int i = 0; i <= 2; i++) {
	this->root->children[i]->execute();
	}*/
	root->utility = 1;
	root->tick(60 + 40 * state->age[ts19_flag], state->resource[ts19_flag].resource, root->utility);
}
void f_player()
{	//init 函数，测试，之后的路有问题

	if (state->turn == 0) {
		my_base_pos = state->building[ts19_flag][0].pos;
		enemy_base_pos = state->building[1 - ts19_flag][0].pos;

		for (int i = 6; i >= 0; i--) {
			if (ts19_map[7][i] == 1)
				road_num++;
			if (ts19_map[i][7] == 1)
				road_num++;
		}
		road_num += (ts19_map[7][7] == 1) ? 1 : 0;
		cout << "road_num:" << road_num << endl;
		Tree t;
		t.tranverse();
		                                                                                                                          
		for (int i = 0; i < 150; i++)
		{
			cout << endl;
			for (int j = 0; j < 150; j++) {
				if (map[i][j] == 12) cout << "b";
				else if (map[i][j] == blank) cout << " ";
				else
					cout << map[i][j] - 2;
			}
		}
		cout << endl;

		for (int i = 0; i < 150; i++)
		{
			cout << endl;
			for (int j = 0; j < 150; j++) {
				if (map2[i][j] == -1)
					cout << " ";
				else
					cout << map2[i][j] / 100;
			}
		}
	}



}

/*************************************子节点方法***************************/
int _UpgradeAGE::assess() {
	int backward = state->age[1 - ts19_flag] > state->age[ts19_flag] ? 10 : 1;  //等级落后于敌人时大力追赶，需要调节 
	int en_re = state->resource[1 - ts19_flag].resource;
	this->utility = en_re + backward;
	return utility;
}

int _Development::assess() {
	
	float s_utility = 0, en_s_utility = 0;
	float b_utility = 0, en_b_utility = 0;
	//计算所有的效用并叠加，直接比较差值
	int basic_develop = 200 * state->age[ts19_flag];
	for (int i = 0; i <= state->building[ts19_flag].size() - 1; i++) {
		if (state->building[ts19_flag][i].building_type != Programmer &&
			state->building[ts19_flag][i].building_type != Base) {
			b_utility += calculate_utility(state->building[ts19_flag][i]);
		}
	}
	for (int i = 0; i <= state->building[1 - ts19_flag].size() - 1; i++) {
		if (state->building[1 - ts19_flag][i].building_type != Programmer &&
			state->building[1 - ts19_flag][i].building_type != Base) {
			en_b_utility += calculate_utility(state->building[1 - ts19_flag][i]);
		}
	}
	for (int i = 0; i < state->soldier[ts19_flag].size() ; i++) {
		s_utility += calculate_utility(state->soldier[ts19_flag][i]);
	}
	for (int i = 0; i <= state->soldier[1 - ts19_flag].size() - 1; i++) {
		en_s_utility += calculate_utility(state->soldier[1 - ts19_flag][i]);
	}

	int utility = 0.65 * (en_b_utility - b_utility) + 0.35 * (en_s_utility - s_utility);
	return utility >= 0 ? utility : basic_develop;
}

int _Resource::assess() {
	vector<Building> building_set;
	vector<Building> en_building_set;
	for (auto iter = state->building[ts19_flag].cbegin(); iter != state->building[ts19_flag].cend(); iter++) {
		if (iter->building_type == Programmer) {
			building_set.push_back(*iter);
		}
	}
	for (auto iter = state->building[1 - ts19_flag].cbegin(); iter != state->building[1 - ts19_flag].cend(); iter++) {
		if (iter->building_type == Programmer) {
			en_building_set.push_back(*iter);
		}
	}

	int resource_get = 0;
	int en_resource_get = 0;
	for (int i = 0; i <= building_set.size() - 1; i++)
		resource_get += (OriginalBuildingAttribute[Programmer][ORIGINAL_ATTACK] *
		((building_set[i].level - BIT) * 0.5 + 1));
	for (int i = 0; i <= en_building_set.size() - 1; i++)
		en_resource_get += (OriginalBuildingAttribute[Programmer][ORIGINAL_ATTACK] *
		((en_building_set[i].level - BIT) * 0.5 + 1));

	//计算单位时间资源获得数
	int delta = en_resource_get - resource_get;
	int backward = (state->age[ts19_flag] > state->age[1 - ts19_flag]) ? 1 : 5;
	int resource_dis = state->resource[ts19_flag].resource - state->resource[1 - ts19_flag].resource;
	int utility = (0.2 * resource_dis + backward + 0.8 * delta);
	return  utility >= 0 ? utility : 0;
}

int _Attack::assess() {
	int w1 = 1;  //敌方主基地掉血权重 
	int utility = w1 * (calculate_hp(state->building[1 - ts19_flag][0]) - state->building[1 - ts19_flag][0].heal);
	return utility;
}

int _Defend::assess() {
	int w1 = 1;  //主基地掉血权重 
	int utility = w1 * (calculate_hp(state->building[ts19_flag][0]) - state->building[ts19_flag][0].heal);
	return utility;
}

int _BuildingNode::assess() {
	//遍历敌方建筑列表 
	vector<Building>& building_set = state->building[1 - ts19_flag];
	int  num[Building_Type] = {};  //保存敌方各种建筑数量 ，话说这一步可以在refresh做 
	for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
		num[int(iter->building_type)] ++;
	}
	int restrain = 0; //表示我方建筑对敌方的克制程度
	for (int i = 0; i<Building_Type; i++)
	{
		restrain += num[i] * restrain_table[int(this->buildingtype)][i];
	}
	int w1 = 1, w2 = 1;
	this->utility = w1 * restrain + w2 * calculate_utility(this->buildingtype, ts19_flag);  //克制能力和建筑本身效用值加权得到最终效用值 
	return utility;		//不明意图，得问
}

int _Programmar::assess() {
	return 1;
}

int _Sell::assess() {
	return 0;
}
void _Sell::execute() {
	int preserve_num = 5;	//保留指令数
	vector<Building> temp;
	for (int i = 0; i <= state->building[ts19_flag].size() - 1; i++) {//血量不够,且排除一些建筑类型
		if (state->building[ts19_flag][i].heal < 0.25 * calculate_hp(state->building[ts19_flag][i])) {
			if (state->building[ts19_flag][i].building_type != Base &&
				state->building[ts19_flag][i].building_type != Programmer &&
				state->building[ts19_flag][i].building_type != Musk) {
				temp.push_back(state->building[ts19_flag][i]);
			}
		}
	}

	for (int i = 0; i <= preserve_num - 1 && ins_num > 0; i++) {
		if (i >= temp.size())
			break;
		else {
			ins_num--;
			sell(temp[i].unit_id);
		}
	}//按顺序选出前五个

}

vector<Position> _Construct::find_soldier_pos(vector<Position> &b_pos) {
	vector<Position> s_pos;
	if (buildingtype >= Bool && buildingtype <= Hawkin) {
		for (int i = 0; i <= b_pos.size() - 1; i++) {
			s_pos.push_back(Position());
		}
		return s_pos;
	}
	else if (buildingtype >= Shannon && buildingtype <= Tony_Stark) {
		for (auto iter = b_pos.cbegin(); iter != b_pos.cend(); iter++) {
			int dis = map2[iter->x][iter->y];
			tranverse_r(*iter, dis, s_pos);
		}
		return s_pos;
	}
}
int _Construct::assess() {
	int w1 = 1;
	vector<Building>& building_set = state->building[ts19_flag];
	int space = lim_building_num - building_set.size();	//???????
	utility = space * w1;
	return utility;
}
struct Node2 {
	int utility;
	Position p;
	friend bool operator < (const Node2 &a, const Node2 &b) {
		return a.utility < b.utility;
	}
};
vector<Position> _Construct::find_best_place()
{
	//对于防御建筑来说 
	if (this->buildingtype >= Bool && this->buildingtype <= Hawkin) {

		vector<int> load_fit = vector<int>(road_num); //判断适合在哪条路周围建造该建筑 
		for (int i = 0; i < road_num; i++)
		{
			for (int j = 0; j < all_unit_num; j++)
				load_fit[i] += ensoldier_num[i][j] * restrain_table[this->buildingtype][j];  //此处应有bug 
		}

		priority_queue<Node2> q;    //优先级队列，保存utility最高的位置 

		int w1 = 1, w2 = 1;
		for (int i = 0; i<50; i++)    //从主基地周围开始寻找，只寻找50*50的地方 
			for (int j = 0; j<50; j++) {
				if (i<7 && j<7)  continue; //不找主基地 
				if (map[i][j] == legal_area)
				{
					int distance = map2[i][j] % 100 > calculate_attackRange(this->buildingtype, state->age[ts19_flag]) ? map2[i][j] % 100 : 0; //只要不超过攻击范围，离路距离越大越好 
					utility = w1 * distance + w2 * load_fit[map2[i][j] / 100];   //map2申明在tree中，未实现 
					Node2* temp = new Node2(); temp->utility = utility; temp->p = Position(i, j);
					q.push(*temp);
				}
			}
		vector<Position> best_places;
		for (int i = 0; i<this->construct_num; i++)
		{
			best_places.push_back(q.top().p); q.pop();
		}
		return best_places;
	}
//对于进攻建筑来说 
	/*
if (this->buildingtype >= Shannon && this->buildingtype <= Tony_Stark) {//太麻烦了未完成，可以先注释掉用random 
	vector<int> weakness = vector<int>(road_num);    //根据我方小兵死亡速度 判断这条路是否值得进攻，这么做可能会落后于敌方建筑情况,另外初始可能有问题 
															//上回合开始时这条路上某类小兵血量 - 上回合掉血量 + 上回合生产量 =  本回合开始时这条路上某类小兵血量
	vector<Building>& building_set = state->building[ts19_flag];
	vector<int> production = vector<int>(road_num);  //话说这一步可以在refresh做 , state里面保存的是回合开始时的数据吗？ 
	for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
		production[map[iter->pos.x][iter->pos.y] - road1] += OriginalSoldierAttribute[OriginalBuildingAttribute[this->buildingtype][4]][2];//如何计算某类建筑对应的小兵血量？ 
	}
	for (int i = 0; i<road_num; i++)
	{
		for (int j = 0; j<all_unit_num; j++)
			weakness[i] += mysoldier_heal_last[i][this->buildingtype] - mysoldier_heal[i][this->buildingtype] + production[i];
	}

	priority_queue<Node2> q;    //优先级队列，保存utility最高的位置 

	int w1 = 1, w2 = 1, w3 = 1;
	Position my_base = state->building[ts19_flag][0].pos;
	for (int i = 0; i<190; i++)    //从主基地周围开始寻找，只寻找190*190的地方 
		for (int j = 0; j<190; j++) {
			if (i<7 && j<7)  continue; //不找主基地 
			if (map[i][j] == legal_area)
			{
				int distance = map2[i][j] % 100 > calculate_attackRange(this->buildingtype, state->age[ts19_flag]) ? map2[i][j] % 100 : 0; //只要不超过攻击范围，离路距离越大越好 
				utility = w1 * dist(Position(i, j), my_base) + w2 * weakness[map2[i][j] / 100] + w3 * distance;
				Node2* temp = new Node2(); temp->utility = utility; temp->p = Position(i, j);
				q.push(*temp);
			}
		}
	vector<Position> best_places;
	for (int i = 0; i<this->construct_num; i++)
	{
		best_places.push_back(q.top().p); q.pop();
	}
	return best_places;

}*/


}

void _Construct::execute() {
	int cost = calculate_resource_cost(buildingtype, state->age[ts19_flag]);
	float f_num = max_resource / cost;
	int num = max_resource / cost;
	if (f_num - num > 0.4) num++;
	construct_num = num;						//选取建造数目
	vector<Position> temp = find_best_place();
	vector<Position> s_pos = find_soldier_pos(temp);
	for (int i = 0; i <= temp.size() - 1 && ins_num > 0; i++) {
		construct(this->buildingtype, temp[i], s_pos[i]);
		ins_num--;
	}

}

int _Maintain::assess() {
	int w1 = 1;
	int loss = 0;
	//计算所有本类建筑的掉血量 
	vector<Building>& building_set = state->building[ts19_flag];
	for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
		if (iter->building_type == buildingtype)
			loss += calculate_hp(*iter) - iter->heal;
	}
	utility = w1 * loss;
	return utility;

}

int _Upgrade::assess() {
	int w1 = 1;
	int age = state->age[ts19_flag];
	int space = 0;
	vector<Building> building_set = state->building[ts19_flag];
	//计算目前本类建筑升级空间，即能升级但还未升级的建筑有多少 
	for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
		if (iter->building_type == buildingtype)
			space += age - iter->level;
	}
	utility = w1 * space;
	return utility;
}

void _UpgradeAGE::execute() {
	update_resource += max_resource;
	if (update_resource >= UPDATE_COST + UPDATE_COST_PLUS * (state->age[ts19_flag] - BIT)
		&& ins_num > 0) {
		updateAge();
		ins_num--;
	}
	else {
		if (all_state[all_state.size() - 1]->age[ts19_flag] < state->age[ts19_flag]) {
			update_resource -= (UPDATE_COST + UPDATE_COST_PLUS * (state->age[ts19_flag] - BIT - 1));
		}
	}
}//

struct msg {	//辅助类型
	float up_index;		//utility_price_index	
	int cost;
	int id;
	msg(float a, int c, int i) { up_index = a; cost = c; id = i; }
	bool operator > (const msg &a) const {
		return cost > a.cost;
	}
	bool operator < (const msg &a) const {
		return cost < a.cost;
	}
};

void _Maintain::execute() {
	vector<int> temp = min_cost();
	for (int i = 0; i < temp.size() && ins_num > 0; i++) {
		toggleMaintain(temp[i]);
		ins_num--;
	}
}

void _Upgrade::execute() {
	vector<int> upgrade_list = min_cost();
	for (int i = 0; i <= upgrade_list.size() - 1 && ins_num > 0; i++) {
		upgrade(upgrade_list[i]);
		ins_num--;
	}
}


vector<int> _Maintain::min_cost() {//
	vector<Building> temp;
	for (auto iter = state->building[ts19_flag].cbegin();
		iter != state->building[ts19_flag].cend();
		iter++) {
		if (iter->building_type == this->buildingtype && iter->level >= (int)state->age - 4) {
			temp.push_back(*iter);
		}//只考虑等级差在4以内的建筑
	}
	vector<int> result;
	vector<msg> index_array;
	float mean_index = 0;
	for (auto iter = temp.cbegin(); iter != temp.cend(); iter++) {
		float blood_percentage = 1 - (iter->heal / calculate_hp((*iter)));
		float percentage = (blood_percentage > 0.2) ? 0.2 : blood_percentage;
		int price = calculate_resource_cost(iter->building_type, iter->level) * blood_percentage;
		float index = percentage / price;
		//用简单的贪婪算法，先找
		mean_index += index;
		index_array.push_back(msg(index, price, iter->unit_id));
	}
	mean_index /= index_array.size();
	vector<msg> temp_2;
	for (auto iter = index_array.cbegin(); iter != index_array.cend(); iter++) {
		if (iter->up_index > 0.8*mean_index) {
			temp_2.push_back((*iter));
		}
	}//	筛去小于平均index*0.8的建筑
	 //在从中找最小成本
	std::sort(temp_2.begin(), temp_2.end());
	int used_resource = 0; int i = 0;
	while (used_resource <= max_resource && i < temp_2.size()) {//直接找最小成本
		if (temp_2[i].cost <= max_resource - used_resource) {
			result.push_back(temp_2[i].id);
			used_resource += temp_2[i].cost;
			i++;
		}
		else
			break;
	}
	return result;
}


vector<int> _Upgrade::min_cost() {	//需要考虑建造力限制，资源限制，再达最优，实际上，提升效用值基本一致，
									//消耗资源和建造力一致
									//所以优先升级血量高的，等级不低的即可
	vector<Building> temp;
	for (int i = 0; i <= state->building[ts19_flag].size() - 1; i++) {
		if (state->building[ts19_flag][i].building_type == buildingtype
			&& state->building[ts19_flag][i].level < (int)state->age
			&& state->building[ts19_flag][i].heal >= 0.3 * calculate_hp(state->building[ts19_flag][i])) {
			//优先选择，血量较高的，因为建筑等级均从最低级开始，所以不考虑等级了
			temp.push_back(state->building[ts19_flag][i]);
		}
	}

	vector<int> id;
	vector<msg> msg_index;
	int f_restrict = max_resource / calculate_upgradeB_Fcost(temp[0]);	//计算资源
	int b_restrict = max_power / calculate_upgradeB_Rcost(temp[0]);		//计算建造力
	int max_num = (f_restrict >= b_restrict) ? b_restrict : f_restrict;
	for (auto iter = temp.cbegin(); iter != temp.cend(); iter++) {
		msg_index.push_back(msg(0, iter->heal, iter->unit_id));

	}
	std::sort(msg_index.begin(), msg_index.end());
	for (int i = 0; i <= max_num - 1; i++) {
		if (msg_index.size() - 1 - i >= 0) {
			id.push_back(msg_index[msg_index.size() - 1 - i].id);
		}
		else
			break;
	}
	return id;

}

vector<Position> _Programmar::find_best_place(int n) {//在基地周围
	int r = 7;
	Position my_base = state->building[ts19_flag][0].pos;
	if (my_base.x > 100) {
		my_base.x = MAP_SIZE - 1;
		my_base.y = MAP_SIZE - 1;
	}
	else {
		my_base.x = 0;
		my_base.y = 0;
	}
	vector<Position> good_place;
	while (n > good_place.size()) {
		tranverse_pro(my_base, r, good_place);
		r++;
	}
	return good_place;
}

void _Programmar::sell_programmer() {
	vector<Building> temp;
	for (int i = 0; i <= state->building[ts19_flag].size() - 1; i++) {
		if (state->building[ts19_flag][i].building_type == Programmer
			&& state->building[ts19_flag][i].heal < 0.18 * calculate_hp(state->building[ts19_flag][i])) {//血量过低才会排除
			temp.push_back(state->building[ts19_flag][i]);
		}
	}


	int abondon_bound = 3;			//每回合最多三个
	for (int i = 0; i <= temp.size() - 1; i++) {
		if (temp[i].level < (int)(state->age[ts19_flag] - 2) && abondon_bound > 0) {
			sell(temp[i].unit_id);
			abondon_bound--;
		}
	}

}

void _Programmar::execute() {	//现在需要考虑Programmer的建造，升级（升级最主要）
	sell_programmer();			//排除老旧建筑
	int upbound = 8 + 2 * (int)(state->age - BIT);
	int programmer_num = 0;
	int grade_space = 0;
	vector<Building> pros;
	for (int i = 0; i <= state->building[ts19_flag].size() - 1; i++) {
		if (state->building[ts19_flag][i].building_type == Programmer) {
			pros.push_back(state->building[ts19_flag][i]);
			grade_space += (state->age[ts19_flag] - state->building[ts19_flag][i].level);
		}
	}
	programmer_num = pros.size();
	int re_cost = calculate_resource_cost(Programmer, state->age[ts19_flag]);
	int fo_cost = calculate_building_force(Programmer, state->age[ts19_flag]);
	int upgrade_re_cost = 0.5 * OriginalBuildingAttribute[Programmer][ORIGINAL_RESOURCE];
	int update_f_cost = 0.5 * OriginalBuildingAttribute[Programmer][ORIGINAL_BUILDING_POINT];



	int build_num = (max_resource / re_cost > max_power / fo_cost) ? max_power / fo_cost : max_resource / re_cost;
	build_num = (build_num > upbound - programmer_num) ? upbound - programmer_num : build_num;
	vector<Position> temp = find_best_place(build_num);
	for (int i = 0; i <= temp.size() - 1 && ins_num > 0; i++) {
		construct(Programmer, temp[i]);
		ins_num--;
	}
}
