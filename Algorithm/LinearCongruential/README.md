# 线性同余

线性同余是一个产生伪随机数的算法。

## 基本概念

线性同余是根据以下的递推关系式：

$$X_{n+1} \equiv (aX_n + c) \ (mod \ m)$$

其中 $a,c,m$ 是产生器设定的常数。

通过线性同余方法构建的伪随机数，当种子不变时，得到的伪随机数也不变。故通常取当前时间作为种子 $X_n$ 的值。

常见的线性同余生成器常数取值为， $a$ 取 $16807$ 或 $48271$ ， $c$ 取 $0$ ， $m$ 取32位有符号二进制整数的最大正值 $2147483647$ 。

点击[此处](https://github.com/CnLzh/NoteBook/tree/main/Algorithm/LinearCongruential/src/main.cc)查看`C++`版本简单实现。
