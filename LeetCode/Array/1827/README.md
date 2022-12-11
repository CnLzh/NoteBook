## [1827. 最少操作使数组递增](https://leetcode.cn/problems/minimum-operations-to-make-the-array-increasing/)

## Solutions

遍历数组，若`num[n] <= num[n-1]`，则记录`res += num[n-1] - num[n] + 1`，并修改`num[n] = num[n-1] + 1`。

`res`的值即为最小操作次数。