##查看使用zookeeper的进程

```sh
netstat -nape | awk '{if($5 =="172.18.130.28:2181")print $4, $9;}'
```



## ZooKeeper常用四字命令：

传递四个字母的字符串给ZooKeeper，会返回一些有用的信息。

| 命令 | 功能描述                                                     |
| ---- | ------------------------------------------------------------ |
| conf | 输出相关服务配置的详细信息。                                 |
| cons | 列出所有连接到服务器的客户端的完全的连接 /会话的详细信息。包括“接受/发送”的包数量、会话id 、操作延迟、最后的操作执行等等信息。 |
| dump | 列出未经处理的会话和临时节点。                               |
| envi | 输出关于服务环境的详细信息（区别于conf命令）。               |
| reqs | 列出未经处理的请求                                           |
| ruok | 测试服务是否处于正确状态。如果确实如此，那么服务返回“imok ”，否则不做任何相应。 |
| stat | 输出关于性能和连接的客户端的列表。                           |
| wchs | 列出服务器 watch的详细信息。                                 |
| wchc | 通过Session列出服务器watch的详细信息，它的输出是一个与watch相关的会话的列表。 |
| wchp | 通过路径列出服务器watch的详细信息。它输出一个与Session相关的路径。 |

ZooKeeper支持某些特定的四字命令字母与其的交互。它们大多是查询命令，用来获取ZooKeeper服务的当前状态及相关信息。用户在客户端可以通过telnet或nc向ZooKeeper提交相应的命令。

下面是常用命令，其他命令替换关键字即可：

查看哪个节点被选择作为follower或者leader

```
echo stat | nc localhost 2181
```

列出所有连接到服务器的客户端的完全的连接/会话的详细信息

```sh
echo envi | nc localhost 2181
```

测试是否启动了该Server，若回复imok表示已经启动

```
echo ruok | nc localhost 2181
```

**列出所有连接到服务器的客户端的完全的连接/会话的详细信息**

```
echo cons | nc localhost 2181
```

# Zookeeper 监控原型开发

## 设计
这里我采用socket的方式： 

- 向指定ip和port 发送4 letter words commands 
- 接收返回的数据信息 
- 将返回的数据信息解析与划分（源数据信息是一堆信息） 
- 将解析划分的带有一定格式的处理后的数据返回（这就是我们要的监控指标了）

## 监控指标

下面我们来梳理一下返回了哪些监控指标：

```
conf:
clientPort:客户端端口号 
dataDir：数据文件目录
dataLogDir：日志文件目录  
tickTime：间隔单位时间
maxClientCnxns：最大连接数  
minSessionTimeout：最小session超时
maxSessionTimeout：最大session超时  
serverId：id  
initLimit：初始化时间  
syncLimit：心跳时间间隔  
electionAlg：选举算法 默认3  
electionPort：选举端口  
quorumPort：法人端口  
peerType：未确认

cons：
ip=ip
port=端口
queued=所在队列
received=收包数
sent=发包数
sid=session id
lop=最后操作
est=连接时间戳
to=超时时间
lcxid=最后id(未确认具体id)
lzxid=最后id(状态变更id)
lresp=最后响应时间戳
llat=最后/最新 延时
minlat=最小延时
maxlat=最大延时
avglat=平均延时


crst:
重置所有连接


dump:
session id : znode path  (1对多   ,  处于队列中排队的session和临时节点)


envi:
zookeeper.version=版本
host.name=host信息
java.version=java版本
java.vendor=供应商
java.home=jdk目录
java.class.path=classpath
java.library.path=lib path
java.io.tmpdir=temp目录
java.compiler=<NA>
os.name=Linux
os.arch=amd64
os.version=2.6.32-358.el6.x86_64
user.name=hhz
user.home=/home/hhz
user.dir=/export/servers/zookeeper-3.4.6


ruok:
查看server是否正常
imok=正常


srst:
重置server状态


srvr：
Zookeeper version:版本
Latency min/avg/max: 延时
Received: 收包
Sent: 发包
Connections: 连接数
Outstanding: 堆积数
Zxid: 操作id
Mode: leader/follower
Node count: 节点数

stat：
Zookeeper version: 3.4.6-1569965, built on 02/20/2014 09:09 GMT
Clients:
 /192.168.147.102:56168[1](queued=0,recved=41,sent=41)
 /192.168.144.102:34378[1](queued=0,recved=54,sent=54)
 /192.168.162.16:43108[1](queued=0,recved=40,sent=40)
 /192.168.144.107:39948[1](queued=0,recved=1421,sent=1421)
 /192.168.162.16:43112[1](queued=0,recved=54,sent=54)
 /192.168.162.16:43107[1](queued=0,recved=54,sent=54)
 /192.168.162.16:43110[1](queued=0,recved=53,sent=53)
 /192.168.144.98:34702[1](queued=0,recved=41,sent=41)
 /192.168.144.98:34135[1](queued=0,recved=61,sent=65)
 /192.168.162.16:43109[1](queued=0,recved=54,sent=54)
 /192.168.147.102:56038[1](queued=0,recved=165313,sent=165314)
 /192.168.147.102:56039[1](queued=0,recved=165526,sent=165527)
 /192.168.147.101:44124[1](queued=0,recved=162811,sent=162812)
 /192.168.147.102:39271[1](queued=0,recved=41,sent=41)
 /192.168.144.107:45476[1](queued=0,recved=166422,sent=166423)
 /192.168.144.103:45100[1](queued=0,recved=54,sent=54)
 /192.168.162.16:43133[0](queued=0,recved=1,sent=0)
 /192.168.144.107:39945[1](queued=0,recved=1825,sent=1825)
 /192.168.144.107:39919[1](queued=0,recved=325,sent=325)
 /192.168.144.106:47163[1](queued=0,recved=17891,sent=17891)
 /192.168.144.107:45488[1](queued=0,recved=166554,sent=166555)
 /172.17.36.11:32728[1](queued=0,recved=54,sent=54)
 /192.168.162.16:43115[1](queued=0,recved=54,sent=54)

Latency min/avg/max: 0/0/599
Received: 224869
Sent: 224817
Connections: 23
Outstanding: 0
Zxid: 0x68000af707
Mode: follower
Node count: 101081

（同上面的命令整合的信息）


wchs:
connectsions=连接数
watch-paths=watch节点数
watchers=watcher数量


wchc:
session id 对应 path

wchp:
path 对应 session id

mntr:
zk_version=版本
zk_avg_latency=平均延时
zk_max_latency=最大延时
zk_min_latency=最小延时
zk_packets_received=收包数  
zk_packets_sent=发包数
zk_num_alive_connections=连接数
zk_outstanding_requests=堆积请求数
zk_server_state=leader/follower 状态
zk_znode_count=znode数量
zk_watch_count=watch数量
zk_ephemerals_count=临时节点（znode）
zk_approximate_data_size=数据大小
zk_open_file_descriptor_count=打开的文件描述符数量
zk_max_file_descriptor_count=最大文件描述符数量
zk_followers=follower数量
zk_synced_followers=同步的follower数量
zk_pending_syncs=准备同步数
```

## 项目

目前写了一个demo的原型出来，完成上面所说的设计流程。 
github：https://github.com/hackerwin7/zookeeper-four-letter

### 使用方式
安装：源码用maven编译，到target下找到相应jar包。 
使用（两种使用方式）： 
基于命令行：

```sh
java -cp zookeeper-four-letter-1.0.jar pers.hw7.zk.monitor.zookeeper.FourLetterCommands 192.168.144.110 2181 mntr
```


基于API：

```java
import pers.hw7.zk.monitor.deployer.Controller;
import pers.hw7.zk.monitor.utils.metrics.MntrMetrics;

public class ControllerTest {
    public static void main(String[] args) throws Exception {
        Controller cont = new Controller("192.168.144.110", 2181);
        MntrMetrics metrics = cont.getMNTR();
        System.out.println(metrics.toString());

    }
}
```

这样就能获得相应的指标了。

数据格式
一共有13个 监控指标 ， 代码中以类似ConfMetrics这样的类存储详细的指标信息： 
- 普通的指标信息获取如: metrics.avgLatency 这样直接获取普通变量 
- 具有1对多的连接信息的： 一般以 List 或者 Map

