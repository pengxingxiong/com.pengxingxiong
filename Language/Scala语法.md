

####在scala中使用fastjson

报错示例：

```scala
val recordJson: JSONObject = JSONObject.parseObject(record)
```

正确做法

```scala
val recordJson: JSONObject = JSON.parseObject(record)
```

#### 使用Option

常用于Map中返回某个键的值

```scala
def remove(key: K): Option[V] = {
  import scala.collection.JavaConversions._
  var result: Option[V] = None
  for (bucket <- _buckets) {
    if (bucket.containsKey(key))  result = Some[V](bucket.remove(key))
  }
  result
}
```

