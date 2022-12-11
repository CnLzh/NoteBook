## [1691. 判断一个数字是否可以表示成三的幂的和](https://leetcode.cn/problems/maximum-height-by-stacking-cuboids/)

## Solutions
对于一个长方体，将其`width,height,length`由大到小排列，再对全部长方体`width,height,length`相加的和由大到小排列。构建一个`dp`数组，`dp[n]`表示从堆叠长方体`n`可得到的最大高度。因对全部长方体由大到小进行排序，故`n-1`的三维一定至少存在一个大于`n`的三维，即`n-1`一定不可能堆叠在`n`上。因此对于每一个`dp[n]`，只需要从`dp[0] -> dp[n-1]`，若`dp[n][0] < dp[x][0] && dp[n][1] < dp[x][1] && dp[n][2] < dp[x][2]`，则记录`max(dp[x][0] + dp[n][0])`为`dp[n]`的值。取`dp`数组中的最大值，即为可堆叠的最大高度。