
#include"communication.h"
#include<vector>
#include<iostream>
#include <algorithm>
using std::cout;
using std::endl;
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



/**************************各种辅助函数**********************/
int abs(int x, int y) { return x > y ? x - y : y - x; }
int dist(Position a, Position b) { return abs(a.x, b.x) + abs(a.y, b.y); }
Position find_best(int building_type);	//寻路函数


bool construct_legal(Position pos) {

}

float calculate_special(BuildingType b,int flag) {

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

float calculate_utility(Soldier a) {
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
	Position find_best_place();			//寻找到最佳的位置来建造
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


/*void Node::tick(int f_power, int f_resource, int f_utility)
{
	//参数为父节点分配到的建造力、资源; 兄弟节点总效用 
	max_power = f_power * utility / f_utility;
	max_resource = f_resource * utility / f_utility;  // 应该有更好的分配方案 
	if (this->children.size() != 0)
	{
		int all_utility = 0;
		for (int i = 0; i < this->children.size(); i++){
			all_utility += children[i]->assess();
		}
		for (int i = 0; i< this->children.size(); i++){
			children[i]->tick(max_power, max_resource, all_utility);
		}
	}
	else
		execute();

}//????*/

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
	float threaten_buliding[all_buildings_num] = {};				//敌方各种类型单位的威胁值
	float threaten_soldier[3][all_unit_num] = {};					//每条路上每种兵的威胁
	//float weight[all_operation] = {	1, 1, 0.1, 0.1, 0.9 };			//己方各种操作权重
	float building_weight[Building_Type] = {};						//各种建筑的权重，建筑标识参照api文件
	float soldier_weight[Soldier_Type] = 
	{ };															//对各种士兵的基础权重
	float evaluate_table[3] = {};									//对三个节点做出评估
	//每回合需更新的信息
	int map[MAP_SIZE][MAP_SIZE];							//自定义map，显示路与建筑
	float ensoldier_num[3][all_unit_num] = {};				//每条路上的敌方兵力
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
	evaluate();									//分配好第一层值，在每个子节点的execute函数尾写出对下一层子节点的递归操作
	for (int i = 0; i <= 2; i++) {
		this->root->children[i]->execute();
	}
}

void Tree::refresh_map() {	//更新地图
	if (state->turn >= 1) {
		vector<Building>* building_set = state->building;
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
		for (auto iter = building_set_last[ts19_flag].cbegin();
			iter != building_set_last[ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = my_building;
		}
		for (auto iter = building_set_last[1 - ts19_flag].cbegin();
			iter != building_set_last[1 - ts19_flag].cend();
			iter++) {
			map[iter->pos.x][iter->pos.y] = en_building;
		}
	}
}

void Tree::init_map() {		//初始化地图并将路进行标号
	if (state->turn == 0) {
		my_base_pos = state->building[ts19_flag][0].pos;
		enemy_base_pos = state->building[1 - ts19_flag][0].pos;
		for (int i = 0; i <= MAP_SIZE - 1; i++) {
			int road_count = 0;
			for (int j = 0; j <= MAP_SIZE - 1; j++) {
				switch (ts19_map[i][j])
				{
				case 1: {
					map[i][j] = road1 + road_count;
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

void Tree::refresh_unit() {		//将敌方unit统计表清零，并重新统计
	for (int i = 0; i <= 2; i++) 
		for (int j = 0; j <= all_unit_num - 1; j++) {
			ensoldier_num[i][j] = 0;
		}
	vector<Soldier>& soldier_set = state->soldier[1 - ts19_flag];
	for (auto iter = soldier_set.cbegin(); iter != soldier_set.cend(); iter++) {
		ensoldier_num[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] ++;
	}
}





/*************************************子节点方法***************************/
int _UpgradeAGE::assess() {

}
int _Development::assess() {
}
int _Resource::assess() {
}
int _Attack::assess() {
}
int _Defend::assess() {

}
int _BuildingNode::assess() {
}

int _Programmar::assess() {
}
int _Sell::assess() {
}

void _UpgradeAGE::execute() {
}
void _Construct::execute() {
}
void _Maintain::execute() {
}
void _Upgrade::execute() {
}
void _Programmar::execute() {
}
void _Sell::execute() {
}
*/
