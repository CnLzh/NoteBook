class Solution {
public:
	long sum(long flg, int cnt) {
		if (cnt >= flg)
			return (1 + flg) * flg / 2 + cnt - flg;
		return (flg + flg - cnt + 1) * cnt / 2;
	}
	int maxValue(int n, int index, int maxSum) {
		int left = 1, right = maxSum;
		while (left < right) {
			long mid = (left + right + 1) / 2;
			if (sum(mid, index + 1) + sum(mid, n - index) - mid > maxSum) 
				right = mid - 1;
			else 
				left = mid;
		}
		return right;
	}
};