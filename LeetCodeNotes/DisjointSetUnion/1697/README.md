## [1697. 检查边长度限制的路径是否存在](https://leetcode.cn/problems/checking-existence-of-edge-length-limited-paths/description/)

## Solutions

使用[并查集](https://oi-wiki.org/ds/dsu/)解决。
对给定的`edgeList `无向图边集和`requries`查询集，以两点间距离从小到大排序，遍历`requries`，对每个`limit`查询，依次将`edgeList`中小于`limit`的边加入到并查集中，然后使用并查集查询点`p`和点`q`是否属于同一集合。若`p`与`q`属于同一集合，则说明存在从`p`到`q`的路径，且这条路径上的每一条边的长度都小于`limit`，查询返回`true`，否则返回`fasle`。

因`requries`经过排序后是非递减的，显然上次查询的并查集中的边都满足当前查询的要求，对于下次查询，只需要将剩余的长度小于`limit`的边加入并查集中即可。