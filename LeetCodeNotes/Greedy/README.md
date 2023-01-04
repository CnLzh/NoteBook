## [1785. 构成特定和需要添加的最少元素](https://leetcode.cn/problems/minimum-elements-to-add-to-form-a-given-sum/)

## Solutions

对整数数组`nums`求和得到`sum`，取`diff = abs(goal - sum)`得到`sum`与目标值的差值。`diff / limit + (diff % limit != 0)`即为需要使用多少个不超过`limit`的数字来从凑齐`diff`。