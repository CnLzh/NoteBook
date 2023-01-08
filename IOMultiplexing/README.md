# I/O多路复用
在Linux系统中，实际上所有的I/O设备都被抽象为了文件的概念，磁盘、网络数据、终端、进程间通信管道等都被当做文件对待，用文件描述符(fd)来表示。

I/O多路复用是指，在一个操作里同时监听多个输入输出源，在其中一个或多个可用时返回，然后对其进行读写。其实现原理是由内核负责监控进程指定的socket文件描述符，当socket准备好数据时，通知应用进程。

我们先了解以下几个概念：

1. 多路复用：
- 多路：多个socket网络连接
- 复用：复用一个线程
- 技术：多路复用主要有三种技术，分别是`select, poll, epoll`，其中`epoll`是最新的，也是目前最好的多路复用技术

2. IO模型：
- blockingIO：阻塞IO
- monblockingIO：非阻塞IO
- IOmultiplexing：IO多路复用
- signaldrivenIO：信号驱动IO
- asynchronousIO：异步IO

3. 事件：
- 可读事件：当文件描述符关联的内核缓冲区非空，有数据可以读取，触发可读事件。
- 可写事件：当文件描述符关联的内核缓冲区不满，有空闲空间可以写入，触发可写事件。

大多数文件系统的默认IO操作都是缓存IO。在Linux的缓存IO机制中，操作系统会将IO的数据缓存在文件系统的页缓存。也就是说，数据会先拷贝到操作系统内核的缓冲区中，然后才会从操作系统内核的缓存区拷贝到应用程序的地址空间。这种做法的缺点是，需要在应用程序地址空间和内核进行多次拷贝，这些拷贝动作带来的CPU和内存的开销是非常大的。

至于为什么不能直接让磁盘控制器把数据送到应用程序的地址空间中呢？最简单的原因之一是应用程序不能直接操作底层硬件。

总的来说，IO分两个阶段：
1. 数据准备阶段
2. 内核空间复制回用户空间缓冲区阶段：

![IO Model](/IOMultiplexing/images/IOmodel.png)

## I/O多路复用之select、poll、epoll
与多进程和多线程技术相比，IO多路复用技术最大的优势是系统开销小，不必创建和维护进程/线程，从而减小了系统的开销。

I/O多路复用就是通过一种机制，一个进程可以监控多个描述符，一旦某个描述符就绪(读就绪或写就绪)，能够通知程序进行相应的读写操作。但`select, poll, epoll`本质上都是同步I/O，因为都需要在读写事件就绪后自己负责进行读写，也就是说读写过程是阻塞的；而异步I/O则无需自己读写，而是把数据从内核拷贝到用户空间。

### 1. select模型
`select`模型中的一个socket文件描述符通常可以看成一个由设备驱动程序管理的设备，驱动程序可以知道自身的数据是否可用。设备的资源如果可用(可读或可写)，则通知应用进程；反之则让应用进程睡眠，等待数据到来时，再唤醒应用进程。

多个这样的文件描述符被放在一个队列中，`select`调用的时候遍历这个队列，如果对应的文件描述符资源可用，则返回该文件描述符(应用进程的回调事件)。当遍历结束后，如果仍然没有一个可用的文件描述符，则`select`让应用进程睡眠，直到资源可用时再唤醒应用进程。所以，`select`每次遍历都是线性的。

#### C++中的select函数
`int select (int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);`

- `nfds`：最大文件描述符+1
- `readfds`：可读文件描述符集合
- `writefds`：可写文件描述符集合
- `exceptfds`：异常文件描述符集合
- `timeout`：定时阻塞监控时间，分三种情况。1. NULL，永远等待。2. 设置timeval，等待固定时间。 3. 设置timeval均为0，轮询。

`select`的工作流程为：
1. 用户态初始化`fdlist`，即socket队列。
2. 调用`select`，将`fdlist`拷贝到内核态。
3. 由内核监听`fdlist`，遍历并判断是否有读写事件发生。
4. 若有读写事件发生，返回大于0的正整数，由用户态判断哪些文件描述符可以被使用，并做对应处理。
5. 若无读写事件发生，返回0，由`timeout`决定是等待固定时间进行下一次监听或是轮询。

点击[此处](https://github.com/CnLzh/NoteBook/blob/main/IOMultiplexing/src/select/main.cpp)查看Linux下C++实现的select模型server完整示例。client可直接使用终端命令`nc -v server的IP地址 server监听的端口号`的方式进行连接，如`nc -v 127.0.0.1 9808`。

通过对`select`的逻辑过程分析，可以发现`select`存在三个问题:

1. 每次调用都需要把被监控的集合从用户态空间拷贝到内核态空间，高并发场景下这样的拷贝会消耗很多资源。
2. 能监听的端口数量有限，单个进程能打开的最大连接数有`FD_SETSIZE`的宏定义。
3. 被监控的集合中只要有一个数据可读写，整个集合就会被遍历一次，收集可读写事件。因为我们只关心是否有数据发生读写事件，当事件发生时，只能通过遍历每个socket来收集可读写事件。即被监控的集合中发生任意一个可读写事件，都需要遍历整个被监控的集合。

### 2. poll
poll的实现和select非常相似，只是对文件描述符集合的描述方式不同。针对select的三个问题，poll使用pollfd结构而不是fd_set结构，解决问题2的fd集合大小限制。但poll和select存在同样性能缺陷：包含大量文件描述符的集合被整体复制于用户态和内核态的地址空间之间，不论这些文件描述符是否就绪，造成的开销随着文件描述符的数量增加而线性增加；以及一个文件描述符就绪触发整体文件描述符集合的遍历的低效问题。因此poll也并不适用于大并发的场景。

#### C++中的poll函数
`int poll(struct pollfd *fds, unsigned int nfds, int timeout)`

- `fds`：指向pollfd结构体数组，待检测的文件描述符集合。
- `nfds`：pollfd结构体数组的个数。
- `timeout`：定时监控阻塞时间。

点击[此处](https://github.com/CnLzh/NoteBook/blob/main/IOMultiplexing/src/poll/main.cpp)查看Linux下C++实现的poll模型server完整示例。

### 3. epoll
相比于select和poll，epoll最大的好处在于不会随监听的fd数量增长而降低效率，不存在随着并发量的提高出现性能明显下降的问题。

#### C++中的epoll函数
接下来，我们从epoll的三个接口函数，来了解epoll的工作原理：

`int epoll_create(int size)`：
- 功能：该函数用于创建一个epoll对象，返回该对象的文件描述符。
- 参数：`size`用于告诉内核监听的数量有多大，但并不限制epoll所能监听的描述符最大个数，只是对内核初始化时分配内部数据结构的一个建议。
- 返回值：成功返回epoll的文件描述符，失败返回-1。

调用`epoll_create()`时，会创建一个`eventpoll`结构体的对象，其主要成员及含义如下：
- wq：等待队列链表。
- rbr：红黑树。为了支持大量链接的高效查找、删除和插入，eventpoll内部使用红黑树管理所有socket连接。
- rdllist：就绪的文件描述符双向链表。当有链接就绪时，内核把就绪连接放到rdllist链表中。用户态只需要判读链表就能找到就绪进程，不需要遍历红黑树所有节点。

`int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);`：
- 功能：epoll的事件注册函数，用于注册监听的事件。把一个socket和这个socket相关的事件添加到epoll对象。
- 参数epfd：epoll的文件描述符，`epoll_create`的返回值。
- 参数op：要做的动作(添加，修改，删除)，由三个宏来表示：
1. EPOLL_CTL_ADD：添加新的fd到epfd中，把socket节点插入到红黑树中。
2. EPOLL_CTL_MOD：修改已经注册的fd的监听时间。
3. EPOLL_CTL_DEL：删除一个fd。
- 参数fd：需要监听的文件描述符
- 参数event：需要监听的事件类型，分以下几种：
1. EPOLLIN：文件描述符可读
2. EPOLLOUT：文件描述符可写
3. EPOLLPRI：文件描述符有紧急数据可读
4. EPOLLERR：文件描述符发生错误
5. EPOLLHUP：文件描述符被挂断
6. EPOLLET：设为边缘触发模式
7. EPOLLONESHOT：只监听一次
- 返回值：成功返回0，失败返回-1。

`int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);`：
- 功能：等待事件发生。从内核态把就绪队列拷贝到用户态。
- 参数epfd：epoll的文件描述符。
- 参数events：分配好的`epoll_events`结构体数组，epoll会把事件赋值到`events`数组中，不能为空，内核只负责赋值，不负责分配内存。
- 参数maxevents：events个数，大小应和events一致。
- 参数timeout：超时时间。
- 返回值：成功返回需要处理的事件数，超时返回0，失败返回-1。

关于边缘触发(ET)和水平触发(LT)：

`select`和`poll`仅支持LT工作方式，`epoll`默认为LT的工作方式。
ET：无论事件是否处理完毕，仅触发一次。
LT：只要事件没处理完，每一次`epoll_wait`都触发该事件。

由上述三个接口，总结一下`epoll`的工作流程：

1. 用户态创建epoll文件描述符。
2. 将需要监听的文件描述符(socket)注册到epoll中，由内核监听事件。
3. 若有事件发生，返回大于0的正整数，由用户态做对应处理。
4. 若无事件发生，返回0，由timeout决定阻塞时间。

接下来，我们来探索`epoll`的内部实现原理：

- `epoll_create`：
主要用于创建`eventpoll`结构体，其中包含如下几种关键数据：
1. 红黑树：用于管理存放事件的集合，这些事件以`epitem`作为节点挂载到红黑树上，并与设备驱动建立回调关系。其主要需求为在大量并发的场景下，快速的添加、修改、删除节点。选择红黑树的数据结构解决该问题，每次通过`epoll_ctl`的任何操作的时间复杂度为O(logN)。
2. 等待队列：在调用`epoll_wait`时，若就绪队列为空，内核将进程引用放入epoll的等待队列中，阻塞进程。
3. 就绪队列：当socket事件发生时，通过相应的回调事件，将对应的`epitem`节点加入到就绪队列中。

- `epoll_ctl`：
主要用于将事件以`epitem`节点的形式挂载到红黑树上，并建立回调关系。

- `epoll_wait`：
由内核检查就绪队列的状态。若就绪队列非空，将其拷贝到用户态并返回事件数量；若就绪队列为空，将进程放入等待队列，阻塞进程。

因此，`epoll`支持大量并发的主要原因为：

1. 不用重复在内核态和用户态拷贝事件集合。
2. 事件以节点的形式挂载到红黑树上，通过`epoll_ctl`的任何操作的事件复杂度都是O(logN)。
3. `epoll_wait`仅关注就绪队列是否为空，不需要遍历事件集合。
4. 向内核中断注册回调函数，一旦有事件触发，由回调将事件对应节点添加到就绪队列中，时间复杂度仅为O(1)。

点击[此处](https://github.com/CnLzh/NoteBook/blob/main/IOMultiplexing/src/epoll/main.cpp)查看Linux下C++实现的epoll模型server完整示例。