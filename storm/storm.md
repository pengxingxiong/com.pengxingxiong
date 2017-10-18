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












































































  [1]: ./images/1508294473965.jpg
  [2]: ./images/1508294569869.jpg