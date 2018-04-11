
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

enum operation {			//各种操作
	updateAGE,
	construct,
	sell,
	maintain,
	upgrade
};

enum map_identify {
	blank,
	my_building,
	en_building,
	road1,
	road2,
	road3,
	legal_area,
	base
};

int programmer_num = 0;
int defending_num = 0;
int producing_num = 0;
int map[MAP_SIZE][MAP_SIZE];	//自定义地图，显示所有建筑位置
vector<Building> building_last;	//上一回合所有建筑
vector<Building>* building = state->building;
/***********************参数表************************/



												/*****************************主要流程函数***************/
void init();
void fresh_map();	//更新自己的地图，标明建筑
void fresh_threaten();	//更新威胁值
void fresh_weight();	//更新权重
void update_decision();	//升级决策
void construct_decision();	//建造决策
void sell_decision();		//除旧迎新
void maintain_decision();	//维修


							/**************************各种辅助函数****************/
int abs(int x, int y) { return x > y ? x - y : y - x; }
int dist(Position a, Position b) { return abs(a.x, b.x) + abs(a.y, b.y); }
Position find_best(int building_type);


bool construct_legal(Position pos) {

}



void f_player()
{



};

void fresh_map() {

}

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
class _UpgradeAGE :public Node			//升级时代
{
public:
	int assess();
	void execute();
};
class _Resource :public Node			//获取资源 
{
public:
	int assess();

	void execute();
};
class _Development :public Node			//建造 进攻或防守 
{
public:
	int assess();
	void execute();
};

class _Attack :public Node
{
public:
	int assess();

	void execute();
};
class _Defend :public Node
{
public:
	int assess();
	void execute();
};
class _Programmar :public Node			//通过码农获取资源 
{
public:
	int assess();
	void execute();
};
class _Sell :public Node				//通过卖房子获取资源 
{
public:
	int assess();
	void execute();
};

class _BuildingNode	:public Node		//代表行为树中建筑节点部分
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_BuildingNode(BuildingType b) :buildingtype(b) {}
};
class _BuildingMethod : public Node {	//决定以何种方式加强建筑
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_BuildingMethod(BuildingType b) : buildingtype(b) {}
};
class _Construct :public Node
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_Construct(BuildingType b) :buildingtype(b) {}
};
class _Maintain :public Node
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_Maintain(BuildingType b) :buildingtype(b) {}
};
class _Upgrade :public Node
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_Upgrade(BuildingType b) :buildingtype(b) {}
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
			all_utility += children[i]->assess();
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
	float calculate_unit_threat(Soldier a);	//计算威胁值
private:
	//参数表
	float threaten_buliding[all_buildings_num] = {};				//敌方各种类型单位的威胁值
	float threaten_soldier[3][all_unit_num] = {};					//每条路上每种兵的威胁
	float weight[all_operation] = {	1, 1, 0.1, 0.1, 0.9 };			//己方各种操作权重
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

void Tree::refresh_map() {	//更新地图
	if (state->turn >= 1) {
		vector<Building>* building_set = state->building;
		vector<Building>* building_set_last = all_state.back()->building;
		for (int index = 0; index <= 1; index++) {
			for (auto iter = building_set_last[index].cbegin();
				iter != building_set_last[index].cend(); iter++) {
				if (find(building_set[index].begin(), building_set[index].end(), (*iter))
					== building_set->end())	//该建筑不在现在回合内
					map[iter->pos.x][iter->pos.y] = blank;
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

float Tree::calculate_unit_threat(Soldier a) {

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
int _BuildingMethod::assess() {
	//决定是维修、建造、还是升级 
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
