为了对无效的redis进行清理，我们就需要先查出无效的key，一般采用keys命令，但是这种方式但由于KEYS命令一次性返回所有匹配的key，所以，当redis中的key非常多时，对于内存的消耗和redis服务器都是一个隐患。特别是对线上的机子带来很大的影响。我们这里推荐使用scan命令

SCAN 每次执行都只会返回少量元素，所以可以用于生产环境，而不会出现像 [KEYS](http://www.redis.cn/commands/keys.html) 或者 [SMEMBERS](http://www.redis.cn/commands/smembers.html) 命令带来的可能会阻塞服务器的问题。

scan相关的redis命令有：

-   SCAN 命令用于迭代当前数据库中的数据库键。
-   SSCAN 命令用于迭代集合键中的元素。
-   HSCAN 命令用于迭代哈希键中的键值对。
-   ZSCAN 命令用于迭代有序集合中的元素（包括元素成员和元素分值）。

为了实现对redis缓存的无效数据进行清理。我们采用shell脚本来实现：

```shell
#/bin/sh

function checkdata() {

    local tmp_array=($(echo $1 |tr "\n" " " |tr "\r" " "))

    tag_index=${tmp_array[0]}

    echo "游标：$tag_index"

    echo "数组大小：${#tmp_array[@]}"

    local i=1

    echo "+++++++++++++++++++++++++++++++"

    while [ $i -lt ${#tmp_array[@]} ]

      do

        echo ${tmp_array[$i] }

        deleteKeys  ${tmp_array[$i] }

        let i++

      done

    sleep 5

    return $res

}

 

function deleteKeys() {

    local tmp_key=$1

    local key_type=`${redis_path} -c  -h ${redis_ip} -p ${redis_port} TYPE $tmp_key`

    echo $key_type

    if [ "$key_type" == "none" ] ;then

        ${redis_path} -c  -h ${redis_ip} -p ${redis_port} DEL  "$key_type"

        echo "delete key:$tmp_key"

    fi

}

 

OLD_IFS="$IFS"

IFS=" "

redis_path=/data/program/redis-3.0.0-rc4/bin/redis-cli

redis_ip=10.25.xx

redis_port=7000

tag_index=1

first_operator=1

 

while [ $tag_index -ne 0 ]

  if [ $first_operator -eq 1 ];then

    first_operator=0

    tag_index=0

  fi

  do

    data=`${redis_path} -c  -h ${redis_ip} -p ${redis_port} --raw sscan NAME_TopicTypeForm  $tag_index`

    echo "${redis_path} -c  -h ${redis_ip} -p ${redis_port} --raw sscan NAME_TopicTypeForm  $tag_index"

    checkdata $data

  done

IFS=$OLD_IFS

```

