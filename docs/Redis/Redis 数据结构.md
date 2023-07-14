## Redis 对象



## Redis Object

Redis 是 key-value 存储， key 和 value 在Redis 都被抽象为对象，key 只能是 String 对象， 而value 支持丰富的对象，包括String、 List、Set、Hash、Sorted Set、 Stream。



### Object的定义

```c
#define LRU_BITS 24

typedef struct redisObject{
  
  unsigned type:4; // Redis 对象
  unsigned encoding:4; // 底层编码， 用OBJECT ENCODING[key]  可以看到编码方式
  unsigned lru:LRU_BITS; // 记录对象访问信息， 用于内存淘汰
  
  int refcount; // 引用 计数， 描述有多少个指针指向该对象
  void *ptr; // 内容指针， 指向实际内容
}robj;
```

### 对象与数据结构

实际操作的有6个Redis 对象， 底层依赖 字符串、跳表、 哈希表、压缩列表、双端列表。





## String

字符串，Redis 中基本的数据对象， 最大为512MB， 可以通过 proto-max-bulk-len 进行修改。

### 场景

存字节数据、文本数据、序列化后的对象数据等。

**创建**， 即 产生一个字符串对象数据， 可以用SET、SETNX。

**查询**操作 可以用GET， 如果想获取多个，可以用MGET。

**更新**：用SET 更新

**删除**： 针对String 对象 本身的销毁， DEL命令



<img src="../../assets/imgs/image-20230704201215296.png" alt="image-20230704201215296" style="zoom: 20%;" />



### 写操作

- #### SET

  语法：`SET key value`

  功能： 设置一个key的值 为特定的value， 成功返回OK

  ```
  SET strniuniu cat
  ```

  Set 的扩展参数：

  - EX second： 设置键 的 过期时间
  - PX millisecond：设置键 过期时间 为多少毫秒
  - NX： 只有键不存在， 才对键 进行设置操作。 SET key value NX 等同于 SETNX key value
  - XX： 只在键已经存在时， 才对键进行设置操作

  

- #### SETNX

  语法：SETNX key value

  功能：用于在指定的key 不存在时，为key 设置 指定的值， 返回值0 表示key 存在不做操作，1表示成功

  如果对 存在的key 调用SETNX

  ```
  SETNX strniuniu fish
  (integer) 0
  ```

  如果对不存在的key 调用SETNX

  ```
  SETNX strmart fish
  (integer) 1
  ```

  

- #### DEL

  语法：DEL key [key...]

  功能： 删除对象， 返回值为 删除成功了几行

  ```
  DEL strmart1
  (integer) 1
  ```

  

### 读操作

- #### GET

  语法：GET key

  功能： 查询某个key， 存在就返回对应的value， 如果不存在 返回nil

  ```
  get strniuniu
  "fish"
  127.0.0.1:6379> get strmart
  (nil)
  ```

  

- #### MGET

  语法： MGET key [key...]

  功能： 依次查询多个key， 如果某个key 不存在对应位置返回nil

  ```
  127.0.0.1:6379> MGET strmart strniuniu
  1) (nil)
  2) "fish"
  ```

## 底层实现

#### 三种编码方式

String 的三种编码方式，

<img src="../../assets/imgs/image-20230704220747286.png" alt="image-20230704220747286" style="zoom:23%;" />

`INT编码`： 存一个 整型， 可以用long 表示的整数以这种编码存储；

`EMBSTR编码`： 字符串小于 阈值字节， 使用EMBSTR 编码

`RAW编码`： 字符串大于阈值字节， 则用RAW编码

阈值字节， 在3.2版本之后是44字节。





EMBSTR 和 RAW 都是由 redisObject 和 SDS 两个结构组成

EMBSTR下 redisObject 和 SDS是连续的内存。 

<img src="../../assets/imgs/image-20230706104841838.png" alt="image-20230706104841838" style="zoom:33%;" />

- **优点**

  redisObject和 SDS 两个结构 一次性分配空间

- **缺点**

  如果重新分配空间， 整体都需要再分配， 所以 RMBSTR 设计为只读， 任何写操作 之后 EMBSTR 都会变成RAW。

RAW下 redisObject 和 SDS的内存是分开的。

<img src="../../assets/imgs/image-20230706104902156.png" alt="image-20230706104902156" style="zoom:33%;" />

编码的转换：

INT -> RAW ： 当存的内容不再是整数，或者大小超过了long

EMBSTR -> RAW ： 任何写操作之后 EMBSTR 都会变成RAW





SDS ， simple Dynamic String， 简单动态字符串， Redis内部 作为基石的字符串封装。



#### 为什么要用SDS

在C语言中， 字符串用一个‘\0’ 结尾的char 数组表示。

比如"hello niuniu" 即 "'hello niuniu\0".

这样可能会导致：

1. 每次计算字符串长度的复杂度为O(N)
2. 对字符串进行追加，需要重新分配内存
3. 非二进制安全

在Redis内部， 字符串的追加和长度计算 很常见， 所以Rediscover封装了一个叫SDS的字符串结构，用于解决上述问题。



Redis中SDS分为 sdshdr8、 sdshdr16、 sdshdr32、 sdshdr64， 字段属性都一致， 区别是应对不同大小的字符串。

下面是sdshdr8

```c
struct __attribute__ ((__packed__)) sdshdr8 {
    uint8_t len; /* used 表示使用了多少 */
    uint8_t alloc; /* excluding the header and null terminator  表示一共分配了多少内存 ， alloc - len 就是预留空间的大小 */
    unsigned char flags; /* 3 lsb of type, 5 unused bits 标记是哪个分类 */
    char buf[];
};

//flags 表示分类
#define SDS_TYPE_8 1
```



<img src="../../assets/imgs/image-20230706152708945.png" alt="image-20230706152708945" style="zoom: 33%;" />

1. 增加长度字段len，快速返回长度
2. 增加空域空间（alloc-len）， 为后续追加数据留余地
3. 不再以'\0' 作为判断标准， 二进制安全

SDS 的预留空间的规则：

1. len小于1M 的情况， alloc = 2倍* len， 即预留len大小的空间
2. len 大于1M的情况， alloc 是 1M + len， 即预留1M大小的空间

预留空间为 min( len, 1M)



## List



### 是什么

是一组连接起来的字符串集合



### 有什么限制

List 最大元素个数是 2^32 -1（4294967295）， 新版本是 2^64 -1 



### 使用场景

List 作为一个列表存储， 属于 底层的数据结构， 比如： 存储一批 任务数据， 存储一批 信息。

### 常用操作

创建 、查询、更新和删除。

创建 即 产生一个List 对象， 一般用 LPUSH、 RPUSH， 分别从左侧进队列 和 右侧进队列

查询，使用LRANG 进行范围查询， 用LLEN 查询元素个数

更新， 对列表对象进行追加、删除元素，有 LPUSH、LPOP、RPOP、RPUSH、LREM等

删除，对List 对象本身的生成和销毁，使用DEL命令进行对象删除，版本4.0之后引入UNLINK 命令进行对象删除



### 写操作

#### LPUSH

语法： LPUSH key value [value...]

功能： 从头部增加元素， 返回值为List中元素的总数

```shell
127.0.0.1:6379> LPUSH listniuniu s1 s2 s4
(integer) 3
127.0.0.1:6379> LPUSH listniuniu s4
(integer) 4
```

从左边List头部 进行插入。

<img src="../../assets/imgs/image-20230707094525492.png" alt="image-20230707094525492" style="zoom: 25%;" />

#### RPUSH

语法： RPUSH key value [value...]

功能： 从尾部增加元素， 返回值为List中元素的总数

```shell
127.0.0.1:6379> RPUSH listniuniu s5
(integer) 5
```

从右边List尾部 进行插入。

<img src="../../assets/imgs/image-20230707094824667.png" alt="image-20230707094824667" style="zoom: 25%;" />

#### LPOP

语法： LPOP key

功能： 从头部删除元素， 返回值为List中删除的元素

```shell
127.0.0.1:6379> LPOP listniuniu
"s4"
```

从左边List头部 进行删除元素。

<img src="../../assets/imgs/image-20230707095206362.png" alt="image-20230707095206362" style="zoom:25%;" />

#### RPOP

语法： RPOP key

功能： 从list尾部删除元素， 返回值为List中删除的元素

```shell
127.0.0.1:6379> RPOP listniuniu
"s5"
```

从右边List的尾部 进行删除元素。

<img src="../../assets/imgs/image-20230707095307315.png" alt="image-20230707095307315" style="zoom:25%;" />



#### LREM

语法： LREM key count value

功能：移除 等于value 的元素， 当 count=0，则移除所有等于value的元素。 当count >0， 则从左到右开始移除count， 当count<0， 则从右到左移除count哥。返回值为被移除元素的数量。

```shell
127.0.0.1:6379> LREM listniuniu 0 s1
(integer) 1
```



#### DEL

语法： DEL key [key...]

功能：删除对象， 返回值为删除成功了几个键

```shell
127.0.0.1:6379> DEL listniuniu
(integer) 1
```



#### UNLINK

语法： ULINK key [key...]

功能：删除对象， 返回值为删除成功了几个键， 和DEL的区别：

del 命令同步删除命令， 回阻塞客户端，直到删除完成， unlink 命令 是 异步删除命令， 只是取消key在键空间的关联，让其不被查到，删除是异步进行，不回阻塞客户端。

```shell
127.0.0.1:6379> LPUSH listniuniu s1 s2 s3
(integer) 3
127.0.0.1:6379> UNLINK listniuniu
(integer) 1
```

### 读操作

<img src="../../assets/imgs/image-20230707101232912.png" alt="image-20230707101232912" style="zoom:25%;" />



#### LLEN

语法： LLEN key

功能：查看List 的长度， 即 List 中元素的总数

```shell
127.0.0.1:6379> LLEN listniuniu
(integer) 3
```



#### LRANGE

语法： LRANGE key start stop

功能：查看List 从start 到stop 为角标的元素

```shell
127.0.0.1:6379> LRANGE listniuniu 0 2
1) "s3"
2) "s2"
3) "s1"
```

start、 stop 为 负数， 表示倒数第几个元素

```shell
127.0.0.1:6379> LRANGE listniuniu -2 -1
1) "s2"
2) "s1"
```

### 底层实现

#### 编码方式

3.2版本前， List对象有两种编码方式，一种ZIPLIST， 另一种LINKEDLIST。

<img src="../../assets/imgs/image-20230707102839456.png" alt="image-20230707102839456" style="zoom:25%;" />

使用ZIPLIST需要满足：

1. 列表对象保存的所有字符串对象长度都小于64字节
2. 列表对象元素个数小于512个， 这是LIST的限制，不是ZIPLIST的限制

ZIPLIST 底层用 压缩列表实现， ZIPLIST 编码如下：

<img src="../../assets/imgs/image-20230707103707008.png" alt="image-20230707103707008" style="zoom:25%;" />



ZIPLAIST 内存排列得很紧凑， 可以有效节约内存空间。

如果不满足ZIPLIST编码的条件， 则使用LINKEDLIST编码。

LINKEDLIST 编码如下：

<img src="../../assets/imgs/image-20230707104145199.png" alt="image-20230707104145199" style="zoom:25%;" />

数据是以链表的形式链接得， 删除更为灵活，但内存不如ZIPLIST 紧凑， 所以只有在列表个数或节点数据长度较大的时候才使用LINKEDLIST编码。



#### QUICKLIST

ZIPLIST 在数据较少时节约内存， LINKEDLIST 为了数据多时提高更新效率， ZIPLIST 数据稍微多时 插入数据会导致内存复制。

3.2版本之后引入QUICKLIST，是ZIPLIST和LINKEDLIST的结合体

<img src="../../assets/imgs/image-20230707104725332.png" alt="image-20230707104725332" style="zoom:25%;" />



LINKEDLIST 单个节点存一个ZIPLIST，即多个数据

<img src="../../assets/imgs/image-20230707104801220.png" alt="image-20230707104801220" style="zoom: 33%;" />

当数据少时，QUICKLIST 就一个节点，

数据多时，同时利用ZIPLIST 和 LINKEDLIST的优势。



7.0版本之后 LISTPACK 的编码模式 取代了ZIPLIST







## Set

Redis 的Set 是一个不重复、无序的字符串集合，

适合无序集合场景。Set还提供了查交集、并集的功能。

### 常规操作

创建、查询、更新、删除等基本操作

创建即 产生一个Set对象， 使用SADD 创建

Set的查询，SISMEMBR可以查询元素是否存在， SCARD、SMEMBERS、SSCAN 可以查询集合元素数据； SINTER、 SUNION、SDIFF可以对集合查交集并集和差异。

更新，使用SADD增加元素， SREM 删除元素

DEL 删除Set对象

<img src="../../assets/imgs/image-20230707112933919.png" alt="image-20230707112933919" style="zoom:25%;" />

#### 写操作

##### SADD

语法： SADD key member [member...]

功能：添加元素，返回值为成功添加了几个元素

```shell
127.0.0.1:6379> SADD setniuniu aa bb cc
(integer) 3
```

<img src="../../assets/imgs/image-20230707140812581.png" alt="image-20230707140812581" style="zoom:25%;" />

```shell
127.0.0.1:6379> SADD setniuniu 11 22  33
(integer) 3
```

<img src="../../assets/imgs/image-20230707140906937.png" alt="image-20230707140906937" style="zoom:25%;" />

##### SREM

语法： SREM key member [member...]

功能：删除元素，返回值为成功删除了几个元素

```shell
127.0.0.1:6379> SREM setniuniu 33
(integer) 1
```

<img src="../../assets/imgs/image-20230707141033015.png" alt="image-20230707141033015" style="zoom:25%;" />

#### 读操作

<img src="../../assets/imgs/image-20230707144950041.png" alt="image-20230707144950041" style="zoom:25%;" />

##### SISMEMBER

语法： SISMEMBER key member 

功能：查询元素是否存在

```shell
127.0.0.1:6379> SISMEMBER setniuniu 11
(integer) 1
```



##### SCARD

语法： SCARD key 

功能：查看集合的元素个数

```shell
127.0.0.1:6379> SCARD setniuniu
(integer) 4
```



##### SMEMBERS

语法： SMEMBERS key 

功能：查看集合所有元素

```shell
127.0.0.1:6379> SMEMBERS setniuniu
1) "bb"
2) "cc"
3) "aa"
4) "11"
```



##### SSCAN

语法： SSCAN key cursor [MATCH pattern] [COUNT count] 

功能： 查看集合元素， 可以理解为 指定游标 进行查询， 可以指定个数，默认为10

从0号位置开始查询，默认10个

```shell
127.0.0.1:6379> SSCAN setniuniu 0
1) "0"
2) 1) "bb"
   2) "cc"
   3) "11"
   4) "aa"
```

例2: 使用MATCH 模糊查询

```
127.0.0.1:6379> SSCAN setniuniu 0 MATCH 1*
1) "0"
2) 1) "11"
127.0.0.1:6379> SSCAN setniuniu 0 MATCH a*
1) "0"
2) 1) "aa"
```

##### SINTER

语法： SINTER key [key...] 

功能：返回在第一个集合里，同时在后面所有集合都存在的元素(交集)

<img src="../../assets/imgs/image-20230707160421012.png" alt="image-20230707160421012" style="zoom:25%;" />

```shell
127.0.0.1:6379> SINTER sets setniuniu
1) "33"
2) "22"
3) "11"
4) "44"
```



##### SUNION

语法： SUNION key [key...] 

功能：返回所有集合的并集，集合个数大于等于2

```shell
127.0.0.1:6379> SUNION sets setniuniu
 1) "cc"
 2) "bb"
 3) "35"
 4) "34"
 5) "12"
 6) "44"
 7) "33"
 8) "45"
 9) "25"
10) "22"
11) "aa"
12) "23"
13) "14"
14) "11"
15) "24"
16) "13"
```

##### SDIFF

语法： SDIFF key [key...] 

功能：返回第一个集合有， 且在后续集合中不存在的元素， 集合个数大于等于2，

```shell
127.0.0.1:6379> SDIFF sets setniuniu
1) "12"
2) "13"
3) "14"
4) "23"
5) "24"
6) "25"
7) "34"
8) "35"
9) "45"
```

### 底层实现

#### 编码方式

<img src="../../assets/imgs/image-20230707160758362.png" alt="image-20230707160758362" style="zoom:25%;" />

Redis 出于性能和内存的考虑，支持两种编码， 如果Set元素都是整数， 且元素数量不超过512个，使用INTSET编码， 排列紧凑，内存占用少， 查询时需要二分查询

<img src="../../assets/imgs/image-20230707161107797.png" alt="image-20230707161107797" style="zoom:33%;" />

不满足INTSET编码， 使用HASHTABLE结构， HASHTABLE 查询一个元素的性能很高，O(1) 就能找到一个元素是否存在。

<img src="../../assets/imgs/image-20230707162228942.png" alt="image-20230707162228942" style="zoom:33%;" />

Set 可以高效的管理无序集合， 为多个集合求交并补集，底层编码有INSTSET 和 HASHTABLE 两种， INTSET 对应少量整数集合介于内存， HASHTABLE 适用于快速定位某个元素。



## Hash

Hash 是一个 field、 value 都为string 的hash表， 存储在Redis的内存中， 可以存2^32-1 个键值对

适用于O(1) 时间字典查找某个field 对应数据的场景，

### 常用操作

创建、查询、更新、删除等基本操作 

创建即 产生一个Hash对象， 使用HSET、 HSETNX 创建

查询： 支持HGET 查询单个元素， HGETALL 查询所有元素， HLEN 查询数据总数， HSCAN 进行游标迭代查询

更新，HSET 可用于增加新元素，HDEL 删除元素

DEL 删除Hash对象

<img src="../../assets/imgs/image-20230707163425947.png" alt="image-20230707163425947" style="zoom:25%;" />

#### 写操作

##### HSET

语法： HSET key field value

功能：为集合对应field设置value数据。

```shell
127.0.0.1:6379> HSET hashniuniu v1 f1 v2 f2 v3 f3 v4 f4
(integer) 4
```



##### HSETNX

语法： HSETNX key field value

功能：如果field不存在， 为集合对应field设置value数据。

```shell
127.0.0.1:6379> HSETNX  hashniuniu v1 newf1
(integer) 0
127.0.0.1:6379> HSETNX  hashniuniu v5 f5
(integer) 1
127.0.0.1:6379> HSETNX  hashniuniu v6 f6
(integer) 1
127.0.0.1:6379> HSETNX  hashniuniu v7 f6
(integer) 1
127.0.0.1:6379> HSETNX  hashniuniu v7 f7
(integer) 0
```

##### HDEL

语法： HDEL key field 

功能：删除指定field，可以一次删除多个

```shell
127.0.0.1:6379> HDEL hashniuniu v7 v5
(integer) 2
```

##### DEL

语法： DEL key [key...]

功能：删除Hash 对象

```shell
127.0.0.1:6379> DEL hashniuniu
(integer) 1
```

##### HMSET

语法： HMSET key field value [field value]

功能：可以设置多个键值对， 4.0 之后 HMSET 弃用

#### 读操作

##### HGETALL

语法： HGETALL  key 

功能：查找全部数据

```shell
127.0.0.1:6379> HGETALL hashniuniu
1) "v1"
2) "f1"
3) "v2"
4) "f2"
5) "v3"
6) "f3"
7) "v4"
8) "f4"
```

##### HGET

语法： HGETALL  key  field

功能：查找某个key

```shell
127.0.0.1:6379> HGET hashniuniu v1
"f1"
```



##### HLEN

语法： HLEN  key 

功能：查找Hash中元素总数

```shell
127.0.0.1:6379> HLEN hashniuniu
(integer) 4
```

##### HSCAN

语法： HLEN  key  cursor [Match pattern] [COUNT count]

功能：从指定位置查询一定数量的数据， 如果小数据量， 处于ZIPLIST ， COUNT不管填多少， 都是返回全部， 因为ZIPLIST 本身就用于小集合， 

```shell
127.0.0.1:6379> HSCAN hashniuniu 0 COUNT 10
1) "0"
2)  1) "v1"
    2) "f1"
    3) "v2"
    4) "f2"
    5) "v3"
    6) "f3"
    7) "v4"
    8) "f4"
    9) "v8"
   10) "f1"
   11) "v7"
   12) "f2"
   13) "v6"
   14) "f3"
   15) "v5"
   16) "f4"
   17) "v9"
   18) "f1"
   19) "v19"
   20) "f2"
   21) "v11"
   22) "f3"
   23) "v12"
   24) "f4"
```

使用MATCH匹配

```
127.0.0.1:6379> HSCAN hashniuniu 0 MATCH *2
1) "0"
2) 1) "v2"
   2) "f2"
   3) "v12"
   4) "f4"
```

### 原理

Hash 底层有两种编码结构， 一种时压缩列表， 一个时HASHTABLE。 

使用压缩列表需要同时满足两个条件：

1. Hash对象 保存的所有值和键的长度都小于64字节
2. Hash对象元素个数少于512字节

两个条件任何一个不满足， 编码结构就用HASHTABLE

<img src="../../assets/imgs/image-20230708095901246.png" alt="image-20230708095901246" style="zoom:25%;" />

ZIPLIST 在数据量较小时将数据紧凑排列， 对应到Hash，将filed-value 当作entry 放入ZIPLIST。

<img src="../../assets/imgs/image-20230708100212141.png" alt="image-20230708100212141" style="zoom:25%;" />

无序集合Set 也使用HASHTABLE ， 在Set中value始终为NULL， 但在HSet中，有对应的值。

<img src="../../assets/imgs/image-20230708100933153.png" alt="image-20230708100933153" style="zoom:25%;" />





## ZSet

ZSet 是有序结合，也是Sorted Set， 是一组 按 关联积分 有序的字符串集合，积分相同 按照字典序排序

场景：游戏排行榜

### 常用操作

#### 写操作

##### ZADD

语法： ZADD key score member [score member ...]

功能：向Sorted Set 增加数据， 如果已经存在的key， 则更新对应的数据

```shell
127.0.0.1:6379> ZADD zsetniuniu 100 user1 128 user2 66 user3
(integer) 3
```

扩展参数：

- XX： 仅更新 存在的成员， 不添加新成员
- NX：不更新 存在的成员， 只添加新成员
- LT：更新 新的分值 比当前分值小的成员， 不存在则新增
- GT：更新 新的分值 比当前分值大的成员， 不存在则新

<img src="../../assets/imgs/image-20230708163010599.png" alt="image-20230708163010599" style="zoom:25%;" />

```shell
127.0.0.1:6379> ZADD zsetniuniu 128 user3 512 user4
(integer) 1

```

<img src="../../assets/imgs/image-20230708192137257.png" alt="image-20230708192137257" style="zoom:25%;" />



##### ZREM

语法： ZREM key  member [score member ...]

功能：删除ZSet 中的元素

```shell
127.0.0.1:6379> ZREM zsetniuniu user4
(integer) 1
```

#### 读操作



##### ZCARD

语法： ZCARD key  

功能：查看ZSet 中成员总数

```shell
127.0.0.1:6379> ZCARD zsetniuniu
(integer) 3
```

##### ZRANGE

语法： ZRANGE key  start stop [WITHSCORES]

功能：查询从 start 到stop 范围的ZSET 数据， WITHSCORES 选项， 不写 输出就只有key， 没有score值

```shell
127.0.0.1:6379> ZRANGE zsetniuniu 0 -1 WITHSCORES
1) "user1"
2) "100"
3) "user2"
4) "128"
5) "user3"
6) "128"
127.0.0.1:6379> ZRANGE zsetniuniu 0 -1 
1) "user1"
2) "user2"
3) "user3"
```



##### ZREVRANGE

语法： ZREVRANGE key  start stop [WITHSCORES]

功能：即reverse range， 从大到小遍历， WITHSCORES 选项

```shell
127.0.0.1:6379> ZREVRANGE zsetniuniu 0 -1 WITHSCORES
1) "user3"
2) "128"
3) "user2"
4) "128"
5) "user1"
6) "100"
127.0.0.1:6379> ZREVRANGE zsetniuniu 0 -1
1) "user3"
2) "user2"
3) "user1"
```



##### ZCOUNT

语法： ZCOUNT key  min max

功能：计算 min - max 积分范围的成员个数

```shell
127.0.0.1:6379> ZCOUNT zsetniuniu 50 200
(integer) 3
```



##### ZRANK

语法： ZRANK key  member

功能：查看ZSet 中member 的排名索引， 索引是从0开始， 如果排第一， 索引就是0

```shell
127.0.0.1:6379> ZRANK zsetniuniu user2
(integer) 1
127.0.0.1:6379> ZRANK zsetniuniu user1
(integer) 0
127.0.0.1:6379> ZRANK zsetniuniu user3
(integer) 2
```



##### ZSCORE

语法： ZSCORE key  member

功能：查询ZSet中成员的分数

```shell
127.0.0.1:6379> ZSCORE zsetniuniu user1
"100"
```

### 底层实现

#### 编码方式

ZSet 底层编码有两种， 一种是ZIPLIST 另一种是SKIPLIST + HASHTABLE

<img src="../../assets/imgs/image-20230708195234434.png" alt="image-20230708195234434" style="zoom:25%;" />



ZSet中ZIPLIST 用于数据量比较小时候的内存节省。

<img src="../../assets/imgs/image-20230708195619646.png" alt="image-20230708195619646" style="zoom:33%;" />





ZSet使用ZIPLIST编码需要同时满足以下规则：

1. 列表对象保存的所有字符串对象长度都小于64字节；
2. 列表对象元素个数小于128个

否则使用SKIPLIST + HASHTABLE， 其中SKIPLIST 是一种 可以快速查找的多级链表结构， 通过skiplist 可以快速 定位到数据所在， 

排名操作、范围查询性能都很高， 通过使用HASHTABLE 配合查询， 就可以在O(1) 时间复杂度查询到成员的分数值。



#### 总结

ZSet 是有序集合， 用于保存、 查询处理有序的集合， 其范围查询、成员分枝查询速度都非常快。



## Stream（不重要）

Stream 是 Redis 5.0 新增的操作对象， 可以看作为一个拥有持久化能力的轻量级消息队列

生产消费场景， 

### 常用操作

流ID 

### 写操作

#### XADD

语法： XADD key ID field string [field string ...]

功能： 向Stream 中添加流数据， 如果没有就新建一个Stream， 返回值是一个流ID， 如果输入时是一个自定义流ID，返回值与之相同，如果用* 自动生成， 返回的就是自动生成的流ID

```shell
127.0.0.1:6379> XADD strmniuniu * f1 msg1 f2 msg2
"1688824739946-0"
127.0.0.1:6379> XADD strmniuniu * f3 msg3 f4 msg4
"1688824782508-0"
```

Redis 生成的 流id 是由 毫秒时间和序列号组成

自定义流ID

```shell
127.0.0.1:6379> XADD newstrm 12345 f5 msg5
"12345-0"
```





### 读操作

#### 查询流信息

##### XLEN

语法： XLEN key

功能： 返回流数目

```shell
127.0.0.1:6379> XLEN strmniuniu
(integer) 2
127.0.0.1:6379> XLEN newstrm
(integer) 1
```



##### XRANGE

语法： XRANGE  key start end [COUNT count]

功能： 范围查询流信息

"+" 代替保存最大ID

"-" 代替保存最小ID

```shell
1) 1) "1688824739946-0"
   2) 1) "f1"
      2) "msg1"
      3) "f2"
      4) "msg2"
2) 1) "1688824782508-0"
   2) 1) "f3"
      2) "msg3"
      3) "f4"
      4) "msg4"
```



### 群组操作

Stream 群组具备特性：

1.  群组里可以多个成员，成员名称 由消费方自定义， 组内唯一即可
2. 同个 群组共享消息， 消息被其中 一个消费者消费之后， 其他消费者不会重复消费
3. 不再群组内的客户端， 也可以通过XREAD 命令 来和群组一起消费， 即 群组和非群组 可以混用



群组核心操作：

- XGROUP 用于创建
- XREADGROUP 用于通过消费者组 从一个Stream中读
- XACK 允许 消费者 将待处理消息标记为已正确 处理的命令

#### 创建消费者群组

语法： XGROUP  CREATE key groupname id-or-$

功能： 创建一个新的消费群组， 成功返回OK

```shell
127.0.0.1:6379> XGROUP CREATE strmniuniu group2 0
OK
127.0.0.1:6379> XGROUP CREATE strmniuniu group1 0
OK
```





#### 消费数据 （不懂）

语法： Xreadgroup  GROUP \${group_id}  \${member_id}  COUNT  \${num} STREAMS \${stream_id}

group_id : 群组id

member_id: 群组成员id

num：消费几条数据

stream_id： stream 流id

功能： 消费流中的数据

```shell
127.0.0.1:6379> XGROUP CREATE strmniuniu group2 0
OK
127.0.0.1:6379> XGROUP CREATE strmniuniu group1 0
OK
```



## Bitmaps

bitmaps 提供二进制位操作， 可以设置位的值， 可以获取位的值

比如： 要存一个uint8 的二进制数据， 并支持更改位值



#### SETBIT

语法：SETBIT key offset value

功能：设置位图对应位置为1， 返回值为 更新前的值

```shell
127.0.0.1:6379> SETBIT data:2023-01-01-00:00 66 1
(integer) 0
```

#### GETBIT

语法：GETBIT key offset

功能：设置位图对应位置为1， 返回值为 更新前的值

```shell
127.0.0.1:6379> GETBIT data:2023-01-01-00:00 66 
(integer) 1
```





```shell
127.0.0.1:6379> GETBIT pings:2023-01-01-00:00 456
(integer) 0
```



### HyperLogLog

Redis HyperLogLog 是用来做基数统计的 算法

优点：输入元素的数量或者体积非常大时， 计算基数 所需的空间总是固定的，

在Redis 中，每个HyperLogLog 只需要花费 12KB 内存， 就可以计算 2^64 个不同元素的技术

```
127.0.0.1:6379> PFADD members 123
(integer) 1
127.0.0.1:6379> PFADD members 500
(integer) 1
127.0.0.1:6379> PFADD members 12
(integer) 1
127.0.0.1:6379> PFCOUNT members
(integer) 3
127.0.0.1:6379> PFADD members 12
(integer) 0
127.0.0.1:6379> PFADD members 13
(integer) 1
127.0.0.1:6379> PFCOUNT members
(integer) 4
```

### Geospatial



Geospatial 地理空间

GEOADD： 增加一个地理位置索引



## 对象过期时间



Redis 的过期时间 是 给一个key 指定一个时间点， 等达到这个时间， 数据就被认为是过期数据

语法： SET key value EX  seconds 设置多少秒后过期

SET key value PX milliseconds 设置多少毫秒之后 过期

TTL key 查看还有多久过期



```shell
127.0.0.1:6379> SET tmpkey data EX 5
OK
127.0.0.1:6379> TTL tmpkey
(integer) -2
127.0.0.1:6379> GET tmpkey
(nil)
127.0.0.1:6379> SET tmpkey data EX 10
OK
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
(nil)
127.0.0.1:6379> SET tmpkey data PX 5000
OK
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> GET tmpkey
"data"
127.0.0.1:6379> TTL tmpkey
(integer) -2
```

通用的过期命令是EXPIRE， 对所有数据对象设置过期时间， EXPORE 分 秒和毫秒：

1. EXPIRE key seconds 设置一个key的过期时间， 单位 为秒
2. PEXPIRE key milliseconds 设置一个key的过期时间， 单位毫秒



### 键 过期 删除

过期的键 的 清楚策略 有三种 定时删除、定期删除和惰性删除

定时删除 是在设置键的过期时间 的同时，创建一个定时器，让定时器 在键的过期时间来临时， 立即执行对键的删除操作。

惰性删除 使用的时候 发现Key过期， 此时在进行删除

定期删除 每隔一段时间， 程序就对数据库进行一次检查，每次删除一部分过期键， 这属于渐进

**Redis 过期键 采用的是 惰性删除+ 定期删除 二者结合的方式**

<img src="../../assets/imgs/image-20230710101644371.png" alt="image-20230710101644371" style="zoom:35%;" />

定期删除需要考虑：

1. 定期删除的频率

   取决于 Redis 周期任务的执行频率， 周期任务 会执行关闭过期客户端、删除过期key一系列任务

   ```shell
   127.0.0.1:6379> info
   # Server
   redis_version:7.0.4
   redis_git_sha1:00000000
   redis_git_dirty:0
   redis_build_id:ef6295796237ef48
   redis_mode:standalone
   os:Darwin 22.1.0 arm64
   arch_bits:64
   monotonic_clock:POSIX clock_gettime
   multiplexing_api:kqueue
   atomicvar_api:c11-builtin
   gcc_version:4.2.1
   process_id:1268
   process_supervised:no
   run_id:e917bf72d07c6024a36b42dae2e65c587fff6e6f
   tcp_port:6379
   server_time_usec:1688955453515034
   uptime_in_seconds:769932
   uptime_in_days:8
   hz:10
   configured_hz:10
   lru_clock:11233853
   executable:/opt/homebrew/opt/redis/bin/redis-server
   config_file:/opt/homebrew/etc/redis.conf
   io_threads_active:0
   
   ```

   hz 就是频率， 默认是10， 1s 10次出发周期任务

2. 每次删除的数量

   每次检查的数量是20个/次， 如果检查过期key 数量占比大于25%， 再抽出20个来检查，重复流程。

   Redis 保证定期删除不会出现循环过度， 导致线程卡死，增加爱了定期删除 循环流程的时间上限，默认不超过25ms。





## 对象引用计数

记录某个内存对象被引用的次数，引用计数大于0，代表这个对象还在被引用，等于0说明这个对象已经没有被引用，可对其进行释放。

redisObject 的结构定义中 有个refcount字段，表示Redis中的引用计数

```c
typedef struct redisObject{
  
  unsigned type:4; // Redis 对象
  unsigned encoding:4; // 底层编码， 用OBJECT ENCODING[key]  可以看到编码方式
  unsigned lru:LRU_BITS; // 记录对象访问信息， 用于内存淘汰
  
  int refcount; // 引用 计数， 描述有多少个指针指向该对象
  void *ptr; // 内容指针， 指向实际内容
}robj;
```

当refcount 减少到0， 就会出发对象的释放，Redis的引用计数 为 整数数据服务的， 范围是0-9999。

Redis 在初始化服务器时， 会创建10000个数值， 当服务器、新创建的键 需要用到值0 到9999的字符串对象， 服务器就会使用这些共享对象。



### 引用计数验证

OBJECT REFCOUNT [arguments [arguments ...]]



```shell
127.0.0.1:6379> SET num 100
OK
127.0.0.1:6379> OBJECT REFCOUNT num
(integer) 2147483647
```

REFCOUNT的 值一直不变





# 底层数据结构

## 压缩列表

排列紧凑的列表， 压缩列表在Redis中有LISTPACK和ZIPLIST。

### 解决什么问题

压缩列表主要是为了底层数据结构提供紧凑型数据存储方式，节约内存，数据量小时遍历访问性能好。（节省链表指针的开销、连续+缓存命中率高）



## HASHTABLE

HASHTABLE 可以想象成目录， 通过目录找到页数。通过HASHTABLE 可以只用O(1) 时间复杂度 就能快速找到key 对应的value

### HASHTABLE结构





## 跳表

跳表是Redis有序集合ZSet 底层的数据结构

跳表本质是链表， 通过给链接增加了多级的索引，通过索引可以一次实现多个节点的跳跃，提高性能。



<img src="../../assets/imgs/image-20230708204327886.png" alt="image-20230708204327886" style="zoom:25%;" />





标准的跳表有限制：

1. score 值不能重复
2. 只有向前指针， 没有回退指针



#### Redis的跳表实现

<img src="../../assets/imgs/image-20230708211739783.png" alt="image-20230708211739783" style="zoom:25%;" />



```c
typedef struct zskiplistNode {
    sds ele;          //SDS结构， 存储数据
    double score;     //节点的分数， 浮点型数据
    struct zskiplistNode *backward; // 指向上一个节点的回退指针， 支持从表尾向表头遍历， 也就是ZREVRANGE  
    struct zskiplistLevel {
        struct zskiplistNode *forward;      // 指向该层下个能跳到的节点， 
        unsigned long span;                 // span 记录了距离下个节点的部署， 
    } level[];                      // 是 zskiplistLevel 结构体数组，  数组结构 表示每个节点都可能是个多层结构
} zskiplistNode;
```

<img src="../../assets/imgs/image-20230708213241635.png" alt="image-20230708213241635" style="zoom:25%;" />

#### Redis 跳表单个节点有几层？

Redis 使用概率均衡的思路 来确定新插入节点的层数；

#### Redis跳表的性能优化了多少



## Redis Object 总结

String、List、Set、HSet、ZSet 5种对象



<img src="../../assets/imgs/image-20230709155158464.png" alt="image-20230709155158464" style="zoom:25%;" />