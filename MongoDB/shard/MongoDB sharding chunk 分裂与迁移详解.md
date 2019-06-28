## Primary shard

使用 MongoDB sharding 后，数据会以 chunk 为单位（默认64MB）根据 `shardKey` 分散到后端1或多个 shard 上。

<font color="red">**每个 database 会有一个 `primary shard`**</font>，在数据库创建时分配

- database 下启用分片（即调用 `shardCollection` 命令）的集合，刚开始会生成一个[minKey, maxKey] 的 chunk，该 chunk 初始会存储在 `primary shard` 上，然后随着数据的写入，不断的发生 chunk 分裂及迁移，整个过程如下图所示。
- <font color="red">database 下没有启用分片的集合，其所有数据都会存储到  `primary shard`</font>

## 何时触发 chunk 分裂？

mongos 上有个 `sharding.autoSplit` 的配置项，可用于控制是否自动触发 chunk 分裂，默认是开启的。如无专业人士指导，强烈建议不要关闭 autoSplit，更好的方式是使用「预分片」的方式来提前分裂，后面会详细介绍。

mongoDB 的自动 chunk 分裂只会发生在 mongos 写入数据时，当写入的数据超过一定量时，就会触发 chunk 的分裂，具体规则如下。

```
int ChunkManager::getCurrentDesiredChunkSize() const {
    // split faster in early chunks helps spread out an initial load better
    const int minChunkSize = 1 &lt;&lt; 20;  // 1 MBytes

    int splitThreshold = Chunk::MaxChunkSize;  // default 64MB

    int nc = numChunks();

    if (nc &lt;= 1) {
        return 1024;
    } else if (nc &lt; 3) {
        return minChunkSize / 2;
    } else if (nc &lt; 10) {
        splitThreshold = max(splitThreshold / 4, minChunkSize);
    } else if (nc getCurrentDesiredChunkSize();
        if (_minIsInf() || _maxIsInf()) {
            splitThreshold = (int)((double)splitThreshold * .9);
        }

        if (_dataWritten _splitHeuristics._splitTickets.tryAcquire()) {
            LOG(1) &lt;&lt; &quot;won't auto split because not enough tickets: &quot; &lt;getns();
            return false;
        }
        ......
}
```

chunkSize 为默认64MB是，分裂阈值如下

| 集合 chunk 数量 | 分裂阈值 |
| --------------- | -------- |
| 1               | 1024B    |
| [1, 3)          | 0.5MB    |
| [3, 10)         | 16MB     |
| [10, 20)        | 32MB     |
| [20, max)       | 64MB     |

写入数据时，当 chunk 上写入的数据量，超过分裂阈值时，就会触发 chunk 的分裂，chunk 分裂后，当出现各个 shard 上 chunk 分布不均衡时，就会触发 chunk 迁移。

## 何时触发 chunk 迁移？

默认情况下，MongoDB 会开启 balancer，在各个 shard 间迁移 chunk 来让各个 shard 间负载均衡。用户也可以手动的调用 `moveChunk` 命令在 shard 之间迁移数据。

Balancer 在工作时，会根据`shard tag`、`集合的 chunk 数量`、`shard 间 chunk 数量差值` 来决定是否需要迁移。

#### （1）根据 shard tag 迁移

MongoBD sharding 支持 `shard tag` 特性，用户可以给 shard 打上标签，然后给集合的某个range 打上标签，mongoDB 会通过 balancer 的数据迁移来保证「拥有 tag 的 range 会分配到具有相同 tag 的 shard 上」。

#### （2）根据 shard 间 chunk 数量迁移

```
int threshold = 8;
if (balancedLastTime || distribution.totalChunks() &lt; 20)
    threshold = 2;
else if (distribution.totalChunks() &lt; 80)
    threshold = 4;
```

| 集合 chunk 数量 | 迁移阈值 |
| --------------- | -------- |
| [1, 20)         | 2        |
| [20, 80)        | 4        |
| [80, max)       | 8        |

针对所有启用分片的集合，如果 「拥有最多数量 chunk 的 shard」 与 「拥有最少数量 chunk 的 shard」 的差值超过某个阈值，就会触发 chunk 迁移； 有了这个机制，当用户调用 `addShard` 添加新的 shard，或者各个 shard 上数据写入不均衡时，balancer 就会自动来均衡数据。

#### （3）removeShard 触发迁移

还有一种情况会触发迁移，当用户调用  `removeShard` 命令从集群里移除shard时，Balancer 也会自动将这个 shard 负责的 chunk 迁移到其他节点，因 `removeShard` 过程比较复杂，这里先不做介绍，后续专门分析下 `removeShard` 的实现。

## chunkSize 对分裂及迁移的影响

MongoDB 默认的 chunkSize 为64MB，如无特殊需求，建议保持默认值；chunkSize 会直接影响到 chunk 分裂、迁移的行为。

- chunkSize 越小，chunk 分裂及迁移越多，数据分布越均衡；反之，chunkSize 越大，chunk 分裂及迁移会更少，但可能导致数据分布不均。
- chunkSize 太小，容易出现 jumbo chunk（即shardKey 的某个取值出现频率很高，这些文档只能放到一个 chunk 里，无法再分裂）而无法迁移；chunkSize 越大，则可能出现 [chunk 内文档数太多（chunk 内文档数不能超过 250000 ）](https://docs.mongodb.com/manual/reference/limits/#Maximum-Number-of-Documents-Per-Chunk-to-Migrate)而无法迁移。
- chunk 自动分裂只会在数据写入时触发，所以如果将 chunkSize 改小，系统需要一定的时间来将 chunk 分裂到指定的大小。
- chunk 只会分裂，不会合并，所以即使将 chunkSize 改大，现有的 chunk 数量不会减少，但 chunk 大小会随着写入不断增长，直到达到目标大小。

## 如何减小分裂及迁移的影响？

mongoDB sharding 运行过程中，自动的 chunk 分裂及迁移如果对服务产生了影响，可以考虑一下如下措施。

#### （1）预分片提前分裂

在使用 `shardCollection` 对集合进行分片时，如果使用 hash 分片，可以对集合进行「预分片」，直接创建出指定数量的 chunk，并打散分布到后端的各个 shard。

指定 `numInitialChunks` 参数在 `shardCollection` 指定初始化的分片数量，该值不能超过 8192。

```
Optional. Specifies the number of chunks to create initially when sharding an empty collection with a hashed shard key. MongoDB will then create and balance chunks across the cluster. The numInitialChunks must be less than 8192 per shard. If the collection is not empty, numInitialChunks has no effect.
```

如果使用 range 分片，因为 shardKey 的取值不确定，预分片意义不大，很容易出现部分 chunk 为空的情况，所以 range 分片只支持 hash 分片。

#### （2）合理配置 balancer

monogDB 的 balancer 能支持非常[灵活的配置策略](https://docs.mongodb.com/manual/tutorial/manage-sharded-cluster-balancer/index.html))来适应各种需求

- Balancer 能动态的开启、关闭

- Blancer 能针对指定的集合来开启、关闭

- Balancer 支持配置时间窗口，**只在制定的时间段内进行迁移**

  - ```javascript
    db.settings.update(
       { _id: "balancer" },
       { $set: { activeWindow : { start : "<start-time>", stop : "<stop-time>" } } },
       { upsert: true }
    )
    ```