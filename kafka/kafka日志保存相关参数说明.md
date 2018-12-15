kafka日志保存机制：
1、kafka topic日志按分区存储，每个分区对应一个目录
2、每个分区由一个或多个分段（segment）组成，一个分段对应磁盘上的一个数据文件和索引文件
3、当段文件数据达到阈值（默认1G）后会新建一个段。

kafka日志清理机制：
1、按照另个条件进行segment的清理工作，达到任一条件则进行清理：
   1）segment的存活时长。存活时长指的是segment的日志文件最近的一段时间没有被修改。
   2）分区的大小。分区的大小决定是否删除最旧的segment。
2、segment是否可删的检查周期是5分钟

kafka chart包对应参数：
相关产生提取到mscp-base/charts/kafka/values.yaml中：
   1）kafka_log_retention_hours: 24  
   2）kafka_log_retention_bytes: "-1"  
​      默认“-1”表示不根据日志文件大小清理日志
   3）kafka_log_segment_bytes: "1073741824" 
​      段文件大小，默认1G
​           
注意事项：
1、当kafka_log_retention_bytes非“-1”时，其值应大于或等于kafka_log_segment_bytes的值
2、若kafka_log_retention_bytes配得太小且消息的生产速度大于消费速度，在文件大小检查周期（5分钟）内消费者无法消费完生产者产生的消息，会出现数据丢失的问题。
3、鉴于第二点提出的风险，在不清楚数据量的情况下，基础版本将kafka_log_retention_bytes的值设为“-1”。
4、对于不同环境可根据实际数据量及磁盘容量设置kafka_log_retention_bytes。
5、若需修改kafka_log_retention_bytes，需开svn分支进行版本管理