```java
StateFactory factory = HbaseFactory.getOptionsConf();
TridentTopology topology = new TridentTopology();
//写入流
topology.newStream("spout1", kafkaSpout)
  .partitionPersist(factory, fields, new HBaseUpdater(), new Fields());
//读取流
TridentState state = topology.newStaticState(factory);
topology.newDRPCStream("drpc-test")
  .each(new Fields("args"), new wordSplit(), new Fields("word"))
  .groupBy(new Fields("word"))
  .stateQuery(state, new Fields("word"), new HBaseQuery(), new Fields("columnName", "columnValue"))
  .each(new Fields("word", "columnValue"), new ReportFunction(), new Fields());
return topology.build();
```

在写入流的partitionPersist方法处理后就创建了一个state，尽管没有任何显式的创建爱你state。因为partitionPersist方法中创建了StateSpec对象，且stateId是每次+1,也就是闯将了state1。