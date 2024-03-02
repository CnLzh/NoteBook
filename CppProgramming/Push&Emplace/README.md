# push 和 emplace的区别

两者都用于向`STL`中的某些数据结构插入数据，功能相同，但还有一些区别。

## 区别1

若插入的数据类型的构造函数接受多个参数，`push`只接受该类型的实例，而`emplace`不仅接受还类型的实例，还能接受该类型构造函数的参数。例如:

```cpp
class ExampleA {
public:
    ExampleA(int n, string s)
        :num(n), str(s) {
    }

private:
    int num;
    string str;
};

int main()
{
    queue<ExampleA> q;

    // 正确 构造一个ExampleA 插入到queue中
    q.push(ExampleA(1,"a"));

    // 正确 构造一个ExampleA 插入到queue中 
    q.emplace(ExampleA(1, "a"));

    // 错误 无法编译
    q.push(2, "b");

    // 正确 构造一个ExampleA 插入到queue中
    q.emplace(2, "b");

    return 0;
}
```

## 区别2:

`push`和`emplace`在插入数据时使用不同的方式，我们分左值和右值讨论。

#### 左值

两者均会以调用拷贝构造函数的方式将参数插入到容器的存储空间中。例如:

```cpp
class ExampleA {
public:
    ExampleA(int n)
        :num(n) {
        cout << "构造函数" << endl;
    }

    ExampleA(const ExampleA& n)
        :num(n.num) {
        cout << "拷贝构造函数" << endl;
    }

    ExampleA(const ExampleA&& n) noexcept
        :num(n.num) {
        cout << "移动构造函数" << endl;
    }

    ~ExampleA() {
        cout << "析构函数" << endl;
    }

private:
    int num;
};

int main()
{
    queue<ExampleA> q;
    ExampleA ex(1);

    cout << "push:" << endl;
    q.push(ex);

    cout << "emplace:" << endl;
    q.emplace(ex);

    cout << "return" << endl;

    return 0;
}
```

运行结果为:

```
构造函数
push:
拷贝构造函数
emplace:
拷贝构造函数
return
析构函数
析构函数
析构函数
```

#### 右值

`push`会调用构造函数创建一个临时对象，再通过移动构造函数将其所有权转移到容器的存储空间中，然后释放临时对象。而`emplace`则是直接在容器的存储空间中构造一个新的对象，节省了移动和析构的过程。例如:

```cpp
class ExampleA {
public:
    ExampleA(int n)
        :num(n) {
        cout << "构造函数" << endl;
    }

    ExampleA(const ExampleA& n)
        :num(n.num) {
        cout << "拷贝构造函数" << endl;
    }

    ExampleA(const ExampleA&& n) noexcept
        :num(n.num) {
        cout << "移动构造函数" << endl;
    }

    ~ExampleA() {
        cout << "析构函数" << endl;
    }

private:
    int num;
};

int main()
{
    queue<ExampleA> q;

    cout << "push:" << endl;
    q.push(1);

    cout << "emplace:" << endl;
    q.emplace(2);

    cout << "return" << endl;

    return 0;
}
```

运行结果为:

```
push:
构造函数
移动构造函数
析构函数
emplace:
构造函数
return
析构函数
析构函数
```

## 结论

`push`和`emplace`均能接受某类型的实例。但`push`仅能接受只有一个参数的构造方式，而`emplace`可以接受参数列表构造方式。

`push`和`emplace`在插入左值时没有区别。但在插入右值时`push`需构造临时对象后移动到容器存储空间中，而`emplace`则直接在容器存储空间中构造，效率更高。

注：
`push_back`和`emplace_back`与`push`和`emplace`同理。