
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

enum operation {			//���ֲ���
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
int map[MAP_SIZE][MAP_SIZE];	//�Զ����ͼ����ʾ���н���λ��
vector<Building> building_last;	//��һ�غ����н���
vector<Building>* building = state->building;
/***********************������************************/



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
Position find_best(int building_type);


bool construct_legal(Position pos) {

}



void f_player()
{



};

void fresh_map() {

}

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
class _UpgradeAGE :public Node			//����ʱ��
{
public:
	int assess();
	void execute();
};
class _Resource :public Node			//��ȡ��Դ 
{
public:
	int assess();

	void execute();
};
class _Development :public Node			//���� ��������� 
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
class _Programmar :public Node			//ͨ����ũ��ȡ��Դ 
{
public:
	int assess();
	void execute();
};
class _Sell :public Node				//ͨ�������ӻ�ȡ��Դ 
{
public:
	int assess();
	void execute();
};

class _BuildingNode	:public Node		//������Ϊ���н����ڵ㲿��
{
public:
	int assess();
	void execute();
	BuildingType buildingtype;
	_BuildingNode(BuildingType b) :buildingtype(b) {}
};
class _BuildingMethod : public Node {	//�����Ժ��ַ�ʽ��ǿ����
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
/***************************�ӽڵ���������*************************/


void Node::tick(int f_power, int f_resource, int f_utility)
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

}

class Tree
{
public:
	Node * root;
	Tree();	     //��������init�е��� 
	void evaluate();		//���������Ե�һ����������
	void init_map();		//��ʼ����ͼ
	void refresh_map();		//ˢ�¼�����ͼ
	void refresh_unit();	//ˢ�µ�λ������Ϣ
	float calculate_unit_threat(Soldier a);	//������вֵ
private:
	//������
	float threaten_buliding[all_buildings_num] = {};				//�з��������͵�λ����вֵ
	float threaten_soldier[3][all_unit_num] = {};					//ÿ��·��ÿ�ֱ�����в
	float weight[all_operation] = {	1, 1, 0.1, 0.1, 0.9 };			//�������ֲ���Ȩ��
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

void Tree::refresh_map() {	//���µ�ͼ
	if (state->turn >= 1) {
		vector<Building>* building_set = state->building;
		vector<Building>* building_set_last = all_state.back()->building;
		for (int index = 0; index <= 1; index++) {
			for (auto iter = building_set_last[index].cbegin();
				iter != building_set_last[index].cend(); iter++) {
				if (find(building_set[index].begin(), building_set[index].end(), (*iter))
					== building_set->end())	//�ý����������ڻغ���
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

void Tree::init_map() {		//��ʼ����ͼ����·���б��
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

float Tree::calculate_unit_threat(Soldier a) {

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
int _BuildingMethod::assess() {
	//������ά�ޡ����졢�������� 
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
