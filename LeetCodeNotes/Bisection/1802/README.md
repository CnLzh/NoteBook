## [1802. 有界数组中指定下标处的最大值](https://leetcode.cn/problems/maximum-value-at-a-given-index-in-a-bounded-array/)

## Solutions
使用二分法解决。

从`nums[index]`开始，向左边界和右边界，下标每相差1，元素和就减少1，直到达到边界或减少到仅为1后保持1不变。

根据这个思路，`nums[index]`确定后，数组的和`numsSum`也就确定了，且`nums[index]`越大，`numsSum`就越大。故可以使用二分法找到最大的使`numsSum <= maxSum`成立的`nums[index]`。

二分的最大最小边界分别为`1`，`maxSum`。`numsSum`由三部分组成，`nums[index]`、左边部分、右边部分之和，需要考虑边界元素是否早已下降到1的情况。