```java
package com.reyun.util;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.pool2.impl.GenericObjectPoolConfig;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.ScanParams;
import redis.clients.jedis.ScanResult;
import redis.clients.jedis.exceptions.JedisConnectionException;

/**
 * redis工具类
 *
 * @author ruijie liruijie@reyun.com
 * @date 2015年3月12日
 */
public class RedisUtil {
    private static final Logger logger = LoggerFactory.getLogger(RedisUtil.class);

    private static RedisUtil instance;

    private static JedisPool writerPool;
    private static JedisPool readerPool;

    private RedisUtil() {
    }

    public static RedisUtil getInstance() {
        if (null == instance) {
            syncInit();
        }
        return instance;
    }

    private static synchronized void syncInit() {
        if (instance == null) {

            GenericObjectPoolConfig config = new GenericObjectPoolConfig();
            config.setTestOnBorrow(true);

            String writerTmp[] = StringUtils.split(Constant.redisWriter, ":");
            writerPool = new JedisPool(config, writerTmp[0],
                    Integer.parseInt(writerTmp[1]), 5000);

            String readerTmp[] = StringUtils.split(Constant.redisReader, ":");
            readerPool = new JedisPool(config, readerTmp[0],
                    Integer.parseInt(readerTmp[1]), 5000);

            instance = new RedisUtil();
        }
    }

    /****************************** keys ******************************/
    /**
     * 通过scan获取所有匹配的key
     *
     * @param pattern
     * @return
     */
    @SuppressWarnings("deprecation")
    public Set<String> scan(String pattern) {
        logger.info("********** redis master scan keys: {} start **********", pattern);

        Set<String> result = new HashSet<String>(1024);

        Jedis jedis = null;
        try {
            jedis = readerPool.getResource();

            String cursor = ScanParams.SCAN_POINTER_START;

            ScanParams params = new ScanParams();
            params.match(pattern);
            params.count(100000);

            do {
                ScanResult<String> scanResult = jedis.scan(cursor, params);
                cursor = scanResult.getStringCursor();
                result.addAll(scanResult.getResult());
            } while (!cursor.equals(ScanParams.SCAN_POINTER_START));

            readerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                readerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }

        logger.info("********** redis master scan keys end, size: {} **********", result.size());
        return result;
    }

    /**
     * 删除key
     *
     * @param key
     */
    // update-Author:ruijie Date:2015-04-17 --------
    @SuppressWarnings("deprecation")
    public void del(String key) {
        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            jedis.del(key);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }
    }

    /****************************** string ******************************/
    /**
     * key值加1
     *
     * @param key
     */
    @SuppressWarnings("deprecation")
    public void incr(String key) {
        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            jedis.incr(key);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
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
    @SuppressWarnings("deprecation")
    public void hincrBy(String key, String field, long value) {
        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            jedis.hincrBy(key, field, value);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }
    }

    /**
     * key值增加value
     *
     * @param key
     * @param value
     */
    @SuppressWarnings("deprecation")
    public void incrBy(String key, long value) {
        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            jedis.incrBy(key, value);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }
    }

    /**
     * key值增加value
     *
     * @param key
     * @param value
     */
    @SuppressWarnings("deprecation")
    public void incrByFloat(String key, float value) {
        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            jedis.incrByFloat(key, value);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }
    }

    /**
     * 获取key值
     *
     * @param key
     * @return
     */
    @SuppressWarnings("deprecation")
    public String get(String key) {
        String result = null;

        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            result = jedis.get(key);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }

        return result;
    }

    @SuppressWarnings("deprecation")
    public Map<String, String> hgetAll(String key) {
        Map<String, String> result = null;

        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            result = jedis.hgetAll(key);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }

        return result;
    }

    @SuppressWarnings("deprecation")
    public String hget(String key, String field) {
        String result = null;

        Jedis jedis = null;
        try {
            jedis = writerPool.getResource();
            result = jedis.hget(key, field);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }

        return result;
    }
    /****************************** hyperloglog ******************************/
    /**
     * @param key
     * @param value
     * @author jiangzhx 使用redis2.8.14版本hyperloglog对象
     */
    @SuppressWarnings("deprecation")
    public Long pfadd(String key, String... value) {
        Jedis jedis = null;
        Long result;
        try {
            jedis = writerPool.getResource();
            result = jedis.pfadd(key, value);
            writerPool.returnResource(jedis);

        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }
        return result;
    }

    /**
     * 获取hyperloglog中value的个数
     *
     * @param key
     */
    // update-Author:ruijie Date:2015-08-26 --------
    @SuppressWarnings("deprecation")
    public long pfcount(String key) {
        long result = 0;

        Jedis jedis = null;
        try {
            jedis = readerPool.getResource();
            result = jedis.pfcount(key);
            readerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                readerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }

        return result;
    }

    /****************************** sets ******************************/
    /**
     * 添加members到set key中
     *
     * @param key
     * @param members
     */
    @SuppressWarnings("deprecation")
    public Long sadd(String key, String... members) {
        Jedis jedis = null;
        Long result = 0L;
        try {
            jedis = writerPool.getResource();
            result = jedis.sadd(key, members);
            writerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                writerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }
        return result;
    }

    /**
     * 获取集合key中所有的值
     *
     * @param key
     * @return
     */
    // update-Author:ruijie Date:2015-08-26 --------
    @SuppressWarnings("deprecation")
    public Set<String> smembers(String key) {
        Set<String> result = null;

        Jedis jedis = null;
        try {
            jedis = readerPool.getResource();
            result = jedis.smembers(key);
            readerPool.returnResource(jedis);
        } catch (JedisConnectionException e) {
            logger.error(e.getMessage(), e);
            if (null != jedis) {
                readerPool.returnBrokenResource(jedis);
            }
            throw new JedisConnectionException(e);
        }

        return result;
    }
}

```