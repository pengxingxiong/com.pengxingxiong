现象：
重建index比较漫长，每个partition都需要重建，406多个paritition重建了有240多秒

```shell
2018-12-11 00:55:08,576 WARN kafka.log.Log: Found an corrupted index file, /data2/kafka/data1/consumer_offsets-12/00000000000000000000.index, deleting and rebuilding index... 2018-12-11 00:55:08,577 WARN kafka.log.Log: Found an corrupted index file, /data4/kafka/data1/consumer_offsets-37/00000000000000000000.index, deleting and rebuilding index... 2018-12-11 00:55:08,577 WARN kafka.log.Log: Found an corrupted index file, /data9/kafka/data1/consumer_offsets-18/00000000000000000000.index, deleting and rebuilding index... 2018-12-11 00:55:08,577 WARN kafka.log.Log: Found an corrupted index file, /data7/kafka/data1/consumer_offsets-31/00000000000000000000.index, deleting and rebuilding index... 2018-12-11 00:55:08,576 WARN kafka.log.Log: Found an corrupted index file, /data6/kafka/data1/__consumer_offsets-0/00000000000000000000.index, deleting and rebuilding index...
```

回头需整理一下kafka重启的过程，感觉rebuild index是串行的，在该broker上遇到leader parition比较多的情况下，重建过程比较漫长，在此期间，该broker不可用

```shell
val numRecoveryThreadsPerDataDir = props.getIntInRange(“num.recovery.threads.per.data.dir”, 1, (1, Int.MaxValue))
```

这个参数可以设置多线程recovery