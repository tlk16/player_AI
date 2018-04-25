vector<Position> _Construct::find_best_place()
{
	cout << "into bestplace" << int(this->buildingtype) << endl;
	//对于防御建筑来说 
	cout << int(Bool) << "bh" << int(Hawkin) << endl;
	if (this->buildingtype >= Bool && this->buildingtype <= Hawkin) {
		cout << "into defned" << endl;
		vector<int> load_fit = vector<int>(road_num); //判断适合在哪条路周围建造该建筑 
		for (int i = 0; i < road_num; i++)
		{
			for (int j = 0; j < all_unit_num; j++)
				load_fit[i] += ensoldier_num[i][j] * restrain_table[this->buildingtype][j];  //此处应有bug 
		}
		priority_queue<Node2> q;    //优先级队列，保存utility最高的位置 

		int w1 = 1, w2 = 1;
		int start = 150 * ts19_flag + 0 * (1 - ts19_flag);
		int end = 200 * ts19_flag + 50 * (1 - ts19_flag);
		for (int i = start; i<end; i++)    //从主基地周围开始寻找，只寻找50*50的地方 
			for (int j = start; j<end; j++) {
				if ((i<7 && j<7) || (i>192 && j>192))  continue; //不找主基地 
				if (map[i][j] == legal_area)
				{
					int distance = map2[i][j] % 100 > calculate_attackRange(this->buildingtype, state->age[ts19_flag]) ? map2[i][j] % 100 : 0; //只要不超过攻击范围，离路距离越大越好 
					utility = w1 * distance + w2 * load_fit[map2[i][j] / 100 - 1];   //map2申明在tree中，未实现 ,此处要减一吗
					Node2* temp = new Node2(); temp->utility = utility; temp->p = Position(i, j);
					q.push(*temp);
				}
			}
		cout << "hhh:" <<construct_num<< endl;
		vector<Position> best_places;
		for (int i = 0; i<this->construct_num; i++)
		{
			cout << "place:" << q.top().p.x<<" "<<q.top().p.y<< endl;
			best_places.push_back(q.top().p); q.pop();
		}
		return best_places;
	}
	//对于进攻建筑来说 
	
	if (this->buildingtype >= Shannon && this->buildingtype <= Tony_Stark) {//太麻烦了未完成，可以先注释掉用random
	cout << "into attack" << endl;
	vector<int> weakness = vector<int>(road_num);    //根据我方小兵死亡速度 判断这条路是否值得进攻，这么做可能会落后于敌方建筑情况,另外初始可能有问题
	//上回合开始时这条路上某类小兵血量 - 上回合掉血量 + 上回合生产量 =  本回合开始时这条路上某类小兵血量
	vector<Building>& building_set = state->building[ts19_flag];
	vector<int> production = vector<int>(road_num);  //话说这一步可以在refresh做 , state里面保存的是回合开始时的数据吗？
	for (int i = 0; i < road_num; i++)  production[i] = 0;

	for (int i = 0; i < building_set.size(); i++) {
		int p = max(map2[building_set[i].pos.x][building_set[i].pos.y] / 100 ,1);
	production[p - 1] += OriginalSoldierAttribute[OriginalBuildingAttribute[this->buildingtype][4]][2];//如何计算某类建筑对应的小兵血量？
	}

	for (int i = 0; i<road_num; i++)
	{
	for (int j = 0; j<all_unit_num; j++)
	weakness[i] += mysoldier_heal_last[i][this->buildingtype] - mysoldier_heal[i][this->buildingtype] + production[i];
	}

	priority_queue<Node2> q;    //优先级队列，保存utility最高的位置

	int w1 = 1, w2 = 1, w3 = 1;
	Position my_base = state->building[ts19_flag][0].pos;
	int start = 10 * ts19_flag + 0 * (1 - ts19_flag);
	int end = 200 * ts19_flag + 190 * (1 - ts19_flag);
	for (int i = 0; i<190; i++)    //从主基地周围开始寻找，只寻找190*190的地方
	for (int j = 0; j<190; j++) {
	if (i<7 && j<7)  continue; //不找主基地
	if (map[i][j] == legal_area)
	{
	int distance = map2[i][j] % 100 > calculate_attackRange(this->buildingtype, state->age[ts19_flag]) ? map2[i][j] % 100 : 0; //只要不超过攻击范围，离路距离越大越好
	utility = w1 * dist(Position(i, j), my_base) + w2 * weakness[map2[i][j] / 100-1] + w3 * distance;
	Node2* temp = new Node2(); temp->utility = utility; temp->p = Position(i, j);
	q.push(*temp);
	}
	}
	vector<Position> best_places;
	for (int i = 0; i<this->construct_num; i++)
	{
		if(!q.empty())
	best_places.push_back(q.top().p); q.pop();
	}
	cout << "here" << endl;
	return best_places;

	}


}
