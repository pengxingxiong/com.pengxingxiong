# Nimbus keeps reporting KeyNotFoundException in logs
```bash
2018-03-05 19:53:07.990 o.a.s.d.nimbus [INFO] ExceptionKeyNotFoundException(msg:1310047_topo_0305_same_copy-74-1520238831-stormconf.ser)
2018-03-05 19:53:08.007 o.a.s.d.nimbus [INFO] ExceptionKeyNotFoundException(msg:1310040_same_topo_0305_wyjtest2-68-1520212690-stormjar.jar)
```
这个问题导致的原因暂且判断是由于storm中删除拓扑过快，使用1秒删除，所以zookeeper删除和storm本地删除文件不同步导致的，也就是没有找到对应的key。一般都是因为storm本地文件未来得及删除。
错误方式是将zookeeper和storm本地文件（相关节点的workers-artifacts目录）删除即可。亲测无效。从真正的原因来分析，这个日志一直存在，主要是存在对应的worker节点，把worker节点进程杀死即可。
## storm UI 显示了拓扑，但是没有组件
该原因请先查看worker/slot是否还有剩余，当资源耗尽之后，拓扑虽然提交成功了，但是无法申请到资源，故而无法生成组件。
## java.net.SocketException: Broken pipe (Write failed)
有一种缓存，存了各种drpc客户端对象，现在把drpc进程给杀死，则缓存里的drpc客户端失效，会报以上错误。
## KeeperErrorCode = NoNode for /meta/766189
```java
java.lang.RuntimeException: java.lang.RuntimeException: org.apache.storm.shade.org.apache.zookeeper.KeeperException$NoNodeException: KeeperErrorCode = NoNode for /meta/766189
java.lang.RuntimeException: org.apache.storm.shade.org.apache.zookeeper.KeeperException$NodeExistsException: KeeperErrorCode = NodeExists for /meta/769074
```
Storm的作者回答是：nathanmarz Do you have multiple topologies using the same id for the spout? Those ids need to be globally unique.
可见，对于topology.newStream()，需要传入一个唯一的SpoutName，如果出现大量相同的txid，那么就会报错。
