# MongoDB #

##  MongoDB的基本操作 ##

### 1.传统数据库与MongoDB数据库结构区别 ###

![](https://i.imgur.com/WxQvfJ2.png)

## pyspark MongoDB 使用经验 ##

### 1. 时区问题 ###

MongoDB的日期格式是 ISO-8601格式的，是一种军事时区后缀（比如UTC的‘Z’后缀），示例如下：ISODate("2018-08-23T10:05:09.000Z")，主要原因是官方MongoDB.driver存储时间按照UTC 0时区存储，值得注意的一个是MongoDB存储的时间会比北京时间相差8小时。

拿读取MongoDB来说：

```shell
#读：
MongoDB：ISODate("2018-08-23T10:05:09.000Z")  --->  pyspark客户端：2018-08-23 18:05:09
#写：
pyspark客户端now()：2018-10-31 11:15:59.337810 --->  ISODate("2018-10-31T03:15:59.337Z")
```

解决的方式一：

读：

本地传入的utc时间utcnow():本地北京时间转为utc 0区时间后传入过滤，但读出来的时间仍然比原时间多8小时。

### 2.时间过滤问题 ###

```
nowTime = datetime.datetime.utcnow()   
print(nowTime)  
isoTime = nowTime.strftime('%Y-%m-%dT%H:%M:%SZ')
match = {'$match': {'ts': {'$lte': {'$date': isoTime}}}}
df = spark.read \
	.format(MONGODB_DRIVER) \
	.options(uri=MONGODB_URI, database="ion", collection="testabc", pipeline=match).load()
```










