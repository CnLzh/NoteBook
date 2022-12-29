# 4. Template in Depth

## 4.1 Template Arguments Deduction of Function Template
函数模板的实参推导发生在名字查找(Name Lookup)之后，重载决议(Overload Resolution)之前。如果函数模板实参推导失败，那么编译器不会直接报错，而是将这个模板从函数的重载集合中无声的删掉。