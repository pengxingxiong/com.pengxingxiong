# CacheBuilder在使用RemovalListeners失效的原因及解决方案

加入maven依赖

```xml
<dependency>
  <groupId>com.google.guava</groupId>
  <artifactId>guava</artifactId>
  <version>27.1-jre</version>
</dependency>
```

`CacheBuilder`在使用过程中最麻烦的是，对于已经过期的缓存数据，不会显式地立即删除，只会让你在查询的时候查不到。但是实际应用中需要有一种方案能够在删除的时候触发类似持久化的操作，通常使用`RemovalListeners`。但是实验发现这个监听器不会自动生效，解决思路是对这个`RemovalListeners`使用异步调用，即`RemovalListeners.asynchronous`。并且重新开一个定时线程，如`Executors.newSingleThreadExecutor()`，每一段时间就执行`cache.cleanUp()`函数，这个函数能够触发`RemovalListeners`里面的持久化操作，对所有过期的key生效。下面是`cleanUp()`函数的触发机制验证。

代码举例：

```java
import com.google.common.cache.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import java.util.ArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

/**
 * Created by 。。。 on 2019/3/18 19:43
 */
public class CacheMap<K, V> {
    private static final Logger log = LoggerFactory.getLogger(CacheMap.class);
    private Cache<K, V> cache;
    private ExpiredCallback<K, V> _callback;

    public CacheMap(long maximumSize, long expireAfterWriteSeconds, ExpiredCallback<K, V> callback) {
        _callback = callback;
        RemovalListener<K, V> removalListeners = RemovalListeners.asynchronous(notification -> {
            _callback.expire(notification.getKey(),notification.getValue());
        },Executors.newSingleThreadExecutor());
        cache = CacheBuilder.newBuilder().maximumSize(maximumSize)
                .expireAfterWrite(expireAfterWriteSeconds, TimeUnit.SECONDS)
                .initialCapacity(10)
                .removalListener(removalListeners).build();
    }

    public CacheMap(long maximumSize, long expireAfterWriteSeconds) {
        cache = CacheBuilder.newBuilder().maximumSize(maximumSize)
                .expireAfterWrite(expireAfterWriteSeconds, TimeUnit.SECONDS)
                .initialCapacity(10)
                .removalListener((RemovalListener<K, V>) rn -> log.info("被移除缓存{}:{}", rn.getKey(), rn.getValue())).build();
    }

    public interface ExpiredCallback<K, V> {
        void expire(K key, V val);
    }

    private void rotate(K key, V val) {
        if (_callback != null) {
            _callback.expire(key, val);
        }
    }

    public V get(K key) {
        return key != null ? cache.getIfPresent(key) : null;
    }

    public void put(K key, V value) {
        if (key != null && value != null) {
            cache.put(key, value);
        }
    }

    public static void main(String[] args) throws InterruptedException {
        ArrayList<String> value1 = new ArrayList<>();
        value1.add("val1");

        ExpiredCallback<String, ArrayList<String>> expiredCallback = (key, val) -> System.out.println(key + ":" + val);
        CacheMap<String, ArrayList<String>> cacheMap = new CacheMap<>(10000, 5,expiredCallback);
        cacheMap.put("key1", value1);
        System.out.println("是否删除 key1");
        cacheMap.cache.cleanUp();
        Thread.sleep(4000);
        cacheMap.get("key1").add("val2");
        //加入一个key2
        ArrayList<String> value2 = new ArrayList<>();
        value2.add("val21");
        cacheMap.put("key2", value2);
        Thread.sleep(2000);//key2不会过期，key1过期，并且没有触发监听器内容
        System.out.println("是否删除 key1");
        cacheMap.cache.cleanUp();//触发监听器内容
        Thread.sleep(4000);
        System.out.println("是否删除 key2");
        cacheMap.cache.cleanUp();
    }
}
```

