class Solution {
public:
    bool checkIfPangram(string sentence) {
        int p = 0;
        for (auto it: sentence) {
            p |= 1 << (it - 'a');
        }
        return p == (1 << 26) - 1;
    }
};