# kafka
## 1.配置文件
184环境中查看opt/kafka/config/server.properties文件中关于zookeeper位置的描述
```bash
zookeeper.connect=emr-header-1:2181,emr-header-2:2181,emr-header-3:2181
```
可以发现zookeeper位于这三个服务器上
## 2.查看主题
进入184环境
```bash
bin/kafka-topics.sh --list --zookeeper emr-header-1:2181
```
该命令将显示kafka所有主题
## 开启服务
由于在服务器环境中zookeeper和kafka是独立的，但是kafka中又带有一个zookeeper的配置，因此只要启动一个zookeeper就可以(最好不要用kafka自带的zookeeper)，这个能够通过进程看，因此这里只需要启动kafka
```bash
bin/kafka-server-start.sh -daemon config/server.properties
```
## 创建主题
当没有test主题时
```bash
bin/kafka-topics.sh --create --zookeeper emr-header-1:2181 --replication-factor 1 --partitions 1 --topic test
```
## 3.测试
开两个窗口，一个生产者，一个消费者，测试生产者产生的消息能否给消费者收到
### 生产者
```bash
bin/kafka-console-producer.sh --broker-list emr-worker-2:9092 --topic test
```
### 消费者
```bash
bin/kafka-console-consumer.sh --zookeeper emr-header-1:2181 --topic test --from-beginning
```
# hdfs
## 配置hdfs
```java
import org.apache.storm.Config;
import org.apache.storm.LocalCluster;
import org.apache.storm.StormSubmitter;
import org.apache.storm.generated.AlreadyAliveException;
import org.apache.storm.generated.AuthorizationException;
import org.apache.storm.generated.InvalidTopologyException;
import org.apache.storm.hdfs.bolt.HdfsBolt;
import org.apache.storm.hdfs.bolt.format.DefaultFileNameFormat;
import org.apache.storm.hdfs.bolt.format.DelimitedRecordFormat;
import org.apache.storm.hdfs.bolt.format.FileNameFormat;
import org.apache.storm.hdfs.bolt.format.RecordFormat;
import org.apache.storm.hdfs.bolt.rotation.FileRotationPolicy;
import org.apache.storm.hdfs.bolt.rotation.TimedRotationPolicy;
import org.apache.storm.hdfs.bolt.sync.CountSyncPolicy;
import org.apache.storm.hdfs.bolt.sync.SyncPolicy;
import org.apache.storm.kafka.*;
import org.apache.storm.spout.SchemeAsMultiScheme;
import org.apache.storm.task.OutputCollector;
import org.apache.storm.task.TopologyContext;
import org.apache.storm.topology.OutputFieldsDeclarer;
import org.apache.storm.topology.TopologyBuilder;
import org.apache.storm.topology.base.BaseRichBolt;
import org.apache.storm.tuple.Fields;
import org.apache.storm.tuple.Tuple;
import org.apache.storm.tuple.Values;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.Collections;
import java.util.Map;

import static org.apache.storm.Config.NIMBUS_HOST;
import static org.apache.storm.Config.NIMBUS_SEEDS;

/**
 * @author pengxingxiong(0016004591) 2018/1/8
 */
public class Kafka2Hdfs {
    private static final String NIMBUS_HOST = "10.206.19.228";
    private static final String ZOOKEEPER_HOST = "10.206.19.228";
    private static final String HDFS_HOST = "hdfs://10.206.19.228:9000";
    private static final String STORM_JAR_PATH = "target/test.storm-1.0-SNAPSHOT.jar";
    public static class KafkaWordToUpperCase extends BaseRichBolt {

        private static final Logger LOG = LoggerFactory.getLogger(KafkaWordToUpperCase.class);
        private static final long serialVersionUID = -5207232012035109026L;
        private OutputCollector collector;

        @Override
        public void prepare(Map stormConf, TopologyContext context, OutputCollector collector) {
            this.collector = collector;
        }

        @Override
        public void execute(Tuple input) {
            String line = input.getString(0).trim();
            LOG.info("RECV[kafka -> splitter] " + line);
            if (!line.isEmpty()) {
                String upperLine = line.toUpperCase();
                LOG.info("EMIT[splitter -> counter] " + upperLine);
                collector.emit(input, new Values(upperLine, upperLine.length()));
            }
            collector.ack(input);
        }

        @Override
        public void declareOutputFields(OutputFieldsDeclarer declarer) {
            declarer.declare(new Fields("line", "len"));
        }

    }

    public static class RealtimeBolt extends BaseRichBolt {

        private static final Logger LOG = LoggerFactory.getLogger(RealtimeBolt.class);
        private static final long serialVersionUID = -4115132557403913367L;
        private OutputCollector collector;

        @Override
        public void prepare(Map stormConf, TopologyContext context,
                            OutputCollector collector) {
            this.collector = collector;
        }

        @Override
        public void execute(Tuple input) {
            String line = input.getString(0).trim();
            LOG.info("REALTIME: " + line);
            collector.ack(input);
        }

        @Override
        public void declareOutputFields(OutputFieldsDeclarer declarer) {

        }

    }

    public static void main(String[] args) throws Exception {
        String zks = ZOOKEEPER_HOST+":2181";
        String topic = "test";
        String zkRoot = "/brokers/topics"; // default zookeeper root configuration for storm
        String id = "test1";
        BrokerHosts brokerHosts = new ZkHosts(zks);
        SpoutConfig spoutConf = new SpoutConfig(brokerHosts, topic, zkRoot, id);
        spoutConf.scheme = new SchemeAsMultiScheme(new StringScheme());
        // spoutConf.forceFromStart = false;
        spoutConf.zkServers = Collections.singletonList(ZOOKEEPER_HOST);
        spoutConf.zkPort = 2181;

        // Configure HDFS bolt
        RecordFormat format = new DelimitedRecordFormat(); // use "\t" instead of "," for field delimiter
        SyncPolicy syncPolicy = new CountSyncPolicy(1000); // sync the filesystem after every 1k tuples
        FileRotationPolicy rotationPolicy = new TimedRotationPolicy(1.0f, TimedRotationPolicy.TimeUnit.MINUTES); // rotate files
        String hdfsDirCorrect = "/myfolder/pengxx";
        String hdfsDirIncorrect = "/user/hive/warehouse/pengxx";
        FileNameFormat fileNameFormat = new DefaultFileNameFormat().withPath(hdfsDirIncorrect).withPrefix("app_").withExtension(".log");
        HdfsBolt hdfsBolt = new HdfsBolt()
                .withFsUrl(HDFS_HOST)
                .withFileNameFormat(fileNameFormat)
                .withRecordFormat(format)
                .withRotationPolicy(rotationPolicy)
                .withSyncPolicy(syncPolicy);

        // configure & build topology
        TopologyBuilder builder = new TopologyBuilder();
        builder.setSpout("kafka-reader", new KafkaSpout(spoutConf));
        builder.setBolt("to-upper", new KafkaWordToUpperCase()).shuffleGrouping("kafka-reader");
        builder.setBolt("hdfs-bolt", hdfsBolt).shuffleGrouping("to-upper");
        builder.setBolt("realtime", new RealtimeBolt()).shuffleGrouping("to-upper");

        // submit topology
        Config conf = new Config();
        String name = Kafka2Hdfs.class.getSimpleName();

        //选择模式
        //clienttoServer(conf,name,builder);

        //localTest(conf,name,builder);
        clusterSubmit(conf,name,builder);


    }
    /**采用远程提交的方式，需要先打成jar包，然后本地运行main函数，注意打包时需要对storm-core依赖使用provided，而运行main时则注释provided*/
    private static void clienttoServer(Config conf,String topoName,TopologyBuilder builder) throws Exception {
        conf.put(NIMBUS_SEEDS, Collections.singletonList(NIMBUS_HOST));
        //conf.put(Config.NIMBUS_THRIFT_PORT,);//配置nimbus连接端口，默认 6627
        conf.put(Config.STORM_ZOOKEEPER_SERVERS, Collections.singletonList(ZOOKEEPER_HOST)); //配置zookeeper连接主机地址，可以使用集合存放多个
        //非常关键的一步，使用StormSubmitter提交拓扑时，不管怎么样，都是需要将所需的jar提交到nimbus上去，如果不指定jar文件路径,
        //storm默认会使用System.getProperty("storm.jar")去取，如果不设定，就不能提交
        System.setProperty("storm.jar",STORM_JAR_PATH);
        StormSubmitter.submitTopology(topoName, conf, builder.createTopology());
    }
    /**
     * 本地测试
     * @author pengxingxiong
     * */
    private static void localTest(Config conf,String topoName,TopologyBuilder builder) throws Exception {
        LocalCluster cluster = new LocalCluster();
        cluster.submitTopology(topoName, conf, builder.createTopology());
    }
    /**
     * 打包为集群模式使用的jar
     * @author pengxingxiong
     * */
    private static void clusterSubmit(Config conf,String topoName,TopologyBuilder builder) throws Exception {
        StormSubmitter.submitTopologyWithProgressBar(topoName, conf, builder.createTopology());
    }
}
```
需要配置的有读取kafka日志的kafkaSpout的SpoutConfig和HdfsBolt。其他的就是自定义的Bolt。

## storm提交模式
### 1 本地模式
#### 关于日志
```java
    private static void localTest(Config conf,String topoName,TopologyBuilder builder) throws Exception {
        LocalCluster cluster = new LocalCluster();
        cluster.submitTopology(topoName, conf, builder.createTopology());
    }
```
这个模式中需要注意的是日志的级别控制，因为storm会打很多zookeeper的日志。storm用slfj对log4j2进行了封装，因此要修改日志属性，就要模仿storm-core.jar包中的log4j2.xml。在自己的resource中将storm-core.jar/log4j2.xml复制进来，稍作修改：
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!--
 Licensed to the Apache Software Foundation (ASF) under one or more
 contributor license agreements.  See the NOTICE file distributed with
 this work for additional information regarding copyright ownership.
 The ASF licenses this file to You under the Apache License, Version 2.0
 (the "License"); you may not use this file except in compliance with
 the License.  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->

<configuration monitorInterval="60">
  <Appenders>
    <Console name="Console" target="SYSTEM_OUT">
     <PatternLayout pattern="%-4r [%t] %-5p %c{1.} - %msg%n"/>
    </Console>
  </Appenders>
  <Loggers>
    <Logger name="org.apache.zookeeper" level="ERROR"/>
    <Logger name="org.apache.storm.shade.org.apache" level="ERROR"/>
      <Logger name="org.apache.storm" level="ERROR"/>
    <Root level="INFO">
      <AppenderRef ref="Console"/>
    </Root>
  </Loggers>
</configuration>
```
我直接将zookeeper级别设置为ERROR级别了，另外还有些其他不重要的日志，如org.apache.storm前缀的，都可以将级别提高，毕竟我们只是想打印我们自定义的日志而已。
#### 关于hdfs问题
当本地测试时出现hdfs错误：
```bash
java.lang.RuntimeException: Error preparing HdfsBolt: No FileSystem for scheme: hdfs  
        at org.apache.storm.hdfs.bolt.AbstractHdfsBolt.prepare(AbstractHdfsBolt.java:109) ~[stormjar.jar:na]  
        ...
```
>参考： [Storm-HDFS整合过程中问题解决](http://blog.csdn.net/u014039577/article/details/49818935)
将打包模式换成maven-shade-plugin，涉及到配置文件的问题，如果想配置的更加完整的话，可以这样，在github的apache-storm的master分支中有example包，其中就有关于整合hdfs的例子，然后我们把它的pom.xml文件中的maven-shade-plugin配置复制过来就可以了，但是有个问题需要注意的是，要加上mainClass，即在`<mainClass>`标签中加入你的main函数入口类：
```xml
......
<transformers>
    <transformer
            implementation="org.apache.maven.plugins.shade.resource.ServicesResourceTransformer"/>
    <transformer
            implementation="org.apache.maven.plugins.shade.resource.ManifestResourceTransformer">
        <mainClass>nubia.kafka2hdfs.Kafka2Hdfs</mainClass>
    </transformer>
</transformers>
......
```
### 2 集群提交
参考代码
```java
private static void clusterSubmit(Config conf,String topoName,TopologyBuilder builder) throws Exception {
    StormSubmitter.submitTopologyWithProgressBar(topoName, conf, builder.createTopology());
}
```
需要注意的是，如果只是这样将jar包提交那么会涉及到几个问题：

#### 1 default.yaml文件重复
由于在集群环境中，已经有了default.yaml文件了，而打包时依赖的storm-core.jar也包含有default.yaml文件，那么直接提交是会报文件重复的错误。需要在pom.xml使用`<scope>`标签。
```xml
<dependency>
    <groupId>org.apache.storm</groupId>
    <artifactId>storm-core</artifactId>
    <version>${storm.version}</version>
    <!--storm-core.jar中存在yam文件，如果提交到集群，会和集群的yam文件冲突，所以要去掉-->
    <!--本地调试的时候，屏蔽掉scope，等打包部署的时候再放开-->
    <scope>provided</scope>
</dependency>
```
也就是说，打包的时候不把这个storm-core.jar一起打包，但是本地模式的时候需要这个包。
#### 2 hdfs问题
打包插件maven-shade-plugin使用的打包命令应该用mvn package。

当使用mvn shade:shade命令打包时会报错：
```
hade (default-cli) on project test.storm: Failed to create shaded artifact, project main artifact does not exist.
```
这个错误让人很头疼，明明已经写了main入口了呀。所以这里请注意下。

最后这个jar包就可以放入到集群环境中提交了
### 3 远程提交
由于业务原因，很多时候，需要从API或者第三方位置提交storm的jar文件，或者觉得每次打包一个jar然后复制到linux服务器，再使用命令行提交很麻烦。这个时候有两种场景：
1. 从本地的编译软件（如IDEA）将jar提交到storm集群；
2. 仅仅使用storm/bin中的storm脚本（不开启storm服务）将jar提交到storm集群。

#### 1 从本地编译软件提交jar
```java
/**采用远程提交的方式，需要先打成jar包，然后本地运行main函数，注意打包时需要对storm-core依赖使用provided，而运行main时则注释provided*/
private static void clienttoServer(Config conf,String topoName,TopologyBuilder builder) throws Exception {
    conf.put(NIMBUS_SEEDS, Collections.singletonList(NIMBUS_HOST));
    //conf.put(Config.NIMBUS_THRIFT_PORT,);//配置nimbus连接端口，默认 6627
    conf.put(Config.STORM_ZOOKEEPER_SERVERS, Collections.singletonList(ZOOKEEPER_HOST)); //配置zookeeper连接主机地址，可以使用集合存放多个
    //非常关键的一步，使用StormSubmitter提交拓扑时，不管怎么样，都是需要将所需的jar提交到nimbus上去，如果不指定jar文件路径,
    //storm默认会使用System.getProperty("storm.jar")去取，如果不设定，就不能提交
    System.setProperty("storm.jar",STORM_JAR_PATH);
    StormSubmitter.submitTopology(topoName, conf, builder.createTopology());
}
```
如代码所示，需要首先本地打包为jar，然后本地执行该代码提交jar。需要注意的点是打包时候要对storm-core依赖使用`<scope>provide</scope>`，但是本地执行代码时需要注释`<scope>`标签。

#### 2 使用storm脚本提交
1.将打包好的jar放到linux相关目录，如/data/storm/test.storm-1.0-SNAPSHOT.jar;

2.编写storm.yaml
```bash
storm.zookeeper.servers:
     - "10.206.19.228"
nimbus.seeds: [ "10.206.19.228" ]
```
仅仅配置远程storm集群的zookeeper和nimbus的host即可。

3.提交jar
```bash
storm --config /data/storm/storm.yaml jar /data/storm/test.storm-1.0-SNAPSHOT.jar nubia.kafka2hdfs.Kafka2Hdfs
```
最后观察hdfs相应目录是否产生文件，还有需要对kafka建立一个针对test主题的生产者来产生测试数据供该拓扑消费。
# storm
## 1.提交storm
```bash
./storm jar /home/pengxx/javaProject/storm/test-storm-1.0.jar simple.WordCountTopology
```
如果是普通用户直接用storm命令，如果是root用户，需要到storm/bin目录下。

# root账户问题
提交topology时发现无论是linux的root账户还是普通用户，提交之后再UI界面看到的Owner都是root，但是查看代码（Nimbus.submitTopologyWithOpts）发现都是指示了用户为当前使用的用户
```java
String submitterUser = principalToLocal.toLocal(principal);
String systemUser = System.getProperty("user.name");
......
String topologyOwner = Utils.OR(submitterUser, systemUser);
```
但是经过调查发现，topology.submitter.user值仅仅与开启storm服务的用户有关，而与谁提交了拓扑无关，当需要做一些权限校验时，明显不能使用这个字段，但是我们发现在linux下观察拓扑名相对于的进程时，用户时提交拓扑的用户，可以在此处思考怎么利用起来校验
