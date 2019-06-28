### **前言：**

监控与troubleshooting永远是任何数据库最需要关注的地方之一。
任何数据库的DBA都应该对数据库情况进行定期的巡检，以清楚了解数据库的运行情况，健康状况，隐患等等。

本文主要讲述的是如何使用Mtools工具对MongoDB数据库进行巡检。
通过mtools并结合监控数据，如MMS,ZABBIX,或者自行绘制的监控图形，能够较好的对mongodb的运行情况，健康状况进行检查，并排除隐患。且在troubleshooting的时候也能提供很好的帮助。

### **Mtools简介：**

Mtools是由MongoDB Inc 官方工程师所写，设计之初是为了方便自己的工作，但是随着MongoDB用户的增加，越来越多的朋友也开始使用Mtools，也越来越感受到Mtools带来的便捷。
Github地址如下：
[Mtools github地址](https://github.com/rueckstiess/mtools)

Mtools主要有以下组件：

```undefined
mlogfilter
mloginfo
mplotqueries
mlogvis
mlaunch
mgenerate
```

在此就不一一介绍了，本文中主要使用到的是 mlogfilter,mloginfo和mplotqueries.

### **前菜：**

首先，我们来简单介绍下 mlogfilter,mloginfo和mplotqueries。
mlogfileter我们可以简单理解为日志的过滤器，参数如下，就不一一简介了

```undefined
mlogfilter [-h] [--version] logfile [logfile ...]
           [--verbose] [--shorten [LENGTH]]
           [--human] [--exclude] [--json]
           [--timestamp-format {ctime-pre2.4, ctime, iso8601-utc, iso8601-local}]
           [--markers MARKERS [MARKERS ...]] [--timezone N [N ...]]
           [--namespace NS] [--operation OP] [--thread THREAD]
           [--slow [SLOW]]  [--fast [FAST]] [--scan]
           [--word WORD [WORD ...]]
           [--from FROM [FROM ...]] [--to TO [TO ...]]
```

mloginfo可以过滤总结出slow query的情况，以及为日志中各类最常常出现情况进行统计，参数如下：

```undefined
mloginfo [-h] [--version] logfile
         [--verbose] 
         [--queries] [--restarts] [--distinct] [--connections] [--rsstate]
```

mplotqueries相对复杂一些，功能是可以根据需求画图，以便更直观的找出问题或者隐患所在。
具体用法参见github中的详情页，为了方便大家阅读，我也在这转发一个小伙伴翻译的中文版本
[mplotqueries 中文翻译 ](http://my.oschina.net/phptiger86/blog/349503#OSC_h2_1)
特此感谢 吕明明的翻译。

参数如下：

```undefined
mplotqueries [-h] [--version] logfile [logfile ...]
             [--group GROUP]
             [--logscale]
             [--type {nscanned/n,rsstate,connchurn,durline,histogram,range,scatter,event} ]
             [--overlay [ {add,list,reset} ]]
             [additional plot type parameters]
```

### **主菜(21道)：**

由于本文主要讲述的是通过Mtools来辅助我们定期巡检，且主打“干货”，我将在下面直接进行一系列的使用分享。

#### 1.通过Mlogfilter 列出日志文件中所有的slow log（以json格式输出）

```undefined
mlogfilter mongod.log-20150721 --slow --json
```

#### 2.将通过mlogfilter输出的所有slow log入库

```undefined
mlogfilter mongod.log-20150721 --slow --json |mongoimport -d test -c mycoll
```

#### 3.通过mlogfilter查询日志中某个表的slow log(超过100ms的)

```undefined
mlogfilter --namespace xxx.xx --slow 100 mongod.log-20150721
```

#### 4.通过mloginfo统计日志中各类信息的distinct

```undefined
mloginfo mongod.log-20150721 --distinct
```

#### 5.通过mloginfo统计日志中connections的来源情况

```undefined
mloginfo mongod.log-20150721 --connections
```

#### 6.通过mloginfo查看日志中所记录的复制集状态变更（如果有的话）

```undefined
mloginfo mongod.log-20150721 --rsstate
```

#### 7.通过mloginfo统计查看日志中慢查询的分类

```undefined
mloginfo --queries mongod.log-20150721
```

[![1-1](http://www.mongoing.com/wp-content/uploads/2015/07/1-1-300x170.png)](http://www.mongoing.com/wp-content/uploads/2015/07/1-1.png)

 

#### 8.通过mplotqueries进行慢查询散点分布图绘制（请原谅我这个图打码技术有限）

```undefined
mplotqueries mongod.log-20150721 --output-file 01-2.png
```

[![1-2](http://www.mongoing.com/wp-content/uploads/2015/07/1-2-300x200.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-2.jpg)

#### 9.通过mplotqueries进行慢查询散点分布图绘制，且只返回前10个

```undefined
mplotqueries mongod.log-20150721 --output-file 01-3.png --group-limit 10
```

[![1-3](http://www.mongoing.com/wp-content/uploads/2015/07/1-3-300x200.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-3.jpg)

#### 10.上一个图中，我们可以发现底部的数据难以看清，我们可以使用对数模式

```undefined
mplotqueries mongod.log-20150721 --output-file 01-4.png --logscale --group-limit 10
```

[![1-4](http://www.mongoing.com/wp-content/uploads/2015/07/1-4-300x200.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-4.jpg)

#### 11.仅看日志中某一个表的慢查询散点分布情况

```undefined
mlogfilter mongod.log-20150721 --namespace xx.xxx |mplotqueries --output-file 01-5.png
```

[![1-5](http://www.mongoing.com/wp-content/uploads/2015/07/1-5-300x199.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-5.jpg)

#### 12.通过mplotqueries来对日志中的慢查询进行操作类型分布

```undefined
mplotqueries mongod.log-20150721 --group operation --output-file 01-6.png
```

[![1-6](http://www.mongoing.com/wp-content/uploads/2015/07/1-6-300x200.png)](http://www.mongoing.com/wp-content/uploads/2015/07/1-6.png)

#### 13.通过mplotqueries对日志中的慢查询进行扫表情况绘图

```undefined
mplotqueries mongod.log-20150721 --type nscanned/n --output-file 01-7.png
```

[![1-7](http://www.mongoing.com/wp-content/uploads/2015/07/1-7-300x200.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-7.jpg)

#### 14.自定义y轴内容，这里以w为例（nscanned, nupdated,ninserted, ntoreturn, nreturned, numYields, r (读锁), w (写锁)）

```undefined
mplotqueries mongod.log-20150721 --yaxis w --output-file 01-8.png
```

[![1-8](http://www.mongoing.com/wp-content/uploads/2015/07/1-8-300x200.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-8.jpg)

#### 15.通过mplotqueries对连接情况进行分析，时间块单位1800（30min）

```undefined
mplotqueries mongod.log-20150721 --type connchurn --bucketsize 1800 --output-file 01-9.png
```

[![1-9](http://www.mongoing.com/wp-content/uploads/2015/07/1-9-300x198.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-9.jpg)

#### 16.通过mlogfileter过滤出xx.xxx的update，然后每30min时间块 以_id分布（这里可以引申为任何条件支持正则）

```undefined
mlogfilter mongod.log-20150721 --operation update --namespace xx.xxx | mplotqueries --type histogram --group "_id:([^,}]*)" --bucketsize 1800 --output-file 01-10.png --group-limit 20
```

[![1-10](http://www.mongoing.com/wp-content/uploads/2015/07/1-10-300x200.png)](http://www.mongoing.com/wp-content/uploads/2015/07/1-10.png)

#### 17.查看每小时的insert情况

```undefined
mlogfilter mongod.log-20150721 --operation insert | mplotqueries --type histogram --bucketsize 3600 --output-file 01-11.png
```

[![1-11](http://www.mongoing.com/wp-content/uploads/2015/07/1-11-300x201.jpg)](http://www.mongoing.com/wp-content/uploads/2015/07/1-11.jpg)

#### 18.对日志中的slow log进行分布绘图，（出现间隔超过10min时，显示间隔）

```undefined
mplotqueries mongod.log-20150721 --type range --group operation --gap 600 --output-file 01-12.png
```

[![1-12](http://www.mongoing.com/wp-content/uploads/2015/07/1-12-300x200.png)](http://www.mongoing.com/wp-content/uploads/2015/07/1-12.png)

#### 19.对日志中的事件进行绘图（事件图，显示出各类事件出现的时间位置等）。 由于测试数据中无getlasterror类型error，故暂无图示

```undefined
grep "getlasterror" mongod.log-20150721 | mplotqueries --type event --output-file 01-13.png
```

#### 20.通过log进行复制集状态的查看。（会绘制出复制集状态变动图形）

```undefined
mplotqueries mongod.log-20150721 --type rsstate --output-file 01-14.png
```

#### 21.overlay的使用

```undefined
overlay参数可以将多组图进行重叠，便于troubleshooting时候的快速分析。
建立overlay仅需加参数--overlay
查看overlay可以使用--overlay list
清空overlay可以使用--overlay reset
```

### **饭后甜点：**

有了这些数据我们如何进行分析呢？
首先需要知道，一切异于常态的状态都是值得关注的，如8图中有一些点完全高于其他点，这就是可以重点关注从此下手进行调查。
再者mloginfo查询出的slow log 类型结合我们mongo本身记录的 profile 也可以一起对业务语句进行很好的分析，查询执行计划，并进行优化。
结合mtools所绘的图片，并将系统数据（cpu,io,mem,进程资源情况）的数据监控图进行重叠比较，也能够为troubleshooting提供很好的帮助。