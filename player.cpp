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

/**********************�Զ������*************************/
const int all_type = 28;	//�ܹ�27�ֵ�λ�뽨�����ϵз�Age
const int all_operation = 5;
const int max_command_num = 50;	//���غ�����ָ����

enum operation {			//���ֲ���
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
int map[MAP_SIZE][MAP_SIZE];	//�Զ����ͼ����ʾ���н���λ��
vector<Building> building_last;	//��һ�غ����н���
vector<Building>* building = state->building;
/***********************������************************/
float threaten_point[all_type] = {};	//�ط��������͵�λ����вֵ
float weight[all_operation] = {
	1, 1, 0.1, 0.1, 0.9};			//�������ֲ���Ȩ��
float building_weight[Building_Type] = {};		//���ֽ�����Ȩ�أ�������ʶ����api�ļ�
float soldier_weight[Soldier_Type] = {};		//�Ը���ʿ����Ȩ��


/*****************************��Ҫ���̺���***************/
void init();
void fresh_map();	//�����Լ��ĵ�ͼ����������
void fresh_threaten();	//������вֵ
void fresh_weight();	//����Ȩ��
void update_decision();	//��������
void construct_decision();	//�������
void sell_decision();		//����ӭ��
void maintain_decision();	//ά��


/**************************���ָ�������****************/
int abs(int x, int y) { return x > y ? x - y : y - x; }
int dist(Position a, Position b) { return abs(a.x, b.x) + abs(a.y, b.y); }


bool construct_legal(Position pos) {

}



void f_player()
{
    


};

void fresh_map() {
	
}