# Redis 是怎样运作的

Redis 两大主题，

- 追求性能的极致
- 追求内存管理的极致





## Redis 在内存中是怎样存储的

reids是内存存储， 放在Redis的数据，都是以键值对形式，存放内存。

### 数据结构

```c
typedef struct redisDb {
    dict *dict;                 /* The keyspace for this DB */
    dict *expires;              /* Timeout of keys with a timeout set */
    dict *blocking_keys;        /* Keys with clients waiting for data (BLPOP)*/
    dict *ready_keys;           /* Blocked keys that received a PUSH */
    dict *watched_keys;         /* WATCHED keys for MULTI/EXEC CAS */
    int id;                     /* Database ID */
    long long avg_ttl;          /* Average TTL, just for stats */
    list *defrag_later;         /* List of key names to attempt to defrag one by one, gradually. */
} redisDb;

```

dict结构 代表 我们存入的

```c
struct dict {
    dictType *type;

    dictEntry **ht_table[2];
    unsigned long ht_used[2];
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    /* Keep small vars at end for optimal (minimal) struct padding */
    int16_t pauserehash; /* If >0 rehashing is paused (<0 indicates coding error) */
    signed char ht_size_exp[2]; /* exponent of size. (size = 1<<exp) */

};
```

redisDb 数据库对象， 指向数据字典， 字典里包含了 平常存储的k-v数据，k是字符串对象， vlaue 支持任意Redis对象

<img src="../../assets/imgs/image-20230710151107517.png" alt="image-20230710151107517" style="zoom:33%;" />

#### 添加数据

将键值对添加到 dict 结构字典中，key 必须为String 对象， Value 为 任何类型的对象

比如， `SET hellomsg "hello mart"`

<img src="../../assets/imgs/image-20230710151444243.png" alt="image-20230710151444243" style="zoom:33%;" />

#### 查询数据

在ict 找到对应的key， 即完成查询



#### 更新数据

对已经 Key 对象的任何变更操作，都是更新， 例如：针对String 对象赋予新值， 给List 对象增添元素

`RPUSH animals bird`

<img src="../../assets/imgs/image-20230710151810513.png" alt="image-20230710151810513" style="zoom:25%;" />

#### 删除数据

删除即把 Key 和Value 都从dict 结构里删除， 

`DEL scoredic` ，成功之后的结构

<img src="../../assets/imgs/image-20230710152050168.png" alt="image-20230710152050168" style="zoom:25%;" />



#### 过期键

Redis 数据都可以设置过期键， 这样 到了一定时间， 这些对象就会自动过期回收，

<img src="../../assets/imgs/image-20230710153428221.png" alt="image-20230710153428221" style="zoom:33%;" />

这里的dict中和expires 中key对象， 存储的String 对象指针， 所以不会重复占用内容。





## Redis是单线程 还是多线程



核心处理逻辑， Redis 是单线程， 辅助模块中 复制模块用的多进程。







## 单线程为什么能这么快

Redis 单线程的处理速度快的原因：

1. Redis的大部分操作都在内存上完成， 内存操作快
2. Redis 追求极致，选择了很多高效的数据结构， 并做了非常多的优化， 比如ziplist、hash、跳表
3. Redis 采用了 多路复用极致， 使其在网络IO 操作中 并发处理大量的客户端亲求，实现高吞吐量



Redis处理的流程：

Redis的服务器启动时， bind 端口， listen操作监听客户端请求， 客户端发起请求连接。

客户端发起一次处理请求， 服务端：

1. 客户端请求处理时， 使用accept 建立连接
2. 调用recv 从套接字中读取请求
3. 解析客户端发送请求，拿到参数
4. 处理请求， Get， nameRedis 通过Key获取对应的数据
5. 最后将数据通过send 发送给客户端

<img src="../../assets/imgs/image-20230710162829449.png" alt="image-20230710162829449" style="zoom:33%;" />



套接字是默认阻塞模式的， 在accept 和recv时客户端一直没有发送数据 ，Redis会发生阻塞。

非阻塞调用下，如果没有数据，不会阻塞在哪里，通过一个循环来不断轮询来检查是否已经就绪。

I/O多路复用， 就是有I/O操作触发时， 就会产生通知， 收到通知， 再去处理通知对应的时间，Redis针对I/O多路复用进行包装， 叫做Reactor模型。



<img src="../../assets/imgs/image-20230710164533424.png" alt="image-20230710164533424" style="zoom:25%;" />









## Redis处理过程

### 处理流程：

#### 监听端口，注册事件

main-> initServer 中， 使用listenToPort 监听端口， 创建监听套接字

```c
int listenToPort(connListener *sfd)
```

之后调用aeCreateFileEvent 函数绑定对应的接收处理函数acceptTcpHandler， 绑定之后等到客户端连接请求发回来，既可以关联到处理函数

#### 连接到达处理

当一个连接到达， 事件循环就能收到这个信息，监听套接字的acceptTcpHandler 函数会先生成一个客户端套接字， 不会立刻开始读客户端发过来的数据。

#### 客户端数据处理

收到一个客户端发来的数据后， readQueryFromClient



## 多线程是怎么回事





<img src="../../assets/imgs/image-20230712105711557.png" alt="image-20230712105711557" style="zoom:25%;" />

详细

![image-20230712110847604](../../assets/imgs/image-20230712110847604-9131328.png)





Redis的多线程模式 默认是关闭的， 需要用户在`redis.config` 配置文件中开启





## 内存淘汰算法-LRU

### LRU算法

最近最久未使用， 记录每个key 的最近访问时间，维护一个访问时间的数据





### Redis近似LRU算法

维护一个全局链表，对Redis 来说，开销过大

#### 算法概述

LRU中， redisObject对象中lru 字段存储的是key 被访问时Redis的时钟server.lruclock， 当key 被访问时，Redis会更新这个key的redisObject的lru字段，

近似LRU算法 在现有的数据结构的基础上，采用随机采用的方式来淘汰元素， 内存不足时，就执行一次近似LRU算法。

随机采用n个key， 根据时间戳淘汰掉 最旧的那个key；如果内存不足，继续随机采用来淘汰。



#### 采用范围

采样范围的选择范围策略，有两种1.allkeys， 所有key中随机采样，2.volatile 从有过起时间的key 随机采样。分别对应 allkeys-lru ， volatitle



#### 淘汰池优化

近似LRU， 优点在于节约了内存， 但缺点是不是一个完整的LRU， 随机采样得到的记过，不是全局真正的最久未访问。



Redis3.0 对近似LRU算法进行了优化。 维护一个大小为16的候选池， 池中数据根据访问时间进行排序， 第一次随机选取的key 都会放入 池中， 然后淘汰掉最久未访问的， 

随后 每次随机选取的key 只有活性 比 池子里 活性最小的key还小时 次放入池中，当池子满时， 如果有新的key需要放入，则将池中活性最大的key移除。

Redis 3.0 会提供一个待淘汰候选key的`pool`，里面默认有16个key，按照空闲时间排好序。更新时从`Redis`键空间随机选择N个key，分别计算它们的空闲时间`idle`，key只会在`pool`不满或者空闲时间大于`pool`里最小的时，才会进入`pool`，然后从`pool`中选择空闲时间最大的key淘汰掉。





## LFU 内存淘汰算法

LFU 淘汰算法， Least Frequently Used，最不频繁淘汰算法， 优先淘汰活跃度最低的， 使用频率最低的

```c
typedef struct redisObject{
  unsigned type:4; // Redis 对象
  unsigned encoding:4; // 底层编码， 用OBJECT ENCODING[key]  可以看到编码方式
  unsigned lru:LRU_BITS; // 记录对象访问信息， 用于内存淘汰
  
  int refcount; // 引用 计数， 描述有多少个指针指向该对象
  void *ptr; // 内容指针， 指向实际内容
}robj;
```



Redis 在LFU 策略下复用lru字段，因为该字段有 24bit， 高16bit 存储ldt， 低8bit 存储logc

<img src="../../assets/imgs/image-20230713103551629.png" alt="image-20230713103551629" style="zoom:25%;" />

高16位保存上次访问时间戳， 后8位存储的是一个访问次数

一个key是否活跃，有这两个字段决定

上一次访问时间很久， 访问次数就会衰减

每次访问，会增加访问次数

访问次数默认为5.

### lru 数据更新

1. #### 计算次数衰减

2. #### 一定概率增加反问次数

   次数不足5， 一定增加

   次数大于5次，小于255次，一定概率加1， 次数越大，越困难

   lfu-log-factor 参数影响，该参数越大 增加的难度越大

3. #### 更新

   当前时间更新到高16位， 次数更新到低8位

