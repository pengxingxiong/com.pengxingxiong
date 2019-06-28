# zookeeper

解压kafka的安装包

# 配置zoo.cfg文件

配置zookeeper.properties

```shell
cd kafka_2.11-2.0.0/config/
vim zookeeper.properties
```

修改内容为：

```shell
dataDir=/tmp/zookeeper
# the port at which the clients will connect
clientPort=2181
# disable the per-ip limit on the number of connections since this is a non-production config
maxClientCnxns=1200
tickTime=5000
initLimit=50
syncLimit=20
autopurge.snapRetainCount=3
autopurge.purgeInterval=0
# 路径分析白名单
4lw.commands.whitelist=*
server.1=172.18.135.131:2888:3888
server.2=172.18.135.132:2888:3888
server.3=172.18.135.133:2888:3888
```

再将kafka的解压文件夹发送到各个节点

```shell
scp -r kafka_2.11-2.0.0/ root@rgibns2:/opt/kafka
scp -r kafka_2.11-2.0.0/ root@rgibns3:/opt/kafka
```

##新建myid文件

修改/tmp/zookeeper的myid文件，按照在zookeeper.properties文件中定义的服务器id分别写入1、2、3。

```shell
mkdir /tmp/zookeeper
vim /tmp/zookeeper/myid
```

##配置环境变量

```shell
vim /etc/profile 
```

在最后添加如下两个:

```shell
# zookeeper
export ZOOKEEPER_HOME=/opt/kafka/kafka_2.11-2.0.0
export PATH=PATH:ZOOKEEPER_HOME/bin:$ZOOKEEPER_HOME/conf
```

保存后进入cd /etc目录下，输入source profile命令使修改生效。

```shell
source /etc/profile
```

##启动zookeeper

分别在3台机器上/opt/zookeeper-3.4.5/bin目录下启动；

```shell
# 启动
./zookeeper-server-start.sh ../config/zookeeper.properties
# 后台启动
./zookeeper-server-start.sh ../config/zookeeper.properties &
# 重启
zkServer.sh restart
# 查看状态
zkServer.sh status
# 关闭
zkServer.sh stop
# 以打印日志方式启动
zkServer.sh start-foreground
```

# kafka

修改配置文件

```shell
vim /opt/kafka/kafka_2.11-2.0.0/config/server.properties
```



修改这两项，能够简单实用

```shell
broker.id=3
listeners=PLAINTEXT://172.18.135.131:9092
zookeeper.connect=172.18.135.131:2181,172.18.135.132:2181,172.18.135.133:2181
zookeeper.connection.timeout.ms=60000
```

启动命令

```sh
./kafka-server-start.sh ../config/server.properties &
```

关闭命令

```shell
./kafka-server-stop.sh ../config/server.properties &
```

