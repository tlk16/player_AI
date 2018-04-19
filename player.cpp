
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
const int all_type = 28;						//�ܹ�27�ֵ�λ�뽨�����ϵз�Age
const int all_buildings_num = Building_Type;	//���н�������
const int all_unit_num = Soldier_Type;			//���е�λ����
const int all_operation = 5;		
const int max_command_num = 50;	//���غ�����ָ����
Position my_base_pos;
Position enemy_base_pos;

enum operation {			//���ֲ���
	_updateAGE,
	_construct,
	_sell,
	_maintain,
	_upgrade
};

enum map_identify {			//��ͼ��ʶ
	blank,
	my_building,
	en_building,
	road1,
	road2,
	road3,
	legal_area,
	base
};

enum _atrributes {			//������������������
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







/***********************������************************/
const double utility_weight1[Soldier_Type][_attribute] = {
	//��λ				����		��Դ		��������		����		λ��		����ӳ�		ƫ��
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
	//����				����		��Դ		��������		����		λ��		����ӳ�		ƫ��
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
	{ Ohm,				0.4,	0,		0.05,		0.5,	0,		4,			1},//���Է�����Чʱ���ӳ�����*�ӳɺ���
	{ Mole,				0.3,	0,		0.07,		0.5,	0,		2,			1},
	{ Monte_Carlo,		0.3,	0,		0.05,		0.5,	0,		0,			1},
	{ Larry_Roberts,	0.4,	0,		0.05,		0.5,	0,		3,			1},
	{ Robert_Kahn,		0.4,	0,		0.05,		0.5,	0,		1,			1},
	{ Musk,				0,		0,		0.2,		0.8,	0,		20,			1},
	{ Hawkin,			0,		0,		0.3,		0.8,	0,		30,			1},
	{ Programmer,		0,		1,		0,			0,		0.1,	0,			1}
};



/**************************���ָ�������**********************/
int abs(int x, int y) { return x > y ? x - y : y - x; }
int dist(Position a, Position b) { return abs(a.x, b.x) + abs(a.y, b.y); }
Position find_best(int building_type);	//Ѱ·����


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

float calculate_hp(Soldier a,int level) {//�ṩ�ȼ�����������Ϊ����ı�������
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

float calculate_utility(Building b) {	//	�����н���������
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


/**************************��Ϊ������****************/

class Node
{
public:
	//������Ա 
	Node();
	~Node();
	void tick(int f_power, int f_resource, int f_utility);   //����Ϊ���ڵ���䵽�Ľ���������Դ; �ֵܽڵ���Ч�� T
	virtual int assess() {
		std::cout << "assess" << endl;
	}
	virtual void execute() {
		std::cout << "execute" << endl;
	}

	//���ݳ�Ա 
	std::vector<Node*> children;
	int utility;     //�Լ���Ч��ֵ���ڵ���assess��������� 
	int max_power;   //���䵽�Ľ����� 
	int max_resource;   //���䵽����Դ 

};

/*******************************��������*******************************/
class _UpgradeAGE :public Node			//����ʱ����ÿ�غ�Ԥ�����Ȩ�ص���Դ������������������
{
public:
	int assess();
	void execute();
};
class _Resource :public Node			//��ȡ��Դ��һ�㲻��Ҫά�ޣ�����ʱ������Ҫ�������������ÿ�غ���Դ��ȡ��Ҫ��һ���������ж�
{										//���ǵз���Դ��ȡ��
public:
	int assess();
	void execute();
	int evaluate();						//��Դ��ȡĿ�����
};
class _Development :public Node			//���� ��������� 
										//���㹥���ͷ�����Ҫ�ﵽ��Ч��ֵ���ټ���ԭ�еĹ�������ƫ�ã�����ÿ����Ϊ��ֵ
{								
public:
	int assess();
	void execute();
};

class _Attack :public Node				//����Ȩֵ�������������ǶԷ��ķ�������Ч�ñ����ж�Ӧ����
{
public:
	int assess();
	void execute();
};
class _Defend :public Node				//����ÿ��·ÿ����λ����вֵ���з��䣬������Ҫ�̶ȷ��䣬ĳЩ��λ�ж�Ӧ���Ƶ�λ����Щ��λ�Ӵ�Ͷ�룬
										//����Ͱ���Ȩֵ����
{
public:
	int assess();
	void execute();
	float table[8] = {};				//��������Ȩֵ��
	void fresh_table();					//��������Ȩֵ���£����ܽ��Ľ���ȨֵΪ0��������ƽ���Ȩֵ�����������ֱ��Ч��ֵ�����ܺ͹�һ��
};
class _Programmar :public Node			//��ũ��ά�ޣ����������죬�����и�Ч�ú���������ÿ�غϻ�õ���ҪЧ��ֵ����
{										//�о���ũ����Ҫ����̫���ӣ�����ʱ������Զһ���λ�þ��У���������Ҫ����ά��
public:
	int assess();
	void execute();
};
class _Sell :public Node				//ͨ�������ӻ�ȡ��Դ
{
public:
	int assess();
	void execute();
	vector<Building> sell_list();		//��Ч��С��ĳһ����ֵ�����Զ�Ϊƽ��ֵ75%��������̭������������
										//˳���ٿ��ǵ����������϶�ʱ������Ч�ýϵ͵�һ������
};

class _BuildingNode	:public Node		//������Ϊ���н����ڵ㲿��
{										//����˵��������������Ч�õĺ������ýڵ���Ҫ�ﵽĳЧ��ֵ��
										//���⻹�гɱ�������
										//����ȫ�������õ� ÿ��������ÿ�ֲ���(f,c)
										//Ŀ�꣺ѡȡĳ�����µ�ĳ������ʹ��sum(ci)��С���ڳɱ�Ԥ���ڣ��ﵽЧ����󻯣�
										//������д���򻯰�

public:
	int assess();
	void execute();

	void attribute();				//������η��佨�������������䵽����Ľڵ㣬�ɳɱ�����

	BuildingType buildingtype;
	_BuildingNode(BuildingType b) :buildingtype(b) {}

};
/*class _BuildingMethod : public Node {	//�����Ժ��ַ�ʽ��ǿ����
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
	Position find_best_place();			//Ѱ�ҵ���ѵ�λ��������
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
	vector<Building> min_cost();	//����ά�޷������ٵģ��޶������Ľ������м�ÿ�غ�ά�����20%��Ѫ��
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
	vector<Building> min_cost();		//��������������С�����ŵĽ���
private:
	int upgrade_num = 0;
};
/***************************�ӽڵ���������*************************/


/*void Node::tick(int f_power, int f_resource, int f_utility)
{
	//����Ϊ���ڵ���䵽�Ľ���������Դ; �ֵܽڵ���Ч�� 
	max_power = f_power * utility / f_utility;
	max_resource = f_resource * utility / f_utility;  // Ӧ���и��õķ��䷽�� 
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
	Tree();	     //��������init�е��� 
	void evaluate();		//���������Ե�һ����������
	void init_map();		//��ʼ����ͼ
	void refresh_map();		//ˢ�¼�����ͼ
	void refresh_unit();	//ˢ�µ�λ������Ϣ
	void tranverse();		//����
private:
	//������
	float threaten_buliding[all_buildings_num] = {};				//�з��������͵�λ����вֵ
	float threaten_soldier[3][all_unit_num] = {};					//ÿ��·��ÿ�ֱ�����в
	//float weight[all_operation] = {	1, 1, 0.1, 0.1, 0.9 };			//�������ֲ���Ȩ��
	float building_weight[Building_Type] = {};						//���ֽ�����Ȩ�أ�������ʶ����api�ļ�
	float soldier_weight[Soldier_Type] = 
	{ };															//�Ը���ʿ���Ļ���Ȩ��
	float evaluate_table[3] = {};									//�������ڵ���������
	//ÿ�غ�����µ���Ϣ
	int map[MAP_SIZE][MAP_SIZE];							//�Զ���map����ʾ·�뽨��
	float ensoldier_num[3][all_unit_num] = {};				//ÿ��·�ϵĵз�����
};

Tree::Tree()
{
	root = new Node();

	//��һ�� 
	root->children.push_back(new _UpgradeAGE());
	root->children.push_back(new _Development());
	root->children.push_back(new _Resource());

	//�ڶ���
	root->children[1]->children.push_back(new _Attack());
	root->children[1]->children.push_back(new _Defend());
	root->children[2]->children.push_back(new _Programmar());
	root->children[2]->children.push_back(new _Sell());

	//������ ��һ��Ϊattack�ĺ��ӣ��ڶ���Ϊdefend�ĺ��ӣ������������� 
	for (int i = 1; i < 8; i++)    root->children[1]->children[0]->children.push_back(new _BuildingNode(BuildingType(i)));
	for (int i = 9; i < 16; i++)   root->children[1]->children[1]->children.push_back(new _BuildingNode(BuildingType(i)));

	//���Ĳ�  ǰ����Ϊ����������������Ϊ���ؽ�����������������ά�޻��ǽ��� 
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
	evaluate();									//����õ�һ��ֵ����ÿ���ӽڵ��execute����βд������һ���ӽڵ�ĵݹ����
	for (int i = 0; i <= 2; i++) {
		this->root->children[i]->execute();
	}
}

void Tree::refresh_map() {	//���µ�ͼ
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

void Tree::init_map() {		//��ʼ����ͼ����·���б��
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

void Tree::refresh_unit() {		//���з�unitͳ�Ʊ����㣬������ͳ��
	for (int i = 0; i <= 2; i++) 
		for (int j = 0; j <= all_unit_num - 1; j++) {
			ensoldier_num[i][j] = 0;
		}
	vector<Soldier>& soldier_set = state->soldier[1 - ts19_flag];
	for (auto iter = soldier_set.cbegin(); iter != soldier_set.cend(); iter++) {
		ensoldier_num[map[iter->pos.x][iter->pos.y] - road1][iter->soldier_name] ++;
	}
}





/*************************************�ӽڵ㷽��***************************/
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
