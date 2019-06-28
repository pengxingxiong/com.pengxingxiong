切分数据

上节中建立好集群后，默认的是不会将存储的每条数据进行分片处理，需要在数据库和集合的粒度上都开启分片功能。开启test库的分片功能：

```
1.    ./bin/mongo –port 20000
2.    mongos> use admin 
3.    switched to db admin
4.    mongos> db.runCommand({"enablesharding":"test"})
5.    { "ok" : 1 }
```

开启user集合分片功能：

```
1.    mongos> db.runCommand({"shardcollection":"test.user","key":{"_id":1}})
2.    { "collectionsharded" : "test.user", "ok" : 1 }
```

注意：需要切换到admin库执行命令。

片键：上面的key就是所谓的片键（shard key）。MongoDB不允许插入没有片键的文档。但是允许不同文档的片键类型不一样，MongoDB内部对不同类型有一个排序：

![img](http://images.cnitblog.com/blog/288950/201304/11204732-82e0aa5743a5403fac5104568c0e42c3.png)

片键的选择至关重要，后面会进行详细的说明。

这时再切换到config库如下查看:

```shell
1.    mongos> use config
2.    mongos> db.databases.find()
3.    { "_id" : "admin", "partitioned" : false, "primary" : "config" }
4.    { "_id" : "OSSP10", "partitioned" : false, "primary" : "shard0000" }
5.    { "_id" : "test", "partitioned" : true, "primary" : "shard0000" }
6.    { "_id" : "test2", "partitioned" : false, "primary" : "shard0000" }
7.    { "_id" : "test3", "partitioned" : false, "primary" : "shard0001" }
8.    mongos> db.chunks.find()
9.    { "_id" : "test.user-_id_MinKey", "lastmod" : { "t" : 1, "i" : 0 }, "lastmodEpoch" : ObjectId("515a3797d249863e35f0e3fe"), "ns" : "test.user", "min" : { "_id" : { "$minKey" : 1 } }, "max" : { "_id" : { "$maxKey" : 1 } }, "shard" : "shard0000" }
```

Chunks：理解MongoDB分片机制的关键是理解Chunks。mongodb不是一个分片上存储一个区间，而是每个分片包含多个区间，这每个区间就是一个块。

```shell
1.    mongos> use config
2.    mongos> db.settings.find()
3.    { "_id" : "chunksize", "value" : 64 }
4.    ……
```

平衡：如果存在多个可用的分片，只要块的数量足够多，MongoDB就会把数据迁移到其他分片上，这个迁移的过程叫做平衡。Chunks默认的大小是64M，查看config.settings可以看到这个值：

只有当一个块的大小超过了64M，MongoDB才会对块进行分割（但根据我的实践2.4版本块分割的算法好像并不是等到超过chunkSize大小就分割，也不是一分为二,有待继续学习），并当最大分片上的块数量超过另一个最少分片上块数量达到一定阈值会发生所谓的chunk migration来实现各个分片的平衡 (如果启动了balancer进程的话)。这个阈值随着块的数量不同而不同：

 ![img](http://images.cnitblog.com/blog/288950/201304/11204955-c20cee0db0c24459b9097a7817466f5d.png)

这带来一个问题是我们测开发测试的时候，往往希望通过观察数据在迁移来证明分片是否成功，但64M显然过大，解决方法是我们可以在启动mongos的时候用—chunkSize来制定块的大小，单位是MB。

```shell
1.    ./bin/mongos --port 20000 --configdb 192.168.32.13:10000 --logpath log/mongos.log  --fork --chunkSize 1
```

我指定1MB的块大小重新启动了一下mongos进程。

或者向下面这要修改chunkSize大小：

```shell
1.    mongos> use config
2.    mongos> db.settings.save( { _id:"chunksize", value: 1 } )
```

 2.4版本之前MongoDB基于范围进行分片的（2.2.4会介绍基于哈希的分片），对一个集合分片时，一开始只会创建一个块，这个块的区间是（-∞，+∞），-∞表示MongoDB中的最小值，也就是上面db.chunks.find()我们看到的$minKey，+∞表示最大值即​$maxKey。Chunck的分割是自动执行的，类似于细胞分裂，从区间的中间分割成两个。

# 分片测试

 现在对上面我们搭建的MongoDB分片集群做一个简单的测试。目前我们的集群情况如下（为了方面展示我采用了可视化的工具MongoVUE）：

![img](http://images.cnitblog.com/blog/288950/201304/11205202-e6cd56e825db4097a1f2097a96e16545.png)

像上面提到那样为了尽快能观察到测试的效果，我么启动mongos时指定的chunkSize为1MB。

我们对OSSP10库和bizuser集合以Uid字段进行分片：

```shell
1.    mongos> use admin
2.    switched to db admin
3.    mongos> db.runCommand({"enablesharding":"OSSP10"})
4.    { "ok" : 1 }
5.    mongos> db.runCommand({"shardcollection":"OSSP10.bizuser","key":{"Uid":1}})
6.    { "collectionsharded" : "OSSP10.bizuser", "ok" : 1 }
```

在插入数据之前查看一下config.chunks,通过查看这个集合我们可以了解数据是怎么切分到集群的：

```shell
1.    mongos> db.chunks.find()
2.    { "_id" : "OSSP10.bizuser-Uid_MinKey", "lastmod" : { "t" : 1, "i" : 0 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : { "$minKey" : 1 } }, "max" : { "Uid" : { "$maxKey" : 1 } }, "shard" : "shard0000" }
```

开始只有一个块，区间（-∞，+∞）在shard0000(192.168.32.13:27019)上，下面我们循环的插入100000条数据：

```shell
1.    mongos> use OSSP10
2.    switched to db OSSP10
3.    mongos> for(i=0;i<100000;i++){ db.bizuser.insert({"Uid":i,"Name":"zhanjindong","Age":13,"Date":new Date()}); }
```

完成后我们在观察一下config.chunks的情况：

```shell
1.    mongos> db.chunks.find()
2.    { "_id" : "OSSP10.bizuser-Uid_MinKey", "lastmod" : { "t" : 3, "i" : 0 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : { "$minKey" : 1 } }, "max" : { "Uid" : 0 }, "shard" : "shard0002" }
3.    { "_id" : "OSSP10.bizuser-Uid_0.0", "lastmod" : { "t" : 3, "i" : 1 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 0 }, "max" : { "Uid" : 6747 }, "shard" : "shard0000" }
4.    { "_id" : "OSSP10.bizuser-Uid_6747.0", "lastmod" : { "t" : 2, "i" : 0 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 6747 }, "max" : { "Uid" : { "$maxKey" : 1 } }, "shard" : "shard0001" }
```

我们可以看见刚开始的块分裂成了三块分别是（-∞,0）在shard0002上,[0,6747）在shard0000上和[6747,+ ∞）在shard0001上。

说明：这里需要说明的是MongoDB中的区间是左闭右开的。这样说上面第一个块不包含任何数据，至于为什么还不清楚，有待继续调研。

我们持续上面的插入操作（uid从0到100000）我们发现块也在不停的分裂：

```shell
1.    mongos> db.chunks.find()
2.    { "_id" : "OSSP10.bizuser-Uid_MinKey", "lastmod" : { "t" : 3, "i" : 0 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : { "$minKey" : 1 } }, "max" : { "Uid" : 0 }, "shard" : "shard0002" }
3.    { "_id" : "OSSP10.bizuser-Uid_0.0", "lastmod" : { "t" : 3, "i" : 1 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 0 }, "max" : { "Uid" : 6747 }, "shard" : "shard0000" }
4.    { "_id" : "OSSP10.bizuser-Uid_6747.0", "lastmod" : { "t" : 3, "i" : 4 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 6747 }, "max" : { "Uid" : 45762 }, "shard" : "shard0001" }
5.    { "_id" : "OSSP10.bizuser-Uid_99999.0", "lastmod" : { "t" : 3, "i" : 3 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 99999 }, "max" : { "Uid" : { "$maxKey" : 1 } }, "shard" : "shard0001" }
6.    { "_id" : "OSSP10.bizuser-Uid_45762.0", "lastmod" : { "t" : 3, "i" : 5 }, "lastmodEpoch" : ObjectId("515a8f1dc1de43249d8ce83e"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 45762 }, "max" : { "Uid" : 99999 }, "shard" : "shard0001" }
```

分片的规则正是上面提到的块的自动分裂和平衡，可以发现不同的块是分布在不同的分片上。

注意：这里使用Uid作为片键通常是有问题的，2.3对递增片键有详细说明。

Note:通过sh.status()可以很直观的查看当前整个集群的分片情况，类似如下：

![img](http://images.cnitblog.com/blog/288950/201304/11205352-a8ab289c60fe438aa82573db8057e525.png)

# 对已有数据进行分片

面的演示都是从零开始构建分片集群，但是实际中架构是一个演进的过程，一开始都不会进行分片，只有当数据量增长到一定程序才会考虑分片，那么对已有的海量数据如何处理。我们在上节的基础上继续探索。[《深入学习MongoDB》](https://www.baidu.com/s?wd=%E3%80%8A%E6%B7%B1%E5%85%A5%E5%AD%A6%E4%B9%A0MongoDB%E3%80%8B&tn=24004469_oem_dg&rsv_dl=gh_pl_sl_csd)中有如下描述：

![img](http://images.cnitblog.com/blog/288950/201304/11205525-85f937a3f19d4371936628cf25262e8d.png)

我们现在来尝试下（也许我使用的最新版本会有所有变化），先在192.168.71.43:27179上再启一个mongod进程：

```shell
1.    numactl --interleave=all ./bin/mongod --dbpath data/shard3/ --logpath log/shard3.log  --fork --port 27019
```

现在我们像这个mongod进程中的OSSP10库的bizuser集合中插入一些数据：

```shell
1.    for(i=0;i<10;i++){ db.bizuser.insert({"Uid":i,"Name":"zhanjindong2","Age":13,"Date":new Date()}); }
```

我们这个时候尝试将这个mongod加入到之前的集群中去：

```shell
1.    mongos> db.runCommand({addshard:"192.168.71.43:27019" })
2.    {
3.            "ok" : 0,
4.            "errmsg" : "can't add shard 192.168.71.43:27019 because a local database 'OSSP10' exists in another shard0000:192.168.32.13:27019"
5.    }
```

果然最新版本依旧不行。我们删除OSSP10库，新建个OSSP20库再添加分片则可以成功。

```shell
1.    { "shardAdded" : "shard0003", "ok" : 1 }
```

那么看来我们只能在之前已有的数据的mongod进程基础上搭建分片集群，那么这个时候添加切片，MongoDB对之前的数据又是如何处理的呢？我们现在集群中移除上面添加的切片192.168.71.43:27019，然后在删除集群中的OSSP10库。

这次我们尽量模拟生产环境进行测试，重新启动mongos并不指定chunksize，使用默认的最优的大小（64M）。然后在192.168.71.43:27019中新建OSSP10库并向集合bizuser中插入100W条数据：

```shell
1.    for(i=0;i<1000000;i++){ db.bizuser.insert({"Uid":i,"Name":"zhanjindong2","Age":13,"Date":new Date()}); }
```

然后将此节点添加到集群中，并仍然使用递增的Uid作为片键对OSSP10.bizuser进行分片：

```shell
1.    mongos> use admin
2.    switched to db admin
3.    mongos> db.runCommand({"enablesharding":"OSSP10"})
4.    { "ok" : 1 }
5.    mongos> db.runCommand({"shardcollection":"OSSP10.bizuser","key":{"Uid":1}})
6.    { "collectionsharded" : "OSSP10.bizuser", "ok" : 1 }
```

观察一下config.chunks可以看见对新添加的切片数据进行切分并进行了平衡（每个分片上一个块），基本是对Uid（0~1000000）进行了四等分，当然没那精确：

```shell
1.    mongos> use config
2.    switched to db config
3.    mongos> db.chunks.find()
4.    { "_id" : "OSSP10.bizuser-Uid_MinKey", "lastmod" : { "t" : 2, "i" : 0 }, "lastmodEpoch" : ObjectId("515b8f3754fde3fbab130f92"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : { "$minKey" : 1 } }, "max" : { "Uid" : 0 }, "shard" : "shard0000" }
5.    { "_id" : "OSSP10.bizuser-Uid_381300.0", "lastmod" : { "t" : 4, "i" : 1 }, "lastmodEpoch" : ObjectId("515b8f3754fde3fbab130f92"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 381300 }, "max" : { "Uid" : 762601 }, "shard" : "shard0003" }
6.    { "_id" : "OSSP10.bizuser-Uid_762601.0", "lastmod" : { "t" : 1, "i" : 2 }, "lastmodEpoch" : ObjectId("515b8f3754fde3fbab130f92"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 762601 }, "max" : { "Uid" : { "$maxKey" : 1 } }, "shard" : "shard0003" }
7.    { "_id" : "OSSP10.bizuser-Uid_0.0", "lastmod" : { "t" : 3, "i" : 0 }, "lastmodEpoch" : ObjectId("515b8f3754fde3fbab130f92"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 0 }, "max" : { "Uid" : 250000 }, "shard" : "shard0001" }
8.    { "_id" : "OSSP10.bizuser-Uid_250000.0", "lastmod" : { "t" : 4, "i" : 0 }, "lastmodEpoch" : ObjectId("515b8f3754fde3fbab130f92"), "ns" : "OSSP10.bizuser", "min" : { "Uid" : 250000 }, "max" : { "Uid" : 381300 }, "shard" : "shard0002" }
```

MongoDB这种自动分片和平衡的能力使得在迁移老数据的时候变得非常简单，但是如果数据特别大话则可能会非常的慢。

# Hashed Sharding

MongoDB2.4以上的版本支持基于哈希的分片，我们在上面的基础上继续探索。

交代一下因为环境变动这部分示例操作不是在之前的那个集群上，但是系统环境都是一样，逻辑架构也一样，只是ip地址不一样（只搭建在一台虚拟机上）,后面环境有改动不再说明：

配置服务器：192.168.129.132:10000

路由服务器：192.168.129.132:20000

分片1:192.168.129.132:27017

分片2:192.168.129.132:27018

……其他分片端口依次递增。

![img](http://images.cnitblog.com/blog/288950/201304/11205908-ae765d579ca34a9882419c035ee4fef7.png)

我们重建之前的OSSP10库，我们仍然使用OSSP10.bizuser不过这次启动哈希分片，选择_id作为片键：

```shell
1.    mongos> use admin
2.    switched to db admin
3.    mongos> db.runCommand({"enablesharding":"OSSP10"})
4.    { "ok" : 1 }
5.    mongos> db.runCommand({"shardcollection":"OSSP10.bizuser","key":{"_id":"hashed"}})
6.    { "collectionsharded" : "OSSP10.bizuser", "ok" : 1 }
```

我们现在查看一下config.chunks

```shell
1.    mongos> use config
2.    switched to db config
3.    mongos> db.chunks.find()
4.    { "_id" : "OSSP10.bizuser-_id_MinKey", "lastmod" : { "t" : 2, "i" : 2 }, "lastmodEpoch" : ObjectId("515e47ab56e0b3341b76f145"), "ns" : "OSSP10.bizuser", "min" : { "_id" : { "$minKey" : 1 } }, "max" : { "_id" : NumberLong("-4611686018427387902") }, "shard" : "shard0000" }
5.    { "_id" : "OSSP10.bizuser-_id_-4611686018427387902", "lastmod" : { "t" : 2, "i" : 3 }, "lastmodEpoch" : ObjectId("515e47ab56e0b3341b76f145"), "ns" : "OSSP10.bizuser", "min" : { "_id" : NumberLong("-4611686018427387902") }, "max" : { "_id" : NumberLong(0) }, "shard" : "shard0000" }
6.    { "_id" : "OSSP10.bizuser-_id_0", "lastmod" : { "t" : 2, "i" : 4 }, "lastmodEpoch" : ObjectId("515e47ab56e0b3341b76f145"), "ns" : "OSSP10.bizuser", "min" : { "_id" : NumberLong(0) }, "max" : { "_id" : NumberLong("4611686018427387902") }, "shard" : "shard0001" }
7.    { "_id" : "OSSP10.bizuser-_id_4611686018427387902", "lastmod" : { "t" : 2, "i" : 5 }, "lastmodEpoch" : ObjectId("515e47ab56e0b3341b76f145"), "ns" : "OSSP10.bizuser", "min" : { "_id" : NumberLong("4611686018427387902") }, "max" : { "_id" : { "$maxKey" : 1 } }, "shard" : "shard0001" }
```

MongoDB的哈希分片使用了哈希索引:

![img](http://images.cnitblog.com/blog/288950/201304/11210050-c96b7edf8d984721bd9ab2a6888749e8.png)

哈希分片仍然是基于范围的，只是将提供的片键散列成一个非常大的长整型作为最终的片键。官方文中描述如下：

![img](http://images.cnitblog.com/blog/288950/201304/11210107-16f931c5a481448ab78ef14d03c6943c.png)

不像普通的基于范围的分片，哈希分片的片键只能使用一个字段。

 

选择哈希片键最大的好处就是保证数据在各个节点分布基本均匀，下面使用_id作为哈希片键做个简单的测试：

```shell
1.    mongos> db.runCommand({"enablesharding":"mydb"})
2.    db.runCommand({"shardcollection":"mydb.mycollection","key":{"_id":"hashed"}})
3.    mongos> use mydb
4.    mongos> for(i=0;i<333333;i++){ db.mycollection.insert({"Uid":i,"Name":"zhanjindong2","Age":13,"Date":new Date()}); }
```

 

通过MongoVUE观察三个切片上的数据量非常均匀：

![img](http://images.cnitblog.com/blog/288950/201304/11210209-80d76e08f0264efe8505b6fdf3c7e40f.png)![img](http://images.cnitblog.com/blog/288950/201304/11210219-3bffdde6da9d412082a576b1ffabf659.png)![img](http://images.cnitblog.com/blog/288950/201304/11210228-486580d98f384643b1c342935a92adb0.png)

上面是使用官方文档中推荐的Objectid作为片键，情况很理想。如果使用一个自增长的Uid作为片键呢：

```shell
1.    db.runCommand({"shardcollection":"mydb.mycollection","key":{"Uid":"hashed"}})
2.    for(i=0;i<333333;i++){ db.mycollection.insert({"Uid":i,"Name":"zhanjindong2","Age":13,"Date":new Date()}); }
```

# 故障恢复

先不考虑集群中每个分片是副本集复杂的情况，只考虑了每个分片只有一个mongod进程，这种配是只是不够健壮还是非常脆弱。我们在testdb.testcollection上启动分片，然后向其中插入一定量的数据（视你的chunkSize而定），通过观察config.chunks确保testdb.testcollection上的数据被分布在了不同的分片上。

```shell
1.    mongos> db.runCommand({"enablesharding":"testdb"})
2.    mongos> db.runCommand({"shardcollection":"testdb.testcollection","key":{"Uid":1}})
3.    use testdb
4.    for(i=0;i<1000000;i++){ db.testcollection.insert({"Uid":i,"Name":"zhanjindong","Age":13,"Date":new Date()}); }
5.    mongos> use config
6.    switched to db config
7.    mongos> db.chunks.find()
8.    ……
9.    { "_id" : "testdb.testcollection-Uid_747137.0", "lastmod" : { "t" : 4, "i" : 1 }, "lastmodEpoch" : ObjectId("515fc5f0365c860f0bf8e0cb"), "ns" : "testdb.testcollection", "min" : { "Uid" : 747137 }, "max" : { "Uid" : 962850 }, "shard" : "shard0000" }
10.    ……
11.    { "_id" : "testdb.testcollection-Uid_0.0", "lastmod" : { "t" : 1, "i" : 3 }, "lastmodEpoch" : ObjectId("515fc5f0365c860f0bf8e0cb"), "ns" : "testdb.testcollection", "min" : { "Uid" : 0 }, "max" : { "Uid" : 6757 }, "shard" : "shard0001" }
12.    ……
13.    { "_id" : "testdb.testcollection-Uid_531424.0", "lastmod" : { "t" : 2, "i" : 4 }, "lastmodEpoch" : ObjectId("515fc5f0365c860f0bf8e0cb"), "ns" : "testdb.testcollection", "min" : { "Uid" : 531424 }, "max" : { "Uid" : 747137 }, "shard" : "shard0002" }
```

这时我们强制杀掉shard2进程：

```shell
1.    [root@localhost mongodb-2.4.1]# ps -ef |grep mongo
2.    root      6329     1  0 22:52 ?        00:00:08 ./bin/mongod --dbpath data/shard3/ --logpath log/shard3.log --fork --port 27019
3.    kill -9 6329
```

我们尝试用属于不同范围的Uid对testdb.testcollection进行写入操作(这里插入一条记录应该不会导致新的块迁移)：

```shell
1.    [root@localhost mongodb-2.4.1]# ps -ef |grep mongo
2.    use testdb
3.    switched to db testdb
4.    ####写
5.    mongos> db.testcollection.insert({"Uid":747138,"Name":"zhanjindong","Age":13,"Date":new Date()})
6.    ##向shard0000插入没有问题
7.    mongos> db.testcollection.insert({"Uid":6756,"Name":"zhanjindong","Age":13,"Date":new Date()})
8.    ##向shard0001插入没有问题
9.    mongos> db.testcollection.insert({"Uid":531425,"Name":"zhanjindong","Age":13,"Date":new Date()})
10.    socket exception [CONNECT_ERROR] for 192.168.129.132:27019
11.    ##向shard0002插入出问题
12.    ####读
13.    mongos> db.testcollection.find({Uid:747139})
14.    ##从shard0000读取没有问题
15.    mongos> db.testcollection.find({Uid: 2})
16.    ##从shard0001读取没有问题
17.    mongos> db.testcollection.find({Uid: 531426})
18.    error: {
19.        "$err" : "socket exception [SEND_ERROR] for 192.168.129.132:27019",
20.        "code" : 9001,
21.        "shard" : "shard0002"
22.    }
23.    ##从shard0002读取有问题
24.    b.testcollection.count()
25.    Sat Apr  6 00:23:19.246 JavaScript execution failed: count failed: {
26.        "code" : 11002,
27.        "ok" : 0,
28.        "errmsg" : "exception: socket exception [CONNECT_ERROR] for 192.168.129.132:27019"
29.    } at src/mongo/shell/query.js:L180
```

可以看到插入操作涉及到分片shard0002的操作都无法完成。这是[顺理成章](https://www.baidu.com/s?wd=%E9%A1%BA%E7%90%86%E6%88%90%E7%AB%A0&tn=24004469_oem_dg&rsv_dl=gh_pl_sl_csd)的。

下面我们重新启动shard3看集群是否能自动恢复正常操作：

```shell
1.    ./bin/mongod --dbpath data/shard3/ --logpath log/shard3.log --fork --port 27019
2.    mongos> use config
3.    switched to db config
4.    mongos> db.shards.find()
5.    { "_id" : "shard0000", "host" : "192.168.129.132:27017" }
6.    { "_id" : "shard0001", "host" : "192.168.129.132:27018" }
7.    { "_id" : "shard0002", "host" : "192.168.129.132:27019" }
```

重复上面的插入和读取shard0002的操作：

```shell
1.    mongos> db.testcollection.insert({"Uid":531425,"Name":"zhanjindong","Age":13,"Date":new Date()})
2.    ##没有问题
3.    db.testcollection.find({Uid: 531426})
4.    { "_id" : ObjectId("515fc791b86c543aa1d7613e"), "Uid" : 531426, "Name" : "zhanjindong", "Age" : 13, "Date" : ISODate("2013-04-06T06:58:25.516Z") }
5.    ##没有问题
```

总结：当集群中某个分片宕掉以后，只要不涉及到该节点的操纵仍然能进行。当宕掉的节点重启后，集群能自动从故障中恢复过来。