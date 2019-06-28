```python
#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
@author:pengxingxiong
@time: 2019/1/14 9:43
@desc:
"""
import time
import pandas
import pymongo


class CollectionsStat:
    """
    统计核心算法来源于command函数，该函数能够使用的mongo命令可以参考以下网址：\n
    https://docs.mongodb.com/manual/reference/command/
    """
    MONGODB_URI = "mongodb://172.18.135.2:26000,172.18.135.3:26000,172.18.135.4:26000"
    DATABASE__STAT_INDEX_ALL = ["raw", "objects", "avgObjSize", "dataSize", "storageSize", "numExtents", "indexes",
                                "indexSize", "fileSize", "extentFreeList"]
    DATABASE__STAT_INDEX = ["objects", "avgObjSize", "dataSize", "storageSize", "numExtents", "indexes", "indexSize",
                            "fileSize"]
    COLLECTION__STAT_INDEX_ALL = ["ns", "sharded", "capped", "count", "size", "storageSize", "totalIndexSize",
                                  "indexSizes",
                                  "avgObjSize", "nindexes", "nchunks", "shards"]
    COLLECTION__STAT_INDEX = ["ns", "sharded", "capped", "count", "size", "storageSize", "totalIndexSize", "avgObjSize",
                              "nindexes", "nchunks"]

    def __init__(self, db_name):
        self.client = pymongo.MongoClient(self.MONGODB_URI)
        self.database = self.client.get_database(db_name)
        print("连接数据库成功，并开始统计")

    def get_db_stat(self):
        # 输出数据库统计
        db_cursor = self.database.command("dbstats")  # type:dict
        db_data = {}
        for ele in self.DATABASE__STAT_INDEX:
            db_data[ele] = db_cursor[ele]
        print(db_data)

    def get_coll_stat(self):
        # 集合统计
        coll_cursor_list = self.database.command("listCollections")["cursor"]["firstBatch"]
        coll_data = {}
        for ele in self.COLLECTION__STAT_INDEX:
            coll_data[ele] = []
        for coll_ele in coll_cursor_list:
            collections_name = coll_ele["name"]
            coll_stat = self.database.command("collstats", collections_name)  # type:dict
            for ele in self.COLLECTION__STAT_INDEX:
                if ele in coll_stat.keys():
                    coll_data[ele].append(coll_stat[ele])
                else:
                    coll_data[ele].append(0)
        # 将集合统计结果转为DataFrame
        coll_df = pandas.DataFrame(coll_data)
        # 获取当前时间
        current_time = time.strftime("%Y%m%d%H%M%S", time.localtime())
        result_path = "e:/data/mongo/coll_stat/coll_stat_%s.csv" % current_time
        # 输出到文件
        coll_df.to_csv(result_path, index=False)

    def __del__(self):
        print("统计成功，并断开连接")
        self.client.close()


if __name__ == "__main__":
    collection_stat = CollectionsStat("ion")
    collection_stat.get_db_stat()
    collection_stat.get_coll_stat()

```

