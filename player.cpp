#include"communication.h"
#include<vector>
#include<priority_queue>
#include<iostream>
#include <algorithm>
using std::cout;
using std::endl;
using std::priority_queue;
using std::vector;
using std::find;

extern State* state;
extern std::vector<State* > all_state;
extern int** ts19_map;
extern bool ts19_flag;

/**********************自定义变量*************************/
const int all_type = 28;						//总共27种单位与建筑加上敌方Age
const int all_buildings_num = Building_Type;	//所有建筑数量
const int all_unit_num = Soldier_Type;			//所有单位数量
const int all_operation = 5;		
const int max_command_num = 50;	//单回合最多打指令数
Position my_base_pos;
Position enemy_base_pos;

enum operation {			//各种操作
	_updateAGE,
	_construct,
	_sell,
	_maintain,
	_upgrade
};

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
	_resource,
	_range,
	_hp,
	_pos,
	_special,
	_preference,
	_attribute
};







/***********************参数表************************/
const double utility_weight1[Soldier_Type][_attribute] = {
	//单位				攻击		资源		攻击距离		生命		位置		特殊加成		偏好
	{ BIT_STREAM,		0.4,	0,		0.05,		0.4,	0,		0,			1},
	{ VOLTAGE_SOURCE,	0.35,	0,		0.05,		0.45,	0,		0,			1},
	{ CURRENT_SOURCE,	0.4,	0,		0,			0.4,	0.5,	0,			1},
	{ ENIAC,			0.4,	0,		0.05,		0.4,	0,		0,			1},
	{ PACKET,			0.4,	0,		0,			0.4,	0.5,	0,			1},
	{ OPTICAL_FIBER,	0.35,	0,		0.05,		0.45,	0,		0,			1},
	{ TURING_MACHINE,	0.4,	0,		0.05,		0.4,	0,		0,			1},
	{ ULTRON,			0.4,	0,		0,			0.4,	0.5,	0,			1}
};

const double utility_weight2[Building_Type][_attribute] = {
	//建筑				攻击		资源		攻击距离		生命		位置		特殊加成		偏好
	{ __Base,			0,		0,		0,			0,		0,		0,			0},
	{ Shannon,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Thevenin,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Norton,			0.1,	0,		0,			0.4,	0.3,	0,			1},
	{ Von_Neumann,		0.1,	0,		0,			0.4,	0,		0,			1},
	{ Berners_Lee,		0.1,	0,		0,			0.4,	0.33,	0,			1},
	{ Kuen_Kao,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Turing,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Tony_Stark,		0.1,	0,		0,			0.4,	0.4,	0,			1},
	{ Bool,				0.2,	0,		0.05,		0.5,	0,		0,			1},
	{ Ohm,				0.4,	0,		0.05,		0.5,	0,		4,			1},//可以发动特效时，加成特殊*加成函数
	{ Mole,				0.3,	0,		0.07,		0.5,	0,		2,			1},
	{ Monte_Carlo,		0.3,	0,		0.05,		0.5,	0,		0,			1},
	{ Larry_Roberts,	0.4,	0,		0.05,		0.5,	0,		3,			1},
	{ Robert_Kahn,		0.4,	0,		0.05,		0.5,	0,		1,			1},
	{ Musk,				0,		0,		0.2,		0.8,	0,		20,			1},
	{ Hawkin,			0,		0,		0.3,		0.8,	0,		30,			1},
	{ Programmer,		0,		1,		0,			0,		0.1,	0,			1}
};

const double restrain_table[Building_Type][Building_Type] = {   //建筑相克表，未修改 
		//建筑			__Base	Shannon	Thevenin    Norton	Von_Neumann		Berners_Lee	Kuen_Kao Turing  Tony_Stark  Bool  Ohm Mole Monte_Carlo  Larry_Roberts Robert_Kahn 	Musk Hawkin Programmer	
	{ __Base,			0,		0,		0,			0,		0,		0,			0},
	{ Shannon,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Thevenin,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Norton,			0.1,	0,		0,			0.4,	0.3,	0,			1},
	{ Von_Neumann,		0.1,	0,		0,			0.4,	0,		0,			1},
	{ Berners_Lee,		0.1,	0,		0,			0.4,	0.33,	0,			1},
	{ Kuen_Kao,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Turing,			0.1,	0,		0,			0.4,	0,		0,			1},
	{ Tony_Stark,		0.1,	0,		0,			0.4,	0.4,	0,			1},
	{ Bool,				0.2,	0,		0.05,		0.5,	0,		0,			1},
	{ Ohm,				0.4,	0,		0.05,		0.5,	0,		4,			1},
	{ Mole,				0.3,	0,		0.07,		0.5,	0,		2,			1},
	{ Monte_Carlo,		0.3,	0,		0.05,		0.5,	0,		0,			1},
	{ Larry_Roberts,	0.4,	0,		0.05,		0.5,	0,		3,			1},
	{ Robert_Kahn,		0.4,	0,		0.05,		0.5,	0,		1,			1},
	{ Musk,				0,		0,		0.2,		0.8,	0,		20,			1},
	{ Hawkin,			0,		0,		0.3,		0.8,	0,		30,			1},
	{ Programmer,		0,		1,		0,			0,		0.1,	0,			1}
}

/**************************各种辅助函数**********************/
int abs(int x, int y) { return x > y ? x - y : y - x; }
int dist(Position a, Position b) { return abs(a.x, b.x) + abs(a.y, b.y); }
Position find_best(int building_type);	//寻路函数


/*bool construct_legal(Position pos) {

}
float calculate_special(BuildingType b,int flag) {

}*/
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
float calculate_hp(Soldier a,int level) {//提供等级，用来计算为建造的兵的生命
	return (1 + 0.5*level)* OriginalSoldierAttribute[a.soldier_name][SOLDIER_ORIGINAL_HEAL];
}
float calculate_attackRange(Building b) {
	return (1 + 0.5*b.level) * OriginalBuildingAttribute[b.building_type][ORIGINAL_RANGE];
}
float calculate_CD(Building b) {
	return (1 + 0.5*b.level) * OriginalBuildingAttribute[b.building_type][ORIGINAL_RANGE];
}
float calculate_resource_cost(BuildingType b, int level) {
	return (1 + 0.5*level) * OriginalBuildingAttribute[b][ORIGINAL_RESOURCE];
}
float calculate_building_force(BuildingType b, int level) {
	return (1 + 0.5*level) * OriginalBuildingAttribute[b][ORIGINAL_BUILDING_POINT];
}
float calculate_utility(Soldier a) {   //计算一个已经存在的兵的效用 
	int distance = (a.flag == ts19_flag) ? dist(a.pos, enemy_base_pos) : dist(a.pos, my_base_pos);
	return	a.heal * utility_weight1[a.soldier_name][_hp] +
			distance * utility_weight1[a.soldier_name][_pos] +
			calculate_attack(a) * utility_weight1[a.soldier_name][_attack] +
			calculate_attackRange(a) * utility_weight1[a.soldier_name][_range];
	
	
}
float calculate_utility(Building b) {	//	对所有建筑都适用
	int distance = (b.flag == ts19_flag) ? dist(b.pos, enemy_base_pos) : dist(b.pos, my_base_pos);
	return b.heal * utility_weight2[b.building_type][_hp] +
		calculate_attack(b) * utility_weight2[b.building_type][_attack] +
		calculate_attackRange(b.building_type, b.level) * utility_weight2[b.building_type][_range] +
		calculate_special(b.building_type,b.flag) * utility_weight2[b.building_type][_special];
}

void f_player()
{
	if (state->turn == 0) {
		for (int i = 0; i <= MAP_SIZE - 1; i++) {
			for (int j = 0; j <= MAP_SIZE - 1; j++)
				cout << ts19_map[i][j];
		}
		cout << endl;
	}

};


/**************************行为树部分****************/

class Node
{
public:
	//函数成员 
	Node();
	~Node();
	void tick(int f_power, int f_resource, int f_utility);   //参数为父节点分配到的建造力、资源; 兄弟节点总效用 T
	virtual int assess() {
		std::cout << "assess" << endl;
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
	int assess();
	void execute();
};
class _Resource :public Node			//获取资源，一般不需要维修，但过时建筑需要清理或者升级，每回合资源获取量要定一个函数来判断
{										//考虑敌方资源获取量
public:
	int assess();
	void execute();
	int evaluate();						//资源获取目标估计
};
class _Development :public Node			//建造 进攻或防守 
										//计算攻击和防御需要达到的效用值，再加上原有的攻击防御偏好，分配每个行为的值
{								
public:
	int assess();
	void execute();
};

class _Attack :public Node				//根据权值表来，附带考虑对方的防御建筑效用表，进行对应策略
{
public:
	int assess();
	void execute();
};
class _Defend :public Node				//根据每条路每个单位的威胁值进行分配，按照需要程度分配，某些单位有对应克制单位，这些单位加大投入，
										//其余就按照权值表来
{
public:
	int assess();
	void execute();
	float table[8] = {};				//防御建筑权值表
	void fresh_table();					//防御建筑权值更新，不能建的建筑权值为0，特殊克制建筑权值上升，其余简直按效用值排序，总和归一化
};
class _Programmar :public Node			//码农的维修，升级，建造，单独有个效用函数，根据每回合获得的需要效用值进行
{										//感觉码农不需要考虑太复杂，建造时生产到远一点的位置就行，基本不需要考虑维修
public:
	int assess();
	void execute();
};
class _Sell :public Node				//通过卖房子获取资源
{
public:
	int assess();
	void execute();
	vector<Building> sell_list();		//若效用小于某一个阈值（可以定为平均值75%），则淘汰，有数量上限
										//顺便再考虑当建筑数量较多时，出售效用较低的一批建筑
};

class _BuildingNode	:public Node		//代表行为树中建筑节点部分
{										//决策说明：假设有评估效用的函数，该节点需要达到某效用值，
										//此外还有成本限制数
										//遍历全部建筑得到 每个建筑，每种操作(f,c)
										//目标：选取某建筑下的某操作，使得sum(ci)最小且在成本预算内，达到效用最大化；
										//可以先写个简化版

public:
	int assess();
	void execute();
	void attribute();				//决策如何分配建筑，按整数分配到下面的节点，由成本决定

	BuildingType buildingtype;
	_BuildingNode(BuildingType b) :buildingtype(b) {}

};
/*class _BuildingMethod : public Node {	//决定以何种方式加强建筑
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_BuildingMethod(BuildingType b) : buildingtype(b) {}
};*/
class _Construct :public Node
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_Construct(BuildingType b) :buildingtype(b) {}

	void fresh_num(int num) { construct_num = (num >= 0) ? num : 0; }
	vector<Position> find_best_place();			//寻找到最佳的位置来建造
private:
	int construct_num = 0;
};
class _Maintain :public Node
{
public:
	
	int assess();
	void execute();
	BuildingType buildingtype;
	_Maintain(BuildingType b) :buildingtype(b) {}	

	void fresh_num(int num) { maintain_num = (num >= 0) ? num : 0; }
	vector<Building> min_cost();	//返回维修费用最少的，限定数量的建筑，切记每回合维修最多20%的血量
private:
	int maintain_num = 0;
};
class _Upgrade :public Node
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_Upgrade(BuildingType b) :buildingtype(b) {}

	void fresh_num(int num) { upgrade_num = (num >= 0) ? num : 0; }
	vector<Building> min_cost();		//返回升级费用最小且最优的建筑
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
		for (int i = 0; i < this->children.size(); i++){
			all_utility += children[i]->assess();    //调用assess的时候需要计算并保存节点的utility 
		}
		for (int i = 0; i< this->children.size(); i++){
			children[i]->tick(max_power, max_resource, all_utility);
		}
	}
	else
		execute();
}

class Tree
{
public:
	Node * root;
	Tree();	     //建树，在init中调用 
	void evaluate();		//评估，并对第一层做出分配
	void init_map();		//初始化地图
	void refresh_map();		//刷新己方地图
	void refresh_unit();	//刷新单位数量信息
	void tranverse();		//遍历
private:
	//参数表
    int road_num = 3; //需要在init中计算 
	float threaten_buliding[all_buildings_num] = {};				//敌方各种类型单位的威胁值
	float threaten_soldier[road_num][all_unit_num] = {};					//每条路上每种兵的威胁
	//float weight[all_operation] = {	1, 1, 0.1, 0.1, 0.9 };			//己方各种操作权重
	float building_weight[Building_Type] = {};						//各种建筑的权重，建筑标识参照api文件
	float soldier_weight[Soldier_Type] = 
	{ };															//对各种士兵的基础权重
	float evaluate_table[3] = {};									//对三个节点做出评估
	//每回合需更新的信息
	int map[MAP_SIZE][MAP_SIZE];							//自定义map，显示路与建筑
	float mysoldier_heal[road_num][all_unit_num] = {};           //每条路上的我方血量
	float mysoldier_heal_last[road_num][all_unit_num] = {};           //每条路上的我方血量_上回合 
	float ensoldier_num[road_num][all_unit_num] = {};				//每条路上的敌方兵力
	float enbuilding_num[road_num][all_buildings_num] = {};           //每条路附近的敌方建筑 
	int lim_building_num;    //建筑数目上限，初始和升级时维护 
	int lim_building_force;   //建造力上限 ，初始和升级时维护 
	int map2[MAP_SIZE][MAP_SIZE]; //维护到路的距离，假如最近的是road_n, 距离为15，则表示为 n15 , 地图尚未实现 
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
	refresh_map();
	refresh_unit();
	evaluate();									
	/*for (int i = 0; i <= 2; i++) {
		this->root->children[i]->execute();
	}*/
	root->utility = 1;
    root->tick(60+40*state->age[ts19_flag], state->resource[ts19_flag], root->utility)
}

void Tree::refresh_map() {	//更新地图
	if (state->turn >= 1) {   //直接全部设blank? 
		vector<Building>* building_set = state->building;     //把一个数组赋值给一个vector? 
		vector<Building>* building_set_last = all_state.back()->building;
		for (int index = 0; index <= 1; index++) {
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
		}
		//有问题，尚未考虑主建筑 
		for (auto iter = building_set_last[ts19_flag].cbegin();
			iter != building_set_last[ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = my_building;
			for(int i=-8; i<=8; j++)                 //在自己建筑周围第一轮遍历，将8以内设为可建造区域 
			    for(int j=-8; j<=8; j++)
			        if((i==0 && j==0) || outofRange(iter->pos.x + i,iter->pos.y + j))    //outogrange还没写 
			            break;
			        else
			            map[iter->pos.x + i][iter->pos.y + j] = legal_area;
		}
		for (auto iter = building_set_last[ts19_flag].cbegin();
			iter != building_set_last[ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = my_building;
			for(int i=-2; i<=2; j++)                 //在自己建筑周围第二轮遍历，将2以内设为不可建造区域 
			    for(int j=-2; j<=2; j++)
			            if((i==0 && j==0) || outofRange(iter->pos.x + i,iter->pos.y + j)) 
			                break;
			            else
			                map[iter->pos.x + i][iter->pos.y + j] = blank;
		}
		for (auto iter = building_set_last[1 - ts19_flag].cbegin();
			iter != building_set_last[1 - ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = en_building;
		}
	}
}

void Tree::init_map() {		//初始化地图并将路进行标号 ,第0回合不需要对建筑标记吗？ 
	if (state->turn == 0) {    //如果路不往回拐的话，似乎可以简单地解决，如果往回拐我就不会了 
		my_base_pos = state->building[ts19_flag][0].pos;
		enemy_base_pos = state->building[1 - ts19_flag][0].pos; 
		//先处理和左边主基地不在同一行的路 
		for (int i = 7; i <= MAP_SIZE - 1; i++) {      //为啥是《=mapsize-1? 
			int road_count = 0;
			for (int j = 0; j <= MAP_SIZE - 1; j++) {
				switch (ts19_map[i][j])
				{
				case 1: {
					map[i][j] = road1 + road_count;
					if(j+1 != MAP_SIZE && map[i][j+1] != 1)  //考虑到同一条路可能横在同一行中的情况 
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
			int road_count = 0;  int i = 7
		    for(int j=0; j<=7; j++) if(ts19_map[i][j] == 1) road_count += 1;
			for (int i = 0; i < 7; i++) {      
			    for (int j = 0; j <= MAP_SIZE - 1; j++){
			    	switch (ts19_map[i][j])
				{
				    case 1: {
					    map[i][j] = road1 + road_count;
					    if(j+1 != MAP_SIZE && map[i][j+1] != 1)  //考虑到同一条路可能横在同一行中的情况 
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
		}
	 
	}
}

void Tree::refresh_unit() {		//将敌方unit统计表清零，并重新统计
	for (int i = 0; i <= road_num; i++) 
		for (int j = 0; j <= all_unit_num - 1; j++) {
			ensoldier_num[i][j] = 0;
		}
	vector<Soldier>& soldier_set1 = state->soldier[1 - ts19_flag];
	for (auto iter = soldier_set1.cbegin(); iter != soldier_set1.cend(); iter++) {
		ensoldier_num[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] ++;
	}
	                           //将我方unit统计表清零，并重新统计
	for (int i = 0; i <= roda_num; i++) 
		for (int j = 0; j <= all_unit_num - 1; j++) {
			mysoldier_heal[i][j] = 0;
		}
	vector<Soldier>& soldier_set2 = state->soldier[ts19_flag];
	for (auto iter = soldier_set2.cbegin(); iter != soldier_set2.cend(); iter++) {
		mysoldier_heal[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] += iter->heal;
	}
	
	for (int i = 0; i <= roda_num; i++)   //将我方上回合unit血量统计表清零并重新统计 
		for (int j = 0; j <= all_unit_num - 1; j++) {
			mysoldier_heal_last[i][j] = 0;
		}
	vector<Soldier>& soldier_set3 = all_state.back()->soldier[ts19_flag]
	for (auto iter = soldier_set3.cbegin(); iter != soldier_set3.cend(); iter++) {
		mysoldier_heal_last[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] += iter->heal;
	}
}





/*************************************子节点方法***************************/
int _UpgradeAGE::assess() {
    int backward = state->age[1 - ts19_flag] > state->age[ts19_flag]? 10:1;  //等级落后于敌人时大力追赶，需要调节 
    int en_re =  state->resource[1 - ts19_flag].resource;   
    this->utility = en_re + backward;
    return utility;
}
int _Development::assess() {
	
}
int _Resource::assess() {
}
int _Attack::assess() {
	int w1 = 1;  //敌方主基地掉血权重 
	int utility = w1*(calculate_hp(state->building[1-ts19_flag][0]) - state->building[1-ts19_flag][0].heal);
	return ultility;
}
int _Defend::assess() {
    int w1 = 1;  //主基地掉血权重 
	int utility = w1*(calculate_hp(state->building[ts19_flag][0]) - state->building[ts19_flag][0].heal);
	return ultility;
}
int _BuildingNode::assess() {
	//遍历敌方建筑列表 
    vector<Building>& building_set = state->building[1 - ts19_flag];
	int  num[Building_Type] = {};  //保存敌方各种建筑数量 ，话说这一步可以在refresh做 
    for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
	    num[int(iter->building_type)] ++;
	}
	int restrain = 0; //表示我方建筑对敌方的克制程度
	for(int i=0; i<Building_Type; i++)
	{
		restrain += num[i]*restrain_table[int(this->BuildingType)][i];
	 } 
	 int w1=1, w2=1;
	 this->utility = w1*restrain + w2*calculate_utility(this->BuildingType);  //克制能力和建筑本身效用值加权得到最终效用值 
	 return utility;
}

int _Programmar::assess() {
}
int _Sell::assess() {
}
int _Construct::assess(){
	int w1;
	vector<Building>& building_set = state->building[ts19_flag];
	int space =  lim_building_num - building_set.size();
	utility = space*w1;
	return utility;
}
int _Maintain::assess(){
	int w1 = 1;
	int loss = 0;
	//计算所有本类建筑的掉血量 
	vector<Building>& building_set = state->building[ts19_flag];
	for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
		if(iter->building_type == buildingtype)
	        loss += calculate_hp(*iter) - iter->heal;
	}
	utility = w1*loss;
	return utility;
	
} 
int _Upgrade()::assess(){
	int w1 = 1;
	int age = state->age[ts19_flag];
	int space = 0; 
	//计算目前本类建筑升级空间，即能升级但还未升级的建筑有多少 
	for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
		if(iter->building_type == buildingtype)
	        space += age - iter->level;
	}
	utility = w1*space;
	return utility;
}
void _UpgradeAGE::execute() {
	
}
void _Construct::execute() {
	
}
void _Maintain::execute() {
	vector<Building> temp = min_cost();
	for(int i=0; i<temp.size(); i++)
	    toggleMaintain(temp[i]->unit_id);
}
void _Upgrade::execute() {
}
void _Programmar::execute() {
}
void _Sell::execute() {
}
vector<Position> _Construct::find_best_place()
{
	//对于防御建筑来说 
	if( this->buildingtype >= Bool && this->buildingtype <= Hawkin){
	int load_fit[road_num] = {};      //判断适合在哪条路周围建造该建筑 
	for(int i=0; i<road_num; i++)
	{
		for(int j=0; j<all_unit_num; j++)
		    load_fit[i] += ensoldier_num[i][j]*restrain_table[this->buildingtype][j];  //此处应有bug 
	}            
	
	priority_queue<Node> q;    //优先级队列，保存utility最高的位置 
	struct Node {
        int utility;
        Position p;
        friend bool operator < (const node &a, const node &b) {  
            return a.priority < b.priority;
        }    
    };
	int w1=1, w2=1;
	for(int i=0; i<50; i++)    //从主基地周围开始寻找，只寻找50*50的地方 
	    for(int j=0; j<50; j++){
	    	if(i<7 && j<7)  continue; //不找主基地 
	    	if(map[i][j] == legal_area) 
	    	{ 
	    	    int distance = map2[i][j]%100 > alculate_attackRange(this->buildingtype)? map2[i][j]%100 : 0; //只要不超过攻击范围，离路距离越大越好 
	    	    utility = w1*distance + w2*load_fit[map2[i][j]/100]   //map2申明在tree中，未实现 
	    	    Node* temp = new Node(); temp->utility = utility; temp->p = Position(i,j);
	    	    q.push(*temp);
	    	} 
		}
	vector<Position> best_places;
	for(int i=0; i<this->construct_num; i++)
    {
    	best_places.push_back(q.top()); q.pop();
	}
	return best_places;
	}
	//对于进攻建筑来说 
    if( this->buildingtype >= Shannon && this->buildingtype <= Tony_Stark){//太麻烦了未完成，可以先注释掉用random 
    	int weakness[road_num] = {};    //根据我方小兵死亡速度 判断这条路是否值得进攻，这么做可能会落后于敌方建筑情况,另外初始可能有问题 
    	//上回合开始时这条路上某类小兵血量 - 上回合掉血量 + 上回合生产量 =  本回合开始时这条路上某类小兵血量
    	vector<Building>& building_set = state->building[ts19_flag];
    	int  production[road_num] = {};  //话说这一步可以在refresh做 , state里面保存的是回合开始时的数据吗？ 
        for (auto iter = building_set.cbegin(); iter != building_set.cend(); iter++) {
	        production[map[iter->pos.x][iter->pos.y]-road1] += caculate_hp();//如何计算某类建筑对应的小兵血量？ 
    	}
    	for(int i=0; i<road_num; i++)
	    {
		    for(int j=0; j<all_unit_num; j++)
		        weakness[i] += mysoldier_heal_last[i][this->buildingtype] - mysoldier_heal[i][this->buildingtype] + production[i];
    	} 
    	
	}        
}

