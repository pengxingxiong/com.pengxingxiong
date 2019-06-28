# 生产者使用Kafka事务的两种方式

##为什么要使用Kafka事务

在日常开发中，数据库的事务几乎是必须用到的，事务回滚不一定在于数据增删改异常，可能系统出现特定逻辑判断的时候也需要进行数据回滚，Kafka亦是如此，我们并不希望消息监听器接收到一些错误的或者不需要的消息。SpringBoot使用数据库事务非常简单，只需要在方法上加上@Transactional注解即可，那Kafka如果需要使用事务也可以如此，不过还需修改其他配置才能生效。

## Kafka使用事务的两种方式

（一）配置Kafka事务管理器并使用@Transactional注解
（二）使用KafkaTemplate的executeInTransaction方法

###使用@Transactional注解方式

使用注解方式开启事务还是比较方便的，不过首先需要我们配置KafkaTransactionManager，这个类就是Kafka提供给我们的事务管理类，我们需要使用生产者工厂来创建这个事务管理类。需要注意的是，我们需要在producerFactory中开启事务功能，并设置TransactionIdPrefix，TransactionIdPrefix是用来生成Transactional.id的前缀。

```java
@Bean
public ProducerFactory<Integer, String> producerFactory() {
  DefaultKafkaProducerFactory factory = new DefaultKafkaProducerFactory<>(senderProps());
  factory.transactionCapable();
  factory.setTransactionIdPrefix("tran-");
  return factory;
}

@Bean
public KafkaTransactionManager transactionManager(ProducerFactory producerFactory) {
  KafkaTransactionManager manager = new KafkaTransactionManager(producerFactory);
  return manager;
}
```

配置Kafka事务还是非常简单的，接下来我们测试一下事务是否能正常使用，在DemoTest类中创建该方法

```java
@Test
@Transactional
public void testTransactionalAnnotation() throws InterruptedException {
  kafkaTemplate.send("topic.quick.tran", "test transactional annotation");
  throw new RuntimeException("fail");
}
```

运行测试方法后我们可以看到控制台中输出了如下日志，这就代表我们使用事务成功，或者你可以打开Kafka Tool 2工具查看一下测试前后的数据是否有变化

```shell
org.apache.kafka.common.KafkaException: Failing batch since transaction was aborted
at org.apache.kafka.clients.producer.internals.Sender.maybeSendTransactionalRequest(Sender.java:317) [kafka-clients-1.0.2.jar:na]
at org.apache.kafka.clients.producer.internals.Sender.run(Sender.java:214) [kafka-clients-1.0.2.jar:na]
at org.apache.kafka.clients.producer.internals.Sender.run(Sender.java:163) [kafka-clients-1.0.2.jar:na]
at java.lang.Thread.run(Thread.java:748) [na:1.8.0_161]
```

###### 注意：

### 使用KafkaTemplate.executeInTransaction开启事务

这种方式开启事务是不需要配置事务管理器的，也可以称为本地事务。直接编写测试方法

```java
@Test
public void testExecuteInTransaction() throws InterruptedException {
  kafkaTemplate.executeInTransaction(new KafkaOperations.OperationsCallback() {
    @Override
    public Object doInOperations(KafkaOperations kafkaOperations) {
      kafkaOperations.send("topic.quick.tran", "test executeInTransaction");
      throw new RuntimeException("fail");
      //return true;
    }
  });
}
```

运行测试方法后控制台同样打印出了事务终止的异常，代表可以正常使用事务

```shell
org.apache.kafka.common.KafkaException: Failing batch since transaction was aborted
at org.apache.kafka.clients.producer.internals.Sender.maybeSendTransactionalRequest(Sender.java:317) [kafka-clients-1.0.2.jar:na]
at org.apache.kafka.clients.producer.internals.Sender.run(Sender.java:214) [kafka-clients-1.0.2.jar:na]
at org.apache.kafka.clients.producer.internals.Sender.run(Sender.java:163) [kafka-clients-1.0.2.jar:na]
at java.lang.Thread.run(Thread.java:748) [na:1.8.0_161]Q
```

#KafkaListener条件启动（禁止自启动）

## 条件启动应用场景：

集群中存在主从逻辑，但主从逻辑却不遵循zookeeper的选主，而是自成一套，能够通过Spring的事件监听注解@EventListener监听由spring-integration工具的OnGrantedEvent（从->主）和OnRevokedEvent（主->从）事件。当节点是从节点时，不消费；而当节点是主节点时开始消费。默认是从节点。另外在业务上要求使用批量消费的方案。

> spring-integration是一个功能强大的EIP(Enterprise Integration Patterns)，即企业集成模式。对，spring-integration是一个集大成者。就我自己的理解，集成了众多功能的它，是一种便捷的事件驱动消息框架用来在系统之间做消息传递的。
>
> pring-integration的目标
>
> - ​    提供一个简单的模型来实现复杂的企业集成解决方案
> - ​    为基于spring的应用添加异步的、消息驱动的行为
> - ​    让更多的Spring用户来使用他
>
> spring-integration的原则
>
> - ​    组件之间应该是松散的，模块性的易测的
> - ​    应用框架应该强迫分离业务逻辑和集成逻辑
> - ​    扩展节点应该有更好的抽象和可以再使用的能力   
>
> 其他人的理解，如果你的系统处在各个系统的中间，需要JMS交互，又需要Database/Redis/MongoDB，还需要监听Tcp/UDP等，还有固定的文件转移，分析。还面对着时不时的更改需求的风险。那么，它再适合不过了。
>
> 参考：spring-integration初探

这里存在几个关键点：

1. 主从切换时，能够自由切换是否消费kafka数据；
2. 初始状态时，不消费kafka。
3. 使用kafka批量消费。

## 使用@KafkaListener

在spring cloud中推荐使用@StreamListener，该注解主要能够适配kafka、RokitMQ等主流消息队列，但是消息都是一条条的拉取，性能主要损失在网络传输上，因此这里使用高度可定制的@KafkaListener。使用@KafkaListener注解需要自定义配置项。其结构主要为：

```java
public @interface KafkaListener {
    String id() default "";

    String containerFactory() default "";

    String[] topics() default {};

    String topicPattern() default "";

    TopicPartition[] topicPartitions() default {};

    String containerGroup() default "";

    String errorHandler() default "";

    String groupId() default "";

    boolean idIsGroup() default true;

    String clientIdPrefix() default "";

    String beanRef() default "__listener";
}

```

各个属性描述

- id：消费者的id，当GroupId没有被配置的时候，默认id为GroupId
- containerFactory：上面提到了@KafkaListener区分单数据还是多数据消费只需要配置一下注解的containerFactory属性就可以了，这里面配置的是监听容器工厂，也就是ConcurrentKafkaListenerContainerFactory，配置BeanName
- topics：需要监听的Topic，可监听多个
- topicPartitions：可配置更加详细的监听信息，必须监听某个Topic中的指定分区，或者从offset为200的偏移量开始监听
- errorHandler：监听异常处理器，配置BeanName
- groupId：消费组ID
- idIsGroup：id是否为GroupId
- clientIdPrefix：消费者Id前缀
- beanRef：真实监听容器的BeanName，需要在 BeanName前加 "__"

最重要的是containerFactory这个实现了KafkaListenerContainerFactory类的bean。下面是其使用方式。

KafkaConfig.java

```java
/**
 * Created by pengxingxiong@ruijie.com.cn on 2019/4/21 14:01
 * 该配置类专门用于物联终端大型流量消费
 */
@Order() //让该类在最后实例化
@Configuration
@EnableKafka
@Slf4j
public class SmartIdentifyKafkaConfig {
    private String groupId;
    private String autoOffsetReset;
    private String bootstrapServers;
    private int maxPollRecords;
    public int kafkaTopicPatitions;
    private String topic;

    private SIConfiguration siConfiguration;

    @Autowired
    public SmartIdentifyKafkaConfig(@NotNull SIConfiguration siConfiguration) {
        groupId = siConfiguration.kafkaGroupIdPrefix + System.currentTimeMillis();
        autoOffsetReset = siConfiguration.kafkaAutoOffsetReset;
        bootstrapServers = siConfiguration.kafkaBrokers;
        maxPollRecords = siConfiguration.kafkaMaxPollRecords;
        topic = siConfiguration.kafkaTopic;
        // 可以在此处求并发线程数
        kafkaTopicPatitions = getKafkaTopicPartitions(topic);
        log.info(String.format("since the partitions of topic [%s] were %s,so seted the thread concurrency= [%s]", kafkaTopicPatitions, topic, kafkaTopicPatitions));

    }

    @Bean
    KafkaListenerContainerFactory<?> smartIdentifyBatchFactory() {
        ConcurrentKafkaListenerContainerFactory<String, String> factory = new
            ConcurrentKafkaListenerContainerFactory<>();
        factory.setConsumerFactory(new DefaultKafkaConsumerFactory<>(smartIdentifyConsumerConfigs()));
        factory.setBatchListener(true); // 开启批量监听
        factory.setConcurrency(kafkaTopicPatitions);//并发线程数
        factory.getContainerProperties().setAckMode(AbstractMessageListenerContainer.AckMode.MANUAL_IMMEDIATE);//设置提交偏移量的方式
        factory.setAutoStartup(false); // 禁止自动启动，用于主从切换，本地调试时可以修改这个配置
        return factory;
    }

    private Map<String, Object> smartIdentifyConsumerConfigs() {

        Map<String, Object> props = new HashMap<>();
        props.put(ConsumerConfig.GROUP_ID_CONFIG, groupId);
        props.put(ConsumerConfig.AUTO_OFFSET_RESET_CONFIG, autoOffsetReset);
        props.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, bootstrapServers);
        props.put(ConsumerConfig.MAX_POLL_RECORDS_CONFIG, maxPollRecords); //设置每次接收Message的数量
        props.put(ConsumerConfig.AUTO_COMMIT_INTERVAL_MS_CONFIG, "100");
        props.put(ConsumerConfig.SESSION_TIMEOUT_MS_CONFIG, 120000);
        props.put(ConsumerConfig.REQUEST_TIMEOUT_MS_CONFIG, 180000);
        props.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        props.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, false);//禁止自动提交
        return props;
    }

    /**
     * 获取kafka的指定topic的分区数
     */
    private int getKafkaTopicPartitions(String topic) {
        // 这里需要创建一个简单的配置
        Map<String, Object> props = new HashMap<>();
        props.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, bootstrapServers);
        props.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        int result;
        try {
            log.info("loading kafka.properties...");
            KafkaConsumer<?, ?> kc = new KafkaConsumer<>(props);

            //连接topic
            kc.subscribe(Collections.singletonList(topic));
            result = kc.partitionsFor(topic).size();
            log.debug("partitions = " + result);
        } catch (Throwable t) {
            log.error("initializing kafka client failed", t);
            throw new RuntimeException(t);
        }
        return result;
    }
}
```

KafkaConfig类在初始化阶段获取了指定topic的分区数，便于后续的多线程处理。同时创建了一个高度自定义的批量处理工厂Bean，其名称就是方法名“smartIdentifyBatchFactory”。下面是使用该处理工厂的方法：

PacketsListener.java

```java
@Component
@Slf4j
public class PacketsListener {
    @KafkaListener(id = "PacketsListener",topics = "smart-identify-packet", containerFactory = "smartIdentifyBatchFactory")
    public void listen(@Payload(required = false) List<ConsumerRecord<String, String>> records, Acknowledgment ack) {
```

可以看到PacketsListener类中的@KafkaListener使用了自定义的批量处理工厂Bean="smartIdentifyBatchFactory"。同时，这里的id用于注明该kafka监听器的ID，用于后续的生命周期管理。在其注解的方法中可以看到@Payload传递的就是一个`List<ConsumerRecord>`结构，而不是@StreamListener的单个对象。

## 控制生命周期

### 禁止自动启动

在smartIdentifyBatchFactory工厂方法中设置kafka禁止自启动即可：

```java
factory.setAutoStartup(false); // 禁止自动启动，用于主从切换，本地调试时可以修改这个配置
```

### 开启/停止消费

根据事件监听来开启KafkaListener。这里使用KafkaListenerEndpointRegistry。@KafkaListener这个注解所标注的方法并没有在IOC容器中注册为Bean，而是会被注册在KafkaListenerEndpointRegistry中，KafkaListenerEndpointRegistry在SpringIOC中已经被注册为Bean，因此kafka监听器受KafkaListenerEndpointRegistry管理。其逻辑在主从切换类MasterSlaveSwitching中实现。

MasterSlaveSwitching.java

```java
/**
 * Created by pengxingxiong@ruijie.com.cn on 2019/5/14 19:46
 * 负责主从切换逻辑
 */
@Service
@Slf4j
public class MasterSlaveSwitching {
    @Autowired
    private KafkaListenerEndpointRegistry kafkaListenerEndpointRegistry;// kafkaListener注册管理器
    @Autowired
    public Boolean isLeader = Boolean.FALSE;
    private String kafkaListenerID = "PacketsListener";

    /**
     * Created by pengxingxiong@ruijie.com.cn on 2019/5/14 21:20
     * 当前节点成为主节点逻辑
     *
     * @since 0.1
     */
    @EventListener(OnGrantedEvent.class)
    public synchronized void onApplicationEvent(OnGrantedEvent event) {
        isLeader = Boolean.TRUE;
        log.info("[terminal-smart-identify] : change to master!");
        //启动各个线程
        ...
        // 启动kafka消费者
        if (!kafkaListenerEndpointRegistry.getListenerContainer(kafkaListenerID).isRunning()) {
            kafkaListenerEndpointRegistry.getListenerContainer(kafkaListenerID).start();
        }
    }

    /**
     * Created by pengxingxiong@ruijie.com.cn on 2019/5/14 21:20
     * 当前节点成为从节点逻辑
     *
     * @since 0.1
     */
    @EventListener(OnRevokedEvent.class)
    public synchronized void onApplicationEvent(OnRevokedEvent event) {
        isLeader = Boolean.FALSE;
        log.info("[terminal-smart-identify] : change to slave!");
        //清理线程
        ...
        //结束kafka消费者
        if (kafkaListenerEndpointRegistry.getListenerContainer(kafkaListenerID).isRunning()) {
            kafkaListenerEndpointRegistry.getListenerContainer(kafkaListenerID).stop();
        }
    }
}
```

# 配置消息过滤器

## 消息过滤器

消息过滤器可以在消息抵达监听容器前被拦截，过滤器根据系统业务逻辑去筛选出需要的数据再交由KafkaListener处理。

配置消息其实是非常简单的额，只需要为监听容器工厂配置一个RecordFilterStrategy(消息过滤策略)，返回true的时候消息将会被抛弃，返回false时，消息能正常抵达监听容器。

这里我们将消息转换为long类型，判断该消息为基数还是偶数，把所有基数过滤，监听容器只接收偶数。

```java
@Component
public class FilterListener {

    private static final Logger log= LoggerFactory.getLogger(TaskListener.class);

    @Autowired
    private ConsumerFactory consumerFactory;

    @Bean
    public ConcurrentKafkaListenerContainerFactory filterContainerFactory() {
        ConcurrentKafkaListenerContainerFactory factory = new ConcurrentKafkaListenerContainerFactory();
        factory.setConsumerFactory(consumerFactory);
        //配合RecordFilterStrategy使用，被过滤的信息将被丢弃
        factory.setAckDiscarded(true);
        factory.setRecordFilterStrategy(new RecordFilterStrategy() {
            @Override
            public boolean filter(ConsumerRecord consumerRecord) {
                long data = Long.parseLong((String) consumerRecord.value());
                log.info("filterContainerFactory filter : "+data);
                if (data % 2 == 0) {
                    return false;
                }
                //返回true将会被丢弃
                return true;
            }
        });
        return factory;
    }

    @KafkaListener(id = "filterCons", topics = "topic.quick.filter",containerFactory = "filterContainerFactory")
    public void filterListener(String data) {
        //这里做数据持久化的操作
        log.error("topic.quick.filter receive : " + data);
    }
}
```

# 花式操作

##使用ConsumerRecord类消费

用ConsumerRecord类接收的好处是什么呢，ConsumerRecord类里面包含分区信息、消息头、消息体等内容，如果业务需要获取这些参数时，使用ConsumerRecord会是个不错的选择。如果使用具体的类型接收消息体则更加方便，比如说用String类型去接收消息体。

这里我们编写一个consumerListener方法，监听"topic.quick.consumer" Topic，并把ConsumerRecord里面所包含的内容打印到控制台中

```java
@Component
public class SingleListener {

    private static final Logger log = LoggerFactory.getLogger(SingleListener.class);

    @KafkaListener(id = "consumer", topics = "topic.quick.consumer")
    public void consumerListener(ConsumerRecord<Integer, String> record) {
        log.info("topic.quick.consumer receive : " + record.toString());
    }
}
```

编写测试方法，发送数据到对应的Topic中，运行测试我们可以看到控制台打印的日志，日志里面包含topic、partition、offset等信息，这其实就是完整的消息储存结构。

```java
@Test
public void testConsumerRecord() {
  kafkaTemplate.send("topic.quick.consumer", "test receive by consumerRecord");
}
2018-09-11 15:52:13.546  INFO 13644 --- [ consumer-0-C-1] com.viu.kafka.listen.SingleListener      : topic.quick.consumer receive : ConsumerRecord(topic = topic.quick.consumer, partition = 0, offset = 0, CreateTime = 1536652333476, serialized key size = -1, serialized value size = 30, headers = RecordHeaders(headers = [], isReadOnly = false), key = null, value = test receive by consumerRecord)
```

## 批量消费案例

1. 重新创建一份新的消费者配置，配置为一次拉取5条消息
2. 创建一个监听容器工厂，设置其为批量消费并设置并发量为5，这个并发量根据分区数决定，必须小于等于分区数，否则会有线程一直处于空闲状态
3. 创建一个分区数为8的Topic
4. 创建监听方法，设置消费id为batch，clientID前缀为batch，监听topic.quick.batch，使用batchContainerFactory工厂创建该监听容器

```java
@Component
public class BatchListener {

    private static final Logger log= LoggerFactory.getLogger(BatchListener.class);

    private Map<String, Object> consumerProps() {
        Map<String, Object> props = new HashMap<>();
        props.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, "localhost:9092");
        props.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, true);
        props.put(ConsumerConfig.AUTO_COMMIT_INTERVAL_MS_CONFIG, "1000");
        props.put(ConsumerConfig.SESSION_TIMEOUT_MS_CONFIG, "15000");
        //一次拉取消息数量
        props.put(ConsumerConfig.MAX_POLL_RECORDS_CONFIG, "5");
        props.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, IntegerDeserializer.class);
        props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        return props;
    }

    @Bean("batchContainerFactory")
    public ConcurrentKafkaListenerContainerFactory listenerContainer() {
        ConcurrentKafkaListenerContainerFactory container = new ConcurrentKafkaListenerContainerFactory();
        container.setConsumerFactory(new DefaultKafkaConsumerFactory(consumerProps()));
        //设置并发量，小于或等于Topic的分区数
        container.setConcurrency(5);
        //设置为批量监听
        container.setBatchListener(true);
        return container;
    }

    @Bean
    public NewTopic batchTopic() {
        return new NewTopic("topic.quick.batch", 8, (short) 1);
    }


    @KafkaListener(id = "batch",clientIdPrefix = "batch",topics = {"topic.quick.batch"},containerFactory = "batchContainerFactory")
    public void batchListener(List<String> data) {
        log.info("topic.quick.batch  receive : ");
        for (String s : data) {
            log.info(  s);
        }
    }

}
```

紧接着我们启动项目，控制台的日志信息非常完整，我们可以看到batchListener这个监听容器的partition分配信息。我们设置concurrency为5，也就是将会启动5条线程进行监听，那我们创建的topic则是有8个partition，意味着将有3条线程分配到2个partition和2条线程分配到1个partition。我们可以看到这段日志的最后5行，这就是每条线程分配到的partition。

```shellshell
2018-09-11 12:47:49.628  INFO 4708 --- [    batch-2-C-1] o.a.k.c.c.internals.AbstractCoordinator  : [Consumer clientId=batch-2, groupId=batch] Successfully joined group with generation 98
2018-09-11 12:47:49.628  INFO 4708 --- [    batch-2-C-1] o.a.k.c.c.internals.ConsumerCoordinator  : [Consumer clientId=batch-2, groupId=batch] Setting newly assigned partitions [topic.quick.batch-4, topic.quick.batch-5]
2018-09-11 12:47:49.630  INFO 4708 --- [    batch-3-C-1] o.a.k.c.c.internals.AbstractCoordinator  : [Consumer clientId=batch-3, groupId=batch] Successfully joined group with generation 98
2018-09-11 12:47:49.630  INFO 4708 --- [    batch-0-C-1] o.a.k.c.c.internals.AbstractCoordinator  : [Consumer clientId=batch-0, groupId=batch] Successfully joined group with generation 98
2018-09-11 12:47:49.630  INFO 4708 --- [    batch-4-C-1] o.a.k.c.c.internals.AbstractCoordinator  : [Consumer clientId=batch-4, groupId=batch] Successfully joined group with generation 98
2018-09-11 12:47:49.630  INFO 4708 --- [    batch-3-C-1] o.a.k.c.c.internals.ConsumerCoordinator  : [Consumer clientId=batch-3, groupId=batch] Setting newly assigned partitions [topic.quick.batch-6]
2018-09-11 12:47:49.630  INFO 4708 --- [    batch-0-C-1] o.a.k.c.c.internals.ConsumerCoordinator  : [Consumer clientId=batch-0, groupId=batch] Setting newly assigned partitions [topic.quick.batch-0, topic.quick.batch-1]
2018-09-11 12:47:49.630  INFO 4708 --- [    batch-4-C-1] o.a.k.c.c.internals.ConsumerCoordinator  : [Consumer clientId=batch-4, groupId=batch] Setting newly assigned partitions [topic.quick.batch-7]
2018-09-11 12:47:49.631  INFO 4708 --- [    batch-1-C-1] o.a.k.c.c.internals.AbstractCoordinator  : [Consumer clientId=batch-1, groupId=batch] Successfully joined group with generation 98
2018-09-11 12:47:49.631  INFO 4708 --- [    batch-1-C-1] o.a.k.c.c.internals.ConsumerCoordinator  : [Consumer clientId=batch-1, groupId=batch] Setting newly assigned partitions [topic.quick.batch-2, topic.quick.batch-3]
2018-09-11 12:47:49.633  INFO 4708 --- [    batch-3-C-1] o.s.k.l.KafkaMessageListenerContainer    : partitions assigned: [topic.quick.batch-6]
2018-09-11 12:47:49.633  INFO 4708 --- [    batch-0-C-1] o.s.k.l.KafkaMessageListenerContainer    : partitions assigned: [topic.quick.batch-0, topic.quick.batch-1]
2018-09-11 12:47:49.633  INFO 4708 --- [    batch-4-C-1] o.s.k.l.KafkaMessageListenerContainer    : partitions assigned: [topic.quick.batch-7]
2018-09-11 12:47:49.633  INFO 4708 --- [    batch-1-C-1] o.s.k.l.KafkaMessageListenerContainer    : partitions assigned: [topic.quick.batch-2, topic.quick.batch-3]
2018-09-11 12:47:49.634  INFO 4708 --- [    batch-2-C-1] o.s.k.l.KafkaMessageListenerContainer    : partitions assigned: [topic.quick.batch-4, topic.quick.batch-5]
```

那我们来编写一下测试方法，在短时间内发送12条消息到topic中，可以看到运行结果，对应的监听方法总共拉取了三次数据，其中两次为5条数据，一次为2条数据，加起来就是我们在测试方法发送的12条数据。证明我们的批量消费方法是按预期进行的。

```java
@Autowired
private KafkaTemplate kafkaTemplate;

@Test
public void testBatch() {
  for (int i = 0; i < 12; i++) {
    kafkaTemplate.send("topic.quick.batch", "test batch listener,dataNum-" + i);
  }
}
2018-09-11 12:08:51.840  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch  receive : 
2018-09-11 12:08:51.840  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-5
2018-09-11 12:08:51.840  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-2
2018-09-11 12:08:51.840  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-10
2018-09-11 12:08:51.840  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-6
2018-09-11 12:08:51.840  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-3
2018-09-11 12:08:51.841  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch  receive : 
2018-09-11 12:08:51.841  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-11
2018-09-11 12:08:51.841  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-0
2018-09-11 12:08:51.841  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-8
2018-09-11 12:08:51.841  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-7
2018-09-11 12:08:51.841  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-4
2018-09-11 12:08:51.842  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch  receive : 
2018-09-11 12:08:51.842  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-1
2018-09-11 12:08:51.842  INFO 12416 --- [    batch-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-9
```

<font color="red">注意：设置的并发量不能大于partition的数量，如果需要提高吞吐量，可以通过增加partition的数量达到快速提升吞吐量的效果。</font>

## 监听Topic中指定的分区

紧接着刚才编写的代码里面编写新的监听器，第一眼看到这代码，妈呀，这注解这么长，哈哈哈，我也不是故意的啊。
 这里使用@KafkaListener注解的topicPartitions属性监听不同的partition分区。
 @TopicPartition：topic--需要监听的Topic的名称，partitions --需要监听Topic的分区id，
 partitionOffsets --可以设置从某个偏移量开始监听
 @PartitionOffset：partition --分区Id，非数组，initialOffset --初始偏移量

```java
@Bean
public NewTopic batchWithPartitionTopic() {
  return new NewTopic("topic.quick.batch.partition", 8, (short) 1);
}

@KafkaListener(id = "batchWithPartition",clientIdPrefix = "bwp",containerFactory = "batchContainerFactory",
               topicPartitions = {
                 @TopicPartition(topic = "topic.quick.batch.partition",partitions = {"1","3"}),
                 @TopicPartition(topic = "topic.quick.batch.partition",partitions = {"0","4"},
                                 partitionOffsets = @PartitionOffset(partition = "2",initialOffset = "100"))
               }
              )
public void batchListenerWithPartition(List<String> data) {
  log.info("topic.quick.batch.partition  receive : ");
  for (String s : data) {
    log.info(s);
  }
}
```

其实和我们刚才写的批量消费区别只是在注解上多了个属性，启动项目我们仔细搜索一下控制台输出的日志，如果存在该日志则说明成功。同样的我们往这个Topic里面写入一些数据，运行后我们可以看到控制台只监听到一部分消息，这是因为创建的Topic的partition数量为8，而我们只监听了0、1、2、3、4这几个partition，也就是说5 6  7这三个分区的消息我们并没有读取出来。

```java
2018-09-11 14:39:52.045  INFO 12412 --- [Partition-4-C-1] o.a.k.c.consumer.internals.Fetcher       : [Consumer clientId=bwp-4, groupId=batchWithPartition] Fetch offset 100 is out of range for partition topic.quick.batch-2, resetting offset
    @Test
    public void testBatch() throws InterruptedException {
        for (int i = 0; i < 12; i++) {
            kafkaTemplate.send("topic.quick.batch.partition", "test batch listener,dataNum-" + i);
        }
    }
2018-09-11 14:51:09.063  INFO 1532 --- [Partition-2-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.063  INFO 1532 --- [Partition-2-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-4
2018-09-11 14:51:09.064  INFO 1532 --- [Partition-1-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.064  INFO 1532 --- [Partition-1-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-2
2018-09-11 14:51:09.075  INFO 1532 --- [Partition-0-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.075  INFO 1532 --- [Partition-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-1
2018-09-11 14:51:09.078  INFO 1532 --- [Partition-1-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.078  INFO 1532 --- [Partition-1-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-10
2018-09-11 14:51:09.091  INFO 1532 --- [Partition-4-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.091  INFO 1532 --- [Partition-4-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-5
2018-09-11 14:51:09.095  INFO 1532 --- [Partition-0-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.096  INFO 1532 --- [Partition-0-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-9
2018-09-11 14:51:09.097  INFO 1532 --- [Partition-3-C-1] com.viu.kafka.listen.BatchListener       : topic.quick.batch.partition  receive : 
2018-09-11 14:51:09.098  INFO 1532 --- [Partition-3-C-1] com.viu.kafka.listen.BatchListener       : test batch listener,dataNum-7
```

##注解方式获取消息头及消息体

当你接收的消息包含请求头，以及你监听方法需要获取该消息非常多的字段时可以通过这种方式，毕竟get方法代码量还是稍多点的。这里使用的是默认的监听容器工厂创建的，如果你想使用批量消费，把对应的类型改为List即可，比如`List<String> data` ，` List<Integer> key`。

- @Payload：获取的是消息的消息体，也就是发送内容
- @Header(KafkaHeaders.RECEIVED_MESSAGE_KEY)：获取发送消息的key
- @Header(KafkaHeaders.RECEIVED_PARTITION_ID)：获取当前消息是从哪个分区中监听到的
- @Header(KafkaHeaders.RECEIVED_TOPIC)：获取监听的TopicName
- @Header(KafkaHeaders.RECEIVED_TIMESTAMP)：获取时间戳

```java
@KafkaListener(id = "anno", topics = "topic.quick.anno")
public void annoListener(@Payload String data,
                         @Header(KafkaHeaders.RECEIVED_MESSAGE_KEY) Integer key,
                         @Header(KafkaHeaders.RECEIVED_PARTITION_ID) int partition,
                         @Header(KafkaHeaders.RECEIVED_TOPIC) String topic,
                         @Header(KafkaHeaders.RECEIVED_TIMESTAMP) long ts) {
  log.info("topic.quick.anno receive : \n"+
           "data : "+data+"\n"+
           "key : "+key+"\n"+
           "partitionId : "+partition+"\n"+
           "topic : "+topic+"\n"+
           "timestamp : "+ts+"\n"
          );

}
```

监听容器编写好了，那就写个测试方法测试一下。启动测试后可以看到监听方法成功的把我们所需要的数据提取出来了，说明这段代码也是ojbk的。

```java
@Test
public void testAnno() throws InterruptedException {
  Map map = new HashMap<>();
  map.put(KafkaHeaders.TOPIC, "topic.quick.anno");
  map.put(KafkaHeaders.MESSAGE_KEY, 0);
  map.put(KafkaHeaders.PARTITION_ID, 0);
  map.put(KafkaHeaders.TIMESTAMP, System.currentTimeMillis());

  kafkaTemplate.send(new GenericMessage<>("test anno listener", map));
}
2018-09-11 15:27:47.108  INFO 7592 --- [     anno-0-C-1] com.viu.kafka.listen.SingleListener      : topic.quick.anno receive : 
data : test anno listener
key : 0
partitionId : 0
topic : topic.quick.anno
timestamp : 1536650867015
```

##使用Ack机制确认消费

Kafka的Ack机制相对于RabbitMQ的Ack机制差别比较大，刚入门Kafka的时候我也被搞蒙了，不过能弄清楚Kafka是怎么消费消息的就能理解Kafka的Ack机制了

我先说说RabbitMQ的Ack机制，RabbitMQ的消费可以说是一次性的，也就是你确认消费后就立刻从硬盘或内存中删除，而且RabbitMQ粗糙点来说是顺序消费，像排队一样，一个个顺序消费，未被确认的消息则会重新回到队列中，等待监听器再次消费。
 但Kafka不同，Kafka是通过最新保存偏移量进行消息消费的，而且确认消费的消息并不会立刻删除，所以我们可以重复的消费未被删除的数据，当第一条消息未被确认，而第二条消息被确认的时候，Kafka会保存第二条消息的偏移量，也就是说第一条消息再也不会被监听器所获取，除非是根据第一条消息的偏移量手动获取。
 

使用Kafka的Ack机制比较简单，只需简单的三步即可：

1. 设置ENABLE_AUTO_COMMIT_CONFIG=false，禁止自动提交
2. 设置AckMode=MANUAL_IMMEDIATE
3. 监听方法加入Acknowledgment ack 参数

怎么拒绝消息呢，只要在监听方法中不调用ack.acknowledge()即可

```java
@Component
public class AckListener {

    private static final Logger log= LoggerFactory.getLogger(AckListener.class);

    private Map<String, Object> consumerProps() {
        Map<String, Object> props = new HashMap<>();
        props.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, "localhost:9092");
        props.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, false);
        props.put(ConsumerConfig.AUTO_COMMIT_INTERVAL_MS_CONFIG, "1000");
        props.put(ConsumerConfig.SESSION_TIMEOUT_MS_CONFIG, "15000");
        props.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, IntegerDeserializer.class);
        props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        return props;
    }

    @Bean("ackContainerFactory")
    public ConcurrentKafkaListenerContainerFactory ackContainerFactory() {
        ConcurrentKafkaListenerContainerFactory factory = new ConcurrentKafkaListenerContainerFactory();
        factory.setConsumerFactory(new DefaultKafkaConsumerFactory(consumerProps()));
        factory.getContainerProperties().setAckMode(AbstractMessageListenerContainer.AckMode.MANUAL_IMMEDIATE);
        factory.setConsumerFactory(new DefaultKafkaConsumerFactory(consumerProps()));
        return factory;
    }


    @KafkaListener(id = "ack", topics = "topic.quick.ack",containerFactory = "ackContainerFactory")
    public void ackListener(ConsumerRecord record, Acknowledgment ack) {
        log.info("topic.quick.ack receive : " + record.value());
        ack.acknowledge();
    }
}
```

编写测试方法，运行后可以方法监听方法能收到消息，紧接着注释ack.acknowledge()方法，重新测试，同样你会发现监听容器能接收到消息，这个时候如果你重启项目还是可以看到未被确认的那几条消息。

```java
@Test
public void testAck() throws InterruptedException {
  for (int i = 0; i < 5; i++) {
    kafkaTemplate.send("topic.quick.ack", i+"");
  }
}
```

在这段章节开头之初我就讲解了Kafka机制会出现的一些情况，导致没办法重复消费未被Ack的消息，解决办法有如下：

1. 重新将消息发送到队列中，这种方式比较简单而且可以使用Headers实现第几次消费的功能，用以下次判断

```java
@KafkaListener(id = "ack", topics = "topic.quick.ack", containerFactory = "ackContainerFactory")
public void ackListener(ConsumerRecord record, Acknowledgment ack, Consumer consumer) {
  log.info("topic.quick.ack receive : " + record.value());

  //如果偏移量为偶数则确认消费，否则拒绝消费
  if (record.offset() % 2 == 0) {
    log.info(record.offset()+"--ack");
    ack.acknowledge();
  } else {
    log.info(record.offset()+"--nack");
    kafkaTemplate.send("topic.quick.ack", record.value());
  }
}
```

1. 使用Consumer.seek方法，重新回到该未ack消息偏移量的位置重新消费，这种可能会导致死循环，原因出现于业务一直没办法处理这条数据，但还是不停的重新定位到该数据的偏移量上。

```java
@KafkaListener(id = "ack", topics = "topic.quick.ack", containerFactory = "ackContainerFactory")
public void ackListener(ConsumerRecord record, Acknowledgment ack, Consumer consumer) {
  log.info("topic.quick.ack receive : " + record.value());

  //如果偏移量为偶数则确认消费，否则拒绝消费
  if (record.offset() % 2 == 0) {
    log.info(record.offset()+"--ack");
    ack.acknowledge();
  } else {
    log.info(record.offset()+"--nack");
    consumer.seek(new TopicPartition("topic.quick.ack",record.partition()),record.offset() );
  }
}
```

# 实现消息转发以及ReplyTemplate

##目的

软件需要使用什么技术都是按照业务逻辑来的嘛，那自动转发相对应的业务可以是什么呢？

可以使用转发功能实现业务解耦，系统A从Topic-A中获取到消息，进行处理后转发到Topic-B中，系统B监听Topic-B获取消息再次进行处理，这个消息可以是订单相关数据，系统A处理用户提交的订单审核，系统B处理订单的物流信息等等。

##实现方式

Spring-Kafka整合了两种消息转发方式：

1. 使用Headers设置回复主题（Reply_Topic），这种方式比较特别，是一种请求响应模式，使用的是ReplyingKafkaTemplate类
2. 手动转发，使用@SendTo注解将监听方法返回值转发到Topic中

### @SendTo方式

SendTo的方式可以说是非常简单的，给我三秒钟，不不不，男人不可以这么快，三个钟好了，嘿嘿嘿。

1. 配置ConcurrentKafkaListenerContainerFactory的ReplyTemplate
2. 监听方法加上@SendTo注解
3. 还想有第三步？回家睡觉吧

**KafkaConfiguration.class**

这里我们为监听容器工厂(ConcurrentKafkaListenerContainerFactory)配置一个ReplyTemplate，ReplyTemplate是我们用来转发消息所使用的类。@SendTo注解本质其实就是利用这个ReplyTemplate转发监听方法的返回值到对应的Topic中，我们也可以是用代码实现KakfaTemplate.send()，不过使用注解的好处就是减少代码量，加快开发效率。

```java
@Bean
public ConcurrentKafkaListenerContainerFactory<Integer, String> kafkaListenerContainerFactory() {
  ConcurrentKafkaListenerContainerFactory<Integer, String> factory = new ConcurrentKafkaListenerContainerFactory<>();
  factory.setConsumerFactory(consumerFactory());
  factory.setReplyTemplate(kafkaTemplate());
  return factory;
}
@Component
public class ForwardListener {

  private static final Logger log= LoggerFactory.getLogger(ForwardListener.class);

  @KafkaListener(id = "forward", topics = "topic.quick.target")
  @SendTo("topic.quick.real")
  public String forward(String data) {
    log.info("topic.quick.target  forward "+data+" to  topic.quick.real");
    return "topic.quick.target send msg : " + data;
  }
}
```

顺便就写个测试方法测试一下吧，可以看到运行成功后，topic.quick.real这个主题会产生一条数据，这条数据就是我们在forward方法返回的值。

```java
@Autowired
private KafkaTemplate kafkaTemplate;

@Test
public void testForward() {
  kafkaTemplate.send("topic.quick.target", "test @SendTo");
}
```

###ReplyTemplate方式

使用ReplyTemplate方式不同于@SendTo方式，@SendTo是直接将监听方法的返回值转发对应的Topic中，而ReplyTemplate也是将监听方法的返回值转发Topic中，但转发Topic成功后，会被请求者消费。

这是怎么回事呢？我们可以回想一下请求响应模式，这种模式其实我们是经常使用的，就像你调用某个第三方接口，这个接口会把响应报文返回给你，你可以根据业务处理这段响应报文。而ReplyTemplate方式的这种请求响应模式也是相同的，首先生成者发送消息到Topic-A中，Topic-A的监听器则会处理这条消息，紧接着将消息转发到Topic-B中，当这条消息转发到Topic-B成功后则会被ReplyTemplate接收。那最终消费者获得的是被处理过的数据。

ReplyTemplate实现的代码也并不复杂，实现的功能确更多。

讲一下流程吧：

1. 配置ConcurrentKafkaListenerContainerFactory的ReplyTemplate
2. 配置topic.quick.request的监听器
3. 注册一个KafkaMessageListenerContainer类型的监听容器，监听topic.quick.reply，这个监听器里面我们不处理任何事情，交由ReplyingKafkaTemplate处理
4. 通过ProducerFactory和KafkaMessageListenerContainer创建一个ReplyingKafkaTemplate类型的Bean，设置回复超时时间为10秒

```java
@Bean
public ConcurrentKafkaListenerContainerFactory<Integer, String> kafkaListenerContainerFactory() {
  ConcurrentKafkaListenerContainerFactory<Integer, String> factory = new ConcurrentKafkaListenerContainerFactory<>();
  factory.setConsumerFactory(consumerFactory());
  factory.setReplyTemplate(kafkaTemplate());
  return factory;
}
@KafkaListener(id = "replyConsumer", topics = "topic.quick.request",containerFactory = "kafkaListenerContainerFactory")
@SendTo
public String replyListen(String msgData){
  log.info("topic.quick.request receive : "+msgData);
  return "topic.quick.reply  reply : "+msgData;
}

@Bean
public KafkaMessageListenerContainer<String, String> replyContainer(@Autowired ConsumerFactory consumerFactory) {
  ContainerProperties containerProperties = new ContainerProperties("topic.quick.reply");
  return new KafkaMessageListenerContainer<>(consumerFactory, containerProperties);
}
@Bean
public ReplyingKafkaTemplate<String, String, String> replyingKafkaTemplate(@Autowired ProducerFactory producerFactory, KafkaMessageListenerContainer replyContainer) {
  ReplyingKafkaTemplate template = new ReplyingKafkaTemplate<>(producerFactory, replyContainer);
  template.setReplyTimeout(10000);
  return template;
}
```

发送消息就显得稍微有点麻烦了，不过在项目编码过程中可以把它封装成一个工具类调用。

1. 我们需要创建ProducerRecord类，用来发送消息，并添加KafkaHeaders.REPLY_TOPIC到record的headers参数中，这个参数配置我们想要转发到哪个Topic中。
2. 使用replyingKafkaTemplate.sendAndReceive()方法发送消息，该方法返回一个Future类RequestReplyFuture，这里类里面包含了获取发送结果的Future类和获取返回结果的Future类。使用replyingKafkaTemplate发送及返回都是异步操作。
3. 调用RequestReplyFuture.getSendFutrue().get()方法可以获取到发送结果
4. 调用RequestReplyFuture.get()方法可以获取到响应结果

```java
@Autowired
private ReplyingKafkaTemplate replyingKafkaTemplate;

@Test
public void testReplyingKafkaTemplate() throws ExecutionException, InterruptedException, TimeoutException {
  ProducerRecord<String, String> record = new ProducerRecord<>("topic.quick.request", "this is a message");
  record.headers().add(new RecordHeader(KafkaHeaders.REPLY_TOPIC, "topic.quick.reply".getBytes()));
  RequestReplyFuture<String, String, String> replyFuture = replyingKafkaTemplate.sendAndReceive(record);
  SendResult<String, String> sendResult = replyFuture.getSendFuture().get();
  System.out.println("Sent ok: " + sendResult.getRecordMetadata());
  ConsumerRecord<String, String> consumerRecord = replyFuture.get();
  System.out.println("Return value: " + consumerRecord.value());
  Thread.sleep(20000);
}
```

<font color="red">注意：由于ReplyingKafkaTemplate也是通过监听容器实现的，所以响应时间可能会较慢，要注意选择合适的场景使用。</font>