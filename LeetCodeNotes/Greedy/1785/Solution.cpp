class Solution {
public:
	int minElements(vector<int>& nums, int limit, int goal) {
		long long sum = accumulate(nums.begin(), nums.end(), long(0));
		long long n = abs(goal - sum);
		return n / limit + (n % limit != 0);
	}
};