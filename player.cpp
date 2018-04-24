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


/***********************参数表************************/
const double utility_weight1[Soldier_Type][_attribute+1] = {
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

const double utility_weight2[Building_Type][_attribute+1] = {
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

/********************类的定义和函数实现******************/

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
		//统计路的数目


	}
};

