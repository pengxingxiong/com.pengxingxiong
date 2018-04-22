hive之于数据民工，就如同锄头之于农民伯伯。hive用的好，才能从地里（数据库）里挖出更多的数据来。用过hive的朋友，我想或多或少都有类似的经历：一天下来，没跑几次hive，就到下班时间了。hive在极大数据或者数据不平衡等情况下，表现往往一般，因此也出现了presto、spark-sql等替代品。今天不谈其它，就来说说关于hive，个人的一点心得。
# 1 表连接优化 #

1.将大表放后头

Hive假定查询中最后的一个表是大表。它会将其它表缓存起来，然后扫描最后那个表。因此通常需要将小表放前面，或者标记哪张表是大表：`/*streamtable(table_name) */`
2.使用相同的连接键

当对3个或者更多个表进行join连接时，如果每个on子句都使用相同的连接键的话，那么只会产生一个MapReduce job。

3.尽量尽早地过滤数据

减少每个阶段的数据量,对于分区表要加分区，同时只选择需要使用到的字段。

4.尽量原子化操作

尽量避免一个SQL包含复杂逻辑，可以使用中间表来完成复杂的逻辑

# 2 用insert into替换union all #
如果union all的部分个数大于2，或者每个union部分数据量大，应该拆成多个insert into 语句，实际测试过程中，执行时间能提升50%。如：

```sql
insert overwite table tablename partition (dt= ....) 　
select ..... from ( select ... from A 
union all 　
select ... from B 　union all select ... from C ) R 　
where ...;
```
可以改写为：

```sql
insert into table tablename partition (dt= ....) select .... from A WHERE ...;
insert into table tablename partition (dt= ....) select .... from B 　WHERE ...;
insert into table tablename partition (dt= ....) select .... from C WHERE ...;
```
# 3 order by & sort by #
order by：对查询结果进行全局排序，消耗时间长。需要`set hive.mapred.mode=nostrict`
sort by : 局部排序，并非全局有序，提高效率。
# 4 transform+python #
一种嵌入在hive取数流程中的自定义函数，通过transform语句可以把在hive中不方便实现的功能在python中实现，然后写入hive表中。

语法：
```sql
select transform({column names1})
using '**.py'
as {column names2}
from {table name}
```
如果除python脚本外还有其它依赖资源，可以使用`ADD ARVHIVE`
# 5 limit 语句快速出结果 #
一般情况下，Limit语句还是需要执行整个查询语句，然后再返回部分结果。

有一个配置属性可以开启，避免这种情况---对数据源进行抽样

hive.limit.optimize.enable=true --- 开启对数据源进行采样的功能

hive.limit.row.max.size --- 设置最小的采样容量

hive.limit.optimize.limit.file --- 设置最大的采样样本数

缺点：有可能部分数据永远不会被处理到
# 6 本地模式 #
对于小数据集，为查询触发执行任务消耗的时间>实际执行job的时间，因此可以通过本地模式，在单台机器上（或某些时候在单个进程上）处理所有的任务。

```sql
set oldjobtracker=${hiveconf:mapred.job.tracker}; 
set mapred.job.tracker=local; 　
set marped.tmp.dir=/home/edward/tmp; sql 语句 　set mapred.job.tracker=${oldjobtracker};
```
-- 可以通过设置属性hive.exec.mode.local.auto的值为true，来让hve在适当的时候自动启动这个优化，也可以将这个配置写在$HOME/.hiverc文件中。

-- 当一个job满足如下条件才能真正使用本地模式：
1. job的输入数据大小必须小于参数：hive.exec.mode.local.auto.inputbytes.max(默认128MB)
2. job的map数必须小于参数：hive.exec.mode.local.auto.tasks.max(默认4)
3. job的reduce数必须为0或者1

可用参数hive.mapred.local.mem(默认0)控制child jvm使用的最大内存数。
# 7 并行执行 #
hive会将一个查询转化为一个或多个阶段，包括：MapReduce阶段、抽样阶段、合并阶段、limit阶段等。默认情况下，一次只执行一个阶段。 不过，如果某些阶段不是互相依赖，是可以并行执行的。

set hive.exec.parallel=true,可以开启并发执行。

set hive.exec.parallel.thread.number=16; //同一个sql允许最大并行度，默认为8。

会比较耗系统资源。
# 8 调整mapper和reducer的个数 #

1. Map阶段优化

map个数的主要的决定因素有： input的文件总个数，input的文件大小，集群设置的文件块大小（默认128M，不可自定义）。

举例：

a) 假设input目录下有1个文件a,大小为780M,那么hadoop会将该文件a分隔成7个块（6个128m的块和1个12m的块），从而产生7个map数

b) 假设input目录下有3个文件a,b,c,大小分别为10m，20m，130m，那么hadoop会分隔成4个块（10m,20m,128m,2m）,从而产生4个map数

即，如果文件大于块大小(128m),那么会拆分，如果小于块大小，则把该文件当成一个块。

map执行时间：map任务启动和初始化的时间+逻辑处理的时间。

    1.1. 减少map数

若有大量小文件（小于128M），会产生多个map，处理方法是：

```
set mapred.max.split.size=100000000; set mapred.min.split.size.per.node=100000000;
set mapred.min.split.size.per.rack=100000000;
``` 
-- 前面三个参数确定合并文件块的大小，大于文件块大小128m的，按照128m来分隔，小于128m,大于100m的，按照100m来分隔，把那些小于100m的（包括小文件和分隔大文件剩下的）进行合并

`set hive.input.format=org.apache.hadoop.hive.ql.io.CombineHiveInputFormat;`
-- 执行前进行小文件合并`

    2.2. 增加map数

当input的文件都很大，任务逻辑复杂，map执行非常慢的时候，可以考虑增加Map数，来使得每个map处理的数据量减少，从而提高任务的执行效率。

`set mapred.reduce.tasks=?`

2. Reduce阶段优化

调整方式：
```
-- set mapred.reduce.tasks=?

-- set hive.exec.reducers.bytes.per.reducer = ?
```
一般根据输入文件的总大小,用它的estimation函数来自动计算reduce的个数：reduce个数 = InputFileSize / bytes per reducer

# 9 严格模式 #

`set hive.marped.mode=strict`
-- 防止用户执行那些可能意想不到的不好的影响的查询

-- 分区表，必须选定分区范围

-- 对于使用order by的查询，要求必须使用limit语句。因为order by为了执行排序过程会将所有的结果数据分发到同一个reducer中进行处理。

-- 限制笛卡尔积查询：两张表join时必须有on语句

 

# 10 数据倾斜 #

表现：任务进度长时间维持在99%（或100%），查看任务监控页面，发现只有少量（1个或几个）reduce子任务未完成。因为其处理的数据量和其他reduce差异过大。

单一reduce的记录数与平均记录数差异过大，通常可能达到3倍甚至更多。 最长时长远大于平均时长。

原因

1)、key分布不均匀

2)、业务数据本身的特性

3)、建表时考虑不周

4)、某些SQL语句本身就有数据倾斜

关键词 | 情形 | 后果
---|---|---
join | 其中一个表较小，但是key集中 | 分发到某一个或几个Reduce上的数据远高于平均值
join | 大表与大表，但是分桶的判断字段0值或空值过多 | 这些空值都由一个reduce处理，灰常慢
group by | group by 维度过小，某值的数量过多 | 处理某值的reduce灰常耗时
count distinct | 某特殊值过 | 处理此特殊值reduce耗时
解决方案：

参数调节`hive.map.aggr=true`

参考文献：

> [1]. 《hive编程指南》Edward Capriolo