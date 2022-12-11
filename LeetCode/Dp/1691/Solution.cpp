class Solution {
public:
	int maxHeight(vector<vector<int>>& cuboids) {

		for(auto& it : cuboids)
			std::sort(it.begin(), it.end(), [](int a, int b) {return a > b; });

		std::sort(cuboids.begin(), cuboids.end(), [](vector<int>& a, vector<int>& b) {
			return a[0] + a[1] + a[2] > b[0] + b[1] + b[2];
			});

		vector<int>v;
		int max_height = 0;
		for (size_t i = 0; i < cuboids.size(); i++) {
			int tmp_max = 0;
			for (size_t j = 0; j < i; j++) {
				if (cuboids[i][0] <= cuboids[j][0] && cuboids[i][1] <= cuboids[j][1] && cuboids[i][2] <= cuboids[j][2]) {
					if (v[j] > tmp_max)
						tmp_max = v[j];
				}
			}
			int max = tmp_max + cuboids[i][0];
			v.push_back(max);
			if (max > max_height)
				max_height = max;
		}
		return max_height;
	}
};