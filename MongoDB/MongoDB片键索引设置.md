



# MongoDB使用



## MongoDB片键、索引检查

### 重要命令

> 重要命令：
>
> * 慢查询检查
>
>   ```
>   db.system.profile.find().sort({millis:-1});
>   ```
>
> * 查询指定数据库表的慢查询
>
>   ```
>   db.system.profile.find({ ns : 'ion.appUserInfo' }).sort({millis:-1}).pretty();
>   db.system.profile.find({ ns : 'ion.terminalInfo' }).sort({millis:-1}).pretty();
>   ```
>
> * explain进行语句执行分析
>
>   ```
>   db.getCollection('acInfo').explain("executionStats").find({"acMac":"1414.4b81.b49a"})
>   ```
>
> * 查看当前操作
>
>   ```
>   # 查看当前操作
>   use ion
>   db.currentOp(true)
>   
>   # 显示当前所有的任务状态
>   print("##########");db.currentOp().inprog.forEach(function(item){if(item.waitingForLock){var lock_info = item["opid"];print("waiting:",lock_info,item.op,item.ns);}});print("----");db.currentOp().inprog.forEach(function(item){if(!item.waitingForLock){var lock_info = item["opid"];print("doing",lock_info,item.op,item.ns);}});print("##########");
>   ```
>

### 片键设置

* 片键选择：

  * 总原则：{**粗 + 细**}，保证数据均匀分布
  * 好片键：保证数据均匀分布，同时具备良好的数据局部性特征。
    * 重点：**结合业务使用情况**
    * eg：业务特征：
      * 根据did(mac/ip)+ts字段从数据库取数据
      * 根据ts字段从数据库取数据
      * 根据idPattern+ts字段从数据库取数据

* 结合业务考虑示例

  * 示例1

  ```
  sh.shardCollection("ion.appUserInfo",{"name":1, "ts":1}) // name为应用名，片键值数量有限，为小基数片键，在数据量大时会出现大chunk。不太合理
  sh.shardCollection("ion.appUserInfo",{"name":1, "sip":1, "ts":1}) // name为粗粒度，sip为细粒度。比较合理
  ```

  * 示例2

  ```
  sh.shardCollection("ion.devHealthInfo",{"idPattern":1, "ts":1}) // 同上，小基数片键
  sh.shardCollection("ion.devHealthInfo",{"idPattern":1, "did":1, "ts":1})
  ```

  * 示例3：netRoute片键改进

    * 表内容

      ```
      { 
          "_id" : ObjectId("5c232d5ea716f8000ac34101"), 
          "ts" : ISODate("2018-12-26T15:26:01.710+0800"), 
          "did" : "ip:172.18.39.2", 
          "code" : "3", 
          "destIp" : "0.0.0.0/0", 
          "next" : "112.54.33.1", 
          "startIp" : NumberLong(0), 
          "endIp" : NumberLong(0)
      }
      ```

    * 当前片键

      ```
      sh.shardCollection("ion.netRoute",{"did":1, "ts":1})
      ```

    * 建议优化方式

      > 1、构建shardKey字段，以数据插入时段的小时构造一个主shardKey；
      >
      > 2、片键设置：sh.shardCollection("ion.netRoute",{"shardKey":1, "did":1})
      >
      > 3、表初始化脚本进行预分片，0~23划分为23个chunk，分别均匀分散到各个shard中

    * 延展

      > 封装构建shardKey的公共方法，传入当前时间戳和参数，能够返回小时、天、旬、月份粒度的值

### 索引设置

* 索引设置

  * 建多列索引的原则：{**精确匹配字段,排序字段,范围查询字段**} 
    * 尽量遵循最左前缀原则
  * 尽量综合评估查询场景,通过评估尽可能的将单列索引并入组合索引以降低索引数量
  * 创建组合索引的时候，应评估索引中包含的字段，尽量将数据基数大的字段放在组合索引的前面 
  * 索引的梳理越少越好。每当你建立一个索引时，系统会为你添加一个索引表，用于索引指定的列，然而当你对已建立索引的列进行插入或修改时，数据库则需要对原来的索引表进行重新排序，**重新排序的过程非常消耗性能**，但应对少量的索引压力并不是很大，但如果索引的数量较多的话对于性能的影响可想而知。所以在创建索引时需要谨慎建立索引，要把每个索引的功能都要发挥到极致，也就是说在可以满足索引需求的情况下，索引的数量越少越好 

* 索引合理性检查

  * 慢查询检查

    ```
    db.system.profile.find().sort({millis:-1});
    db.system.profile.find().sort({millis:-1}).pretty();
    ```

  * 查询指定数据库表的慢查询

    ```
    db.system.profile.find({ ns : 'ion.terminalInfo' }).sort({millis:-1}).pretty();
    db.system.profile.find({ ns : 'ion.appUserInfo' }).sort({millis:-1}).pretty();
    ```

  * 使用explain来分析语句执行情况

    ```
    db.getCollection('acInfo').explain("executionStats").find({"acMac":"1414.4b81.b49a"})
    ```

    