```java
package com.reyun.util;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.pool2.impl.GenericObjectPoolConfig;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import redis.clients.jedis.*;
import redis.clients.jedis.exceptions.JedisConnectionException;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * Created by guosy on 2017-9-18.
 */
public class RedisClusterUtil {

    private static final Logger logger = LoggerFactory.getLogger(RedisUtil.class);

    private static RedisClusterUtil instance;
    private JedisCluster jedisCluster;

    private RedisClusterUtil(JedisCluster jedisCluster) {
        this.jedisCluster = jedisCluster;
    }

    public static RedisClusterUtil getInstance() {
        if (null == instance) {
            syncInit();
        }
        return instance;
    }

    private static synchronized void syncInit() {
        if (instance == null) {
            GenericObjectPoolConfig config = new GenericObjectPoolConfig();
            config.setMaxTotal(100);
            config.setMaxIdle(64);
            config.setMinIdle(8);
            config.setMaxWaitMillis(100000);
            config.setTestOnBorrow(true);

            String writerTmp[] = StringUtils.split(Constant.redisCluster, ":");
            JedisCluster jedisCluster = new JedisCluster(new HostAndPort(writerTmp[0], Integer.parseInt(writerTmp[1])), 5000, 100, config);

            instance = new RedisClusterUtil(jedisCluster);
        }
    }

    /**
     * 获取key值
     *
     * @param key
     * @return
     */
    public String get(String key) {
        return jedisCluster.get(key);
    }

    /**
     * 订阅
     *
     * @param key
     * @return
     */
    public void subscribe(JedisPubSub pubSub, String key) {
        jedisCluster.subscribe(pubSub, key);
    }

    /**
     * 通过scan获取所有匹配的key
     *
     * @param pattern
     * @return
     */
    public Set<String> scan(String pattern) {
        Set<String> result = new HashSet<String>(1024);

        try {
            String cursor = ScanParams.SCAN_POINTER_START;

            ScanParams params = new ScanParams();
            params.match(pattern);
            params.count(100000);

            do {
                ScanResult<String> scanResult = jedisCluster.scan(cursor, params);
                cursor = scanResult.getStringCursor();
                result.addAll(scanResult.getResult());
            } while (!cursor.equals(ScanParams.SCAN_POINTER_START));

        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

    /**
     * 删除key
     *
     * @param key
     */
    public void del(String key) {
        try {
            jedisCluster.del(key);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
    }

    /**
     * key值加1
     *
     * @param key
     */
    public void incr(String key) {
        try {
            jedisCluster.incr(key);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
    }

    /**
     * key+field的值加value
     *
     * @param key
     * @param field
     * @param value
     */
    public void hincrBy(String key, String field, long value) {
        try {
            jedisCluster.hincrBy(key, field, value);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
    }

    /**
     * key值增加value
     *
     * @param key
     * @param value
     */
    public void incrBy(String key, long value) {
        try {
            jedisCluster.incrBy(key, value);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
    }

    /**
     * key值增加value
     *
     * @param key
     * @param value
     */
    public void incrByFloat(String key, float value) {
        try {
            jedisCluster.incrByFloat(key, value);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
    }

    public Map<String, String> hgetAll(String key) {
        Map<String, String> result = null;
        try {
            result = jedisCluster.hgetAll(key);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

    public String hget(String key, String field) {
        String result = null;
        try {
            result = jedisCluster.hget(key, field);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

    public Long pfadd(String key, String... value) {
        Long result;
        try {
            result = jedisCluster.pfadd(key, value);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

    public long pfcount(String key) {
        long result = 0;
        try {
            result = jedisCluster.pfcount(key);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

    public Long sadd(String key, String... members) {
        Long result = 0L;
        try {
            result = jedisCluster.sadd(key, members);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

    public Set<String> smembers(String key) {
        Set<String> result = null;
        try {
            result = jedisCluster.smembers(key);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            throw new JedisConnectionException(e);
        }
        return result;
    }

}

```