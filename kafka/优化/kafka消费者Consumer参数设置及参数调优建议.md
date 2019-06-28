## 1  消息的接收->基于Consumer Group

Consumer Group 主要用于实现高伸缩性，高容错性的Consumer机制。因此，消息的接收是基于Consumer Group 的。组内多个Consumer实例可以同时读取Kafka消息，同一时刻一条消息只能被一个消费者消费，而且一旦某一个consumer "挂了"， Consumer Group 会立即将已经崩溃的Consumer负责的分区转交给其他Consumer来负责。从而保证 Consumer Group 能够正常工作。

## 2 位移保存->基于Consumer Group

说来奇怪，位移保存是基于Consumer Group，同时引入检查点模式，定期实现offset的持久化。

## 3 位移提交->抛弃ZooKeeper

Consumer会定期向kafka集群汇报自己消费数据的进度，这一过程叫做位移的提交。这一过程已经抛弃Zookeeper，因为Zookeeper只是一个协调服务组件，不能作为存储组件，高并发的读取势必造成Zk的压力。

- 新版本位移提交是在kafka内部维护了一个内部Topic(_consumer_offsets)。
- 在kafka内部日志目录下面，总共有50个文件夹，每一个文件夹包含日志文件和索引文件。日志文件主要是K-V结构，（[group.id](https://link.juejin.im?target=http%3A%2F%2Fgroup.id),topic,分区号）。
- 假设线上有很多的consumer和ConsumerGroup，通过对group.id做Hash求模运算，这50个文件夹就可以分散同时位移提交的压力。

## 4 官方案例

### 4.1 自动提交位移

```java
     Properties props = new Properties();
     props.put("bootstrap.servers", "localhost:9092");
     props.put("group.id", "test");
     props.put("enable.auto.commit", "true");
     props.put("auto.commit.interval.ms", "1000");
     props.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
     props.put("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
     KafkaConsumer<String, String> consumer = new KafkaConsumer<>(props);
     consumer.subscribe(Arrays.asList("foo", "bar"));
     while (true) {
         ConsumerRecords<String, String> records = consumer.poll(100);
         for (ConsumerRecord<String, String> record : records)
             System.out.printf("offset = %d, key = %s, value = %s%n", record.offset(), record.key(), record.value());
     }
```

### 4.2 手动提交位移

```java
     Properties props = new Properties();
     props.put("bootstrap.servers", "localhost:9092");
     props.put("group.id", "test");
     props.put("enable.auto.commit", "false");
     props.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
     props.put("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
     KafkaConsumer<String, String> consumer = new KafkaConsumer<>(props);
     consumer.subscribe(Arrays.asList("foo", "bar"));
     final int minBatchSize = 200;
     List<ConsumerRecord<String, String>> buffer = new ArrayList<>();
     while (true) {
         ConsumerRecords<String, String> records = consumer.poll(100);
         for (ConsumerRecord<String, String> record : records) {
             buffer.add(record);
         }
         if (buffer.size() >= minBatchSize) {
             insertIntoDb(buffer);
             consumer.commitSync();
             buffer.clear();
         }
     }
```

## 5 kafka Consumer参数设置

- consumer.poll(1000) 重要参数
- 新版本的Consumer的Poll方法使用了类似于Select I/O机制，因此所有相关事件（包括reblance，消息获取等）都发生在一个事件循环之中。
- 1000是一个超时时间，一旦拿到足够多的数据（参数设置），consumer.poll(1000)会立即返回 ConsumerRecords<String, String> records。
- 如果没有拿到足够多的数据，会阻塞1000ms，但不会超过1000ms就会返回。

------

- session. timeout. ms <=  coordinator检测失败的时间
- 默认值是10s
- 该参数是 Consumer Group 主动检测 (组内成员comsummer)崩溃的时间间隔。若设置10min，那么Consumer Group的管理者（group coordinator）可能需要10分钟才能感受到。太漫长了是吧。

------

- max. poll. interval. ms <= 处理逻辑最大时间
- 这个参数是0.10.1.0版本后新增的，可能很多地方看不到喔。这个参数需要根据实际业务处理时间进行设置，一旦Consumer处理不过来，就会被踢出Consumer Group 。
- 注意：如果业务平均处理逻辑为1分钟，那么max. poll. interval. ms需要设置稍微大于1分钟即可，但是session. timeout. ms可以设置小一点（如10s），用于快速检测Consumer崩溃。

------

- auto.offset.reset
- 该属性指定了消费者在读取一个没有偏移量后者偏移量无效（消费者长时间失效当前的偏移量已经过时并且被删除了）的分区的情况下，应该作何处理，默认值是latest，也就是从最新记录读取数据（消费者启动之后生成的记录），另一个值是earliest，意思是在偏移量无效的情况下，消费者从起始位置开始读取数据。

------

- enable.auto.commit
- 对于精确到一次的语义，最好手动提交位移

------

- fetch.max.bytes
- 单次获取数据的最大消息数。

------

- max.poll.records  <=  吞吐量
- 单次poll调用返回的最大消息数，如果处理逻辑很轻量，可以适当提高该值。
- 一次从kafka中poll出来的数据条数,max.poll.records条数据需要在在session.timeout.ms这个时间内处理完
- 默认值为500

------

- heartbeat. interval. ms <= 居然拖家带口
- heartbeat心跳主要用于沟通交流，及时返回请求响应。这个时间间隔真是越快越好。因为一旦出现reblance,那么就会将新的分配方案或者通知重新加入group的命令放进心跳响应中。

------

- connection. max. idle. ms <= socket连接
- kafka会定期的关闭空闲Socket连接。默认是9分钟。如果不在乎这些资源开销，推荐把这些参数值为-1，即不关闭这些空闲连接。

------

- request. timeout. ms
- 这个配置控制一次请求响应的最长等待时间。如果在超时时间内未得到响应，kafka要么重发这条消息，要么超过重试次数的情况下直接置为失败。
- 消息发送的最长等待时间.需大于session.timeout.ms这个时间

------

- fetch.min.bytes
- server发送到消费端的最小数据，若是不满足这个数值则会等待直到满足指定大小。默认为1表示立即接收。

------

- [fetch.wait.max.ms](https://link.juejin.im?target=http%3A%2F%2Ffetch.wait.max.ms)
- 若是不满足fetch.min.bytes时，等待消费端请求的最长等待时间 

------

- 0.11 新功能
- 空消费组延时rebalance，主要在server.properties文件配置
- [group.initial.rebalance.delay.ms](https://link.juejin.im?target=http%3A%2F%2Fgroup.initial.rebalance.delay.ms)  <=牛逼了，我的kafka，防止成员加入请求后本应立即开启的rebalance
- 对于用户来说，这个改进最直接的效果就是新增了一个broker配置：[group.initial.rebalance.delay.ms](https://link.juejin.im?target=http%3A%2F%2Fgroup.initial.rebalance.delay.ms)，
- 默认是3秒钟。
- 主要作用是让coordinator推迟空消费组接收到成员加入请求后本应立即开启的rebalance。在实际使用时，假设你预估你的所有consumer组成员加入需要在10s内完成，那么你就可以设置该参数=10000。

## 6 线上采坑

```shell
org.apache.kafka.clients.consumer.CommitFailedException:
 Commit cannot be completed since the group has already rebalanced and assigned the partitions to another member. 
This means that the time between subsequent calls to poll() was longer than the configured session.timeout.ms, which typically implies that the poll loop is spending too much time message processing. 
You can address this either by increasing the session timeout or by reducing the maximum size of batches returned in poll() with max.poll.records. [com.bonc.framework.server.kafka.consumer.ConsumerLoop]
```

###### 基于最新版本10，注意此版本session. timeout. ms 与max.poll.interval.ms进行功能分离了。

- 可以发现频繁reblance，并伴随者重复性消费，这是一个很严重的问题，就是处理逻辑过重,max.poll. [interval.ms](https://link.juejin.im?target=http%3A%2F%2Finterval.ms) 过小导致。发生的原因就是 poll（）的循环调用时间过长，出现了处理超时。此时只用调大max.poll. [interval.ms](https://link.juejin.im?target=http%3A%2F%2Finterval.ms) ，调小max.poll.records即可，同时要把request. timeout. ms设置大于max.poll. [interval.ms](https://link.juejin.im?target=http%3A%2F%2Finterval.ms)

## 7 总结

优化会继续，暂时把核心放在request. timeout. ms, max. poll. interval. ms，max.poll.records 上，避免因为处理逻辑过重，导致Consumer被频繁的踢出Consumer group。
