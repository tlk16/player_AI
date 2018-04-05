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
const int all_type = 28;	//总共27种单位与建筑加上敌方Age
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
	my_building,
	en_building,
	road,
	legal_area
};

int programmer_num = 0;
int defending_num = 0;
int producing_num = 0;
int map[MAP_SIZE][MAP_SIZE];	//自定义地图，显示所有建筑位置
vector<Building> building_last;	//上一回合所有建筑
vector<Building>* building = state->building;
/***********************参数表************************/
float threaten_point[all_type] = {};	//地方各种类型单位的威胁值
float weight[all_operation] = {
	1, 1, 0.1, 0.1, 0.9};			//己方各种操作权重
float building_weight[Building_Type] = {};		//各种建筑的权重，建筑标识参照api文件
float soldier_weight[Soldier_Type] = {};		//对各种士兵的权重


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


bool construct_legal(Position pos) {

}



void f_player()
{
    


};

void fresh_map() {
	
}