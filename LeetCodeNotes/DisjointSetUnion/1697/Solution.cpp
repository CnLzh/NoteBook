class Solution {
public:
	int find(vector<int>& uf, int x) {
		if (uf[x] == x)
			return x;
		return uf[x] = find(uf, uf[x]);
	}

	void merge(vector<int>& uf, int x, int y) {
		x = find(uf, x);
		y = find(uf, y);
		uf[y] = x;
	}

	vector<bool> distanceLimitedPathsExist(int n, vector<vector<int>>& edgeList, vector<vector<int>>& queries) {
		sort(edgeList.begin(), edgeList.end(), [](vector<int>& a, vector<int>& b) {return a[2] < b[2]; });
		vector<int>index(queries.size());
		iota(index.begin(), index.end(), 0);
		sort(index.begin(), index.end(), [&](int a, int b) {return queries[a][2] < queries[b][2]; });

		vector<bool> v(queries.size());
		vector<int> uf(n);
		iota(uf.begin(), uf.end(), 0);

		int ind = 0;
		for (auto it : index) {
			while (ind < edgeList.size() && edgeList[ind][2] < queries[it][2]) {
				merge(uf, edgeList[ind][0], edgeList[ind][1]);
				ind++;
			}
			v[it] = find(uf, queries[it][0]) == find(uf, queries[it][1]);
		}
		return v;
	}
};