# storm 框架
# 1 Storm的特点
Storm可用于许多领域中，如实时分析、在线机器学习、持续计算、远程RPC、数据提取加载转换等

Storm具有以下主要特点：
- 整合性：Storm可方便地与队列系统和数据库系统进行整合
- 简易的API：Storm的API在使用上即简单又方便
- 可扩展性：Storm的并行特性使其可以运行在分布式集群中
- 容错性：Storm可自动进行故障节点的重启、任务的重新分配
- 可靠的消息处理：Storm保证每个消息都能完整处理
- 支持各种编程语言：Storm支持使用各种编程语言来定义任务
- 快速部署：Storm可以快速进行部署和使用
- 免费、开源：Storm是一款开源框架，可以免费使用

# 2 Storm设计思想
- Storm主要术语包括Streams、Spouts、Bolts、Topology和Stream Groupings
## 2.1 Streams
Storm将流数据Stream描述成一个无限的Tuple序列，这些Tuple序列会以分布式的方式并行地创建和处理。


![Streams][1]


## 2.2 tuple
- 每个tuple是一堆值，每个值有一个名字，并且每个值可以是任何类型
- Tuple本来应该是一个Key-Value的Map，由于各个组件间传递的tuple的字段名称已经事先定义好了，所以Tuple只需要按序填入各个Value，所以就是一个Value List（值列表）

![Value List][2]
## 2.3 Spout
- Storm认为每个Stream都有一个源头，并把这个源头抽象为Spout
- 通常Spout会从外部数据源（队列、数据库等）读取数据，然后封装成Tuple形式，发送到Stream中。Spout是一个主动的角色，在接口内部有个nextTuple函数，Storm框架会不停的调用该函数
- 
![Spout][3]

## 2.4 Bolt
- Storm将Streams的状态转换过程抽象为Bolt。Bolt即可以处理Tuple，也可以将处理后的Tuple作为新的Streams发送给其他Bolt
- Bolt可以执行过滤、函数操作、Join、操作数据库等任何操作
- Bolt是一个被动的角色，其接口中有一个execute(Tuple input)方法，在接收到消息之后会调用此函数，用户可以在此方法中执行自己的处理逻辑

![Bolt][4]

## 2.5 Topology
- Storm将Spouts和Bolts组成的网络抽象成Topology，它可以被提交到Storm集群执行。Topology可视为流转换图，图中节点是一个Spout或Bolt，边则表示Bolt订阅了哪个Stream。当Spout或者Bolt发送元组时，它会把元组发送到每个订阅了该Stream的Bolt上进行处理
- Topology里面的每个处理组件（Spout或Bolt）都包含处理逻辑， 而组件之间的连接则表示数据流动的方向
- Topology里面的每一个组件都是并行运行的
- 在Topology里面可以指定每个组件的并行度， Storm会在集群里面分配那么多的线程来同时计算
- 在Topology的具体实现上，Storm中的Topology定义仅仅是一些Thrift结构体（二进制高性能的通信中间件），支持各种编程语言进行定义

![enter description here][5]

## 2.6 Stream Groupings
Storm中的Stream Groupings用于告知Topology如何在两个组件间（如Spout和Bolt之间，或者不同的Bolt之间）进行Tuple的传送。每一个Spout和Bolt都可以有多个分布式任务，一个任务在什么时候、以什么方式发送Tuple就是由Stream Groupings来决定的

![enter description here][6]

目前，Storm中的Stream Groupings有如下几种方式：
(1)ShuffleGrouping：随机分组，随机分发Stream中的Tuple，保证每个Bolt的Task接收Tuple数量大致一致

(2)FieldsGrouping：按照字段分组，保证相同字段的Tuple分配到同一个Task中

(3)AllGrouping：广播发送，每一个Task都会收到所有的Tuple

(4)GlobalGrouping：全局分组，所有的Tuple都发送到同一个Task中

(5)NonGrouping：不分组，和ShuffleGrouping类似，当前Task的执行会和它的被订阅者在同一个线程中执行

(6)DirectGrouping：直接分组，直接指定由某个Task来执行Tuple的处理


# 3 Storm框架设计
- Storm运行任务的方式与Hadoop类似：Hadoop运行的是MapReduce作业，而Storm运行的是“Topology”
- 但两者的任务大不相同，主要的不同是：MapReduce作业最终会完成计算并结束运行，而Topology将持续处理消息（直到人为终止）


![Storm和Hadoop架构组件功能对应关系][7]

- Storm集群采用“Master—Worker”的节点方式：
（1） Master节点运行名为“Nimbus”的后台程序（类似Hadoop中的“JobTracker”），负责在集群范围内分发代码、为Worker分配任务和监测故障
（2） Worker节点运行名为“Supervisor”的后台程序，负责监听分配给它所在机器的工作，即根据Nimbus分配的任务来决定启动或停止Worker进程，一个Worker节点上同时运行若干个Worker进程
- Storm使用Zookeeper来作为分布式协调组件，负责Nimbus和多个Supervisor之间的所有协调工作。借助于Zookeeper，若Nimbus进程或Supervisor进程意外终止，重启时也能读取、恢复之前的状态并继续工作，使得Storm极其稳定

![Storm集群架构示意图][8]

## Worker、Executor和Task的关系
- worker:每个worker进程都属于一个特定的Topology，每个Supervisor节点的worker可以有多个，每个worker对Topology中的每个组件（Spout或 Bolt）运行一个或者多个executor线程来提供task的运行服务
- executor：executor是产生于worker进程内部的线程，会执行同一个组件的一个或者多个task。
- task:实际的数据处理由task完成，在Topology的生命周期中，每个组件的task数目是不会发生变化的，而executor的数目却不一定。executor数目小于等于task的数目，默认情况下，二者是相等的

![Worker、Executor和Task的关系][9]

## Storm工作流程
- 基于这样的架构设计，Storm的工作流程如下图所示：
- 
![enter description here][10]

- 所有Topology任务的提交必须在Storm客户端节点上进行，提交后，由Nimbus节点分配给其他Supervisor节点进行处理
- Nimbus节点首先将提交的Topology进行分片，分成一个个Task，分配给相应的Supervisor，并将Task和Supervisor相关的信息提交到Zookeeper集群上
- Supervisor会去Zookeeper集群上认领自己的Task，通知自己的Worker进程进行Task的处理

说明：在提交了一个Topology之后，Storm就会创建Spout/Bolt实例并进行序列化。之后，将序列化的组件发送给所有的任务所在的机器(即Supervisor节点)，在每一个任务上反序列化组件

# 4 Spark Streaming与Storm的对比
- Spark Streaming和Storm最大的区别在于，Spark Streaming无法实现毫秒级的流计算，而Storm可以实现毫秒级响应
- Spark Streaming构建在Spark上，一方面是因为Spark的低延迟执行引擎（100ms+）可以用于实时计算，另一方面，相比于Storm，RDD数据集更容易做高效的容错处理
- Spark Streaming采用的小批量处理的方式使得它可以同时兼容批量和实时数据处理的逻辑和算法，因此，方便了一些需要历史数据和实时数据联合分析的特定应用场合

# 5 Storm、Spark Streaming选型
- 从编程的灵活性来讲，Storm是比较理想的选择，它使用Apache Thrift，可以用任何编程语言来编写拓扑结构（Topology）
- 当需要在一个集群中把流计算和图计算、机器学习、SQL查询分析等进行结合时，可以选择Spark Streaming，因为，在Spark上可以统一部署Spark SQL，Spark Streaming、MLlib，GraphX等组件，提供便捷的一体化编程模型
- 当应用场景需要毫秒级响应时，可以选择Storm，因为Spark Streaming无法实现毫秒级的流计算

# 6 编写Storm程序
-基于Storm的单词统计在形式上与基于MapReduce的单词统计是类似的，MapReduce使用的是Map和Reduce的抽象，而Storm使用的是Spout和Bolt的抽象
## 程序任务：单词统计
- Storm进行单词统计的整个流程：
（1）从Spout中发送Stream（每个英文句子为一个Tuple）
（2）用于分割单词的Bolt将接收的句子分解为独立的单词，将单词作为Tuple的字段名发送出去
（3）用于计数的Bolt接收表示单词的Tuple，并对其进行统计
（4）输出每个单词以及单词出现过的次数

![一个句子经Storm的单词统计得出的结果][11]

Storm的编程模型非常简单，如下代码即定义了整个单词统计Topology的整体逻辑
```java
import org.apache.storm.Config;
Import ……
public class WordCountTopology {
	public  static class RandomSentenceSpout extends BaseRichSpout {
		……
	}
	public static class SplitSentence extends ShellBolt implements IRichBolt {
		……
	}
	public static class WordCount extends BaseBasicBolt {
		……
	}
	public static void main(String[] args) throws Exception {
		TopologyBuilder builder = new TopologyBuilder();
		builder.setSpout("sentences", new RandomSentenceSpout(), 5);
		builder.setBolt("split", new SplitSentence(), 8).shuffleGrouping("sentences");
		builder.setBolt("count", new WordCount(), 12).fieldsGrouping("split", new Fields("word"));
		……
	}
}
```
## main()函数中的处理逻辑

![main()函数中的处理逻辑][12]

Topology中仅定义了整体的计算逻辑，还需要定义具体的处理函数。具体的处理函数可以使用任一编程语言来定义，甚至也可以结合多种编程语言来实现

### RandomSentenceSpout类
```java
//为简单起见，RandomSentenceSpout省略了类中的一些方法
public class RandomSentenceSpout extends BaseRichSpout {
	SpoutOutputCollector _collector;
	Random _rand;
	@Override
	  public void nextTuple() {
		Utils.sleep(100);
		String[] sentences = new String[]{ "the cow jumped over the moon", "an apple a day keeps the doctor away",
		        "four score and seven years ago", "snow white and the seven dwarfs", "i am at two with nature" };
		String sentence = sentences[_rand.nextint(sentences.length)];
		_collector.emit(new Values(sentence));
	}
	@Override
	  public void declareOutputFields(OutputFieldsDeclarer declarer) {
		declarer.declare(new Fields(“sentences"));
  }
}
```
### SplitSentence类
- 如SplitSentence()方法虽然是通过Java语言定义的，但具体的操作可通过Python脚本来完成
- Topology里面的每个组件必须定义它要发射的Tuple的每个字段

![SplitSentence类][13]

### splitsentence.py
Python脚本splitsentence.py定义了一个简单的单词分割方法，即通过空格来分割单词。分割后的单词通过emit()方法以Tuple的形式发送给订阅了该Stream的Bolt进行接收和处理

![splitsentence.py][14]
### WordCount类 
单词统计的具体逻辑：首先判断单词是否统计过，若未统计过，需先将count值置为0。若单词已统计过，则每出现一次该单词，count值就加1

![WordCount类][15]

上述虽然是一个简单的单词统计，但对其进行扩展，便可应用到许多场景中，如微博中的实时热门话题。Twitter也正是使用了Storm框架实现了实时热门话题

![Twitter实时热门话题处理流程示意图][16]


  [1]: ./images/1508294473965.jpg
  [2]: ./images/1508294569869.jpg
  [3]: ./images/1508294704398.jpg
  [4]: ./images/1508294871536.jpg
  [5]: ./images/1508295518783.jpg
  [6]: ./images/1508295552998.jpg
  [7]: ./images/1508295866173.jpg
  [8]: ./images/1508296082262.jpg
  [9]: ./images/1508296208499.jpg
  [10]: ./images/1508296278042.jpg
  [11]: ./images/1508296784833.jpg
  [12]: ./images/1508297142533.jpg
  [13]: ./images/1508297471900.jpg
  [14]: ./images/1508297560657.jpg
  [15]: ./images/1508297594602.jpg
  [16]: ./images/1508297712133.jpg