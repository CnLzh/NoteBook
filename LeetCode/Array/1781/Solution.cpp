class Solution {
public:
    int beautySum(string s) {
        int res = 0;
        for (int i = 0; i < s.size(); i++) {
            int v[26] = {0};
            for (int j = i; j < s.size(); j++) {
                v[s[j] - 'a']++;
                int maxnum = 0, minnum = s.size();
                for (int k = 0; k < 26; k++) {
                    if (v[k] > 0) {
                        maxnum = max(maxnum, v[k]);
                        minnum = min(minnum, v[k]);
                    }
                }
                res = res + maxnum - minnum;
            }
        }
        return res;
    }
};