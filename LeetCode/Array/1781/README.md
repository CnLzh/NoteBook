## [1781.所有子字符串美丽值之和](https://leetcode.cn/problems/sum-of-beauty-of-all-substrings/)

## Solutions

暴力搜索，遍历字符串，对每个`str[i]`，枚举所有子串，以哈希表的方式记录每个子串的所有字符出现的次数，并对累加其出现次数最大值与最小值的差`res = res + max - min`。

`res`的值即为美丽值之和。