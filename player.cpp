void init_map2() {
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			if (ts19_map[i][j] == 1) {
				map2[i][j] = 0 + 100 * (map[i][j] - road1 + 1);
				continue;
			}
		}
	}
	for (int k = 1; k <30; k++) {
		int map_temp[200][200] = { 0 };
		for (int i = 0; i < MAP_SIZE; i++) {
			for (int j = 0; j < MAP_SIZE; j++) {
				if (map_temp[i][j] == 0 && map2[i][j] != 0) {
					if (!outofRange(i, j + 1) && map2[i][j + 1] == 0) {
						map2[i][j + 1] = map2[i][j] + 1;  map_temp[i][j + 1] = 1;
					}
					if (!outofRange(i, j - 1) && map2[i][j - 1] == 0) {
						map2[i][j - 1] = map2[i][j] + 1; map_temp[i][j - 1] = 1;
						}
					if (!outofRange(i + 1, j) && map2[i + 1][j] == 0) { map2[i + 1][j] = map2[i][j] + 1; map_temp[i + 1][j] = 1; }
					if (!outofRange(i - 1, j) && map2[i - 1][j] == 0) {
						map2[i - 1][j] = map2[i][j] + 1; map_temp[i - 1][j] = 1;
					}
				}
			}

		}

	}
}
void init_map() {		//初始化地图并将路进行标号 ,第0回合不需要对建筑标记吗？ 
	if (state->turn == 0) {    //如果路不往回拐的话，似乎可以简单地解决，如果往回拐我就不会了 
		my_base_pos = state->building[ts19_flag][0].pos;
		enemy_base_pos = state->building[1 - ts19_flag][0].pos;
		//先处理和左边主基地不在同一行的路 
		for (int i = 7; i < MAP_SIZE ; i++) {      
			int road_count = 0;
			for (int j = 0; j < MAP_SIZE; j++) {
				switch (ts19_map[i][j])
				{
				case 1: {
					map[i][j] = road1 + road_count;
					if (j + 1 != MAP_SIZE && ts19_map[i][j + 1] != 1)  //考虑到同一条路可能横在同一行中的情况 
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
			int road_left = 0;  
			int i = 7;
			for (int j = 0; j <= 7; j++) if (ts19_map[i][j] == 1) road_left += 1;
			for (int i = 6; i >=0; i--) {
				int road_count = road_left;
				for (int j = 0; j <= MAP_SIZE - 1; j++) {
					switch (ts19_map[i][j])
					{
					case 1: {
						map[i][j] = road1 + road_count;
						
						if (j + 1 != MAP_SIZE && ts19_map[i][j + 1] != 1)  //考虑到同一条路可能横在同一行中的情况 
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
				if (ts19_map[i][7] == 1)  road_left++;
			}
		}

	} 
	cout << "map is OK" << endl;
	//以下为map2的初始化   
	
	/*if (state->turn == 0){
																																		int point = 0; int p = 0;
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			map2[i][j] = -1;
			//  cout << "h";
			if (ts19_map[i][j] == 1) {
				map2[i][j] = 0 + 100 * (map[i][j] - road1 + 1);
				//cout << "ij" << i << " " << j << endl;
				continue;
			}
			if (map[i][j] == blank) {
				p++;
				for (int k = 1; k <= 50; k++) {
					//cout << "k:" << k<<endl;
					tranverse_r(Position(i, j), k, paint_map2);
					if (map2[i][j] != -1) {
						point++;
						break;
					}
				}
			}
		}
		}
	cout << "p" << p << endl;
	cout << "point" << point;
	cout << "map2 is OK" << endl;
	}*/
	if (state->turn == 0) {
		init_map2();
	}
}
