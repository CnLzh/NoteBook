## [1780. 判断一个数字是否可以表示成三的幂的和](https://leetcode.cn/problems/check-if-number-is-a-sum-of-powers-of-three/)

## Solutions
对于整数`num`，将其由十进制转换为三进制，取最后一位，若小于等于1，则可以表示为三的幂的和。

例如:
- `5`

十进制`5`转为三进制`12`，`2 > 1`不为三的幂的和。
- `7`

十进制`7`转为三进制`21`，`1 = 1`为三的幂的和。
