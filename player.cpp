vector<Position> _Construct::find_soldier_pos(vector<Position> &b_pos) {
	cout << "find soldier " << endl;
	vector<Position> s_pos;
	if (buildingtype >= Bool && buildingtype <= Hawkin) {
		for (int i = 0; i < b_pos.size(); i++) {
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
	cout << " soldied done" << endl;
}
