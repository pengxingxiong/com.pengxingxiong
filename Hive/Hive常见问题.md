## hive 插入数据
注意的是map结构部分的插入
```sql
insert into table nubiastats.profile_stats partition (bucketid, ds)
select '5906c31263134cb589187611ebf0afa1','neotheme_rom','0341de8f776cefdad36eb5f124bc3a1bd8f90d82f37b47b24ae5a09595c9c6dd',true,'unknown','unknown',NULL,5,NULL,5,NULL,NULL,'unknown','unknown','unknown','unknown','unknown','unknown','unknown','unknown','unknown','unknown','nubiaNX607J','unknown',NULL,NULL,'4389969',NULL,NULL,NULL,'unknown','unknown','unknown','nubiaNX607J','unknown','unknown','unknown','unknown','unknown','unknown',map('device_info_rom_version','rom_version_100','device_info_model','model_567'),'unknown',NULL,NULL,NULL,'nubia','NX607J','unknown','nubia','NX607J','unknown','unknown','unknown','597ddca8372d00b031476fff38a5f590','2017-11-01' from default.profile limit 1;
```
实际上还有个伪表的插入方式
```sql
--创建 DUAL 表，用于单条记录插入。DUAL 表示一种单行单列的伪表，用于数据处理或传递
create table dual (x string);
--必须有个初始值的插入
insert overwrite table dual select 'x' from users limit 1;
--插入用户信息表：（其中concat为连接函数）
select '1','2' from dual;
```
## hive导入和导出
导出
```sql
insert overwrite local directory '/data/tmp/qisf/baohao/' row format delimited fields terminated by '\t' collection items terminated by '\002' map keys terminated by '\001' SELECT * FROM biz_data where ds='2017-11-01' and appid='6a0d8723a877422591b662f025a791b0';
```
导入
```sql
load data local inpath '/data/baohao/*' into table biz_data partition(appid='511ae8cbbf3f450d9ea13d7275e66bac',ds='2017-11-01');
```
## 字符串替换（正则）
```sh
hive> SELECT REGEXP_REPLACE('Ellen Hildi Smith','(.*) (.*) (.*)', '$3, $1 $2');
OK
Smith, Ellen Hildi

```
## 字符串分割
```sql
INSERT INTO TABLE cn_nubia_cloud.p_aliyun_size_distribute
SELECT
region,
people_num,
ds
FROM
cn_nubia_cloud.aliyun_base
lateral view explode(str_to_map(size_distribute,'\\|',':')) tf as region, people_num
WHERE ds=date_sub(current_date,1)
```
把一个字符串`size_distribute`先按照|分割为多个字符串，再对每个子字符串按照：分割为键和值，这里键是`region`，值是`people_num`。
## 从多个字段中选择一个字段查询
```sql
SELECT
(CASE
WHEN ${{field}}=d_file_size THEN to_char(round(cast(A.t_file_size*1.0/power(1024,3) as double),2),'#.##')
WHEN ${{field}}=pv THEN to_char(round(cast(A.pv*1.0/10000 as double),2),'#.##')
WHEN ${{field}}=api_sum THEN to_char(round(cast(A.api_sum*1.0/10000 as double),2),'#.##')
WHEN ${{field}}=t_file_count THEN to_char(round(cast(A.t_file_count*1.0/10000 as double),2),'#.##')
WHEN ${{field}}=t_photo_count THEN to_char(round(cast(A.t_photo_count*1.0/10000 as double),2),'#.##')
WHEN ${{field}}=t_video_count THEN to_char(round(cast(A.t_video_count*1.0/10000 as double),2),'#.##')
WHEN ${{field}}=t_file_size THEN to_char(round(cast(A.t_file_size*1.0/power(1024,3) as double),2),'#.##')
ELSE A.DS END ) AS field,
A.DS
FROM
CN_NUBIA_CLOUD.USER_INVOLVE AS A
WHERE ds>='${{start_ds}}'
AND ds<='${{end_ds}}'
```
用大量的case when的方式，然而case when语法要保证前后的数据类型一致，因此使用to_char将后面计算的数据进行转化；同时为了防止round函数计算出来的浮点数出现科学计数法格式，可以使用`#.##`将格式控制为小数后两位形式。

## 字符串转为map格式
使用两个分隔符将文本拆分为键值对。 Delimiter1将文本分成K-V对，Delimiter2分割每个K-V对。对于delimiter1默认分隔符是'，'，对于delimiter2默认分隔符是'='。
- 案例1：
```sh
hive> 
    > select str_to_map('aaa:11&bbb:22', '&', ':')
    > from tmp.tmp_jzl_20140725_test11;
OK
{"bbb":"22","aaa":"11"}
```
- 案例2:
```sh
hive> select str_to_map('aaa:11&bbb:22', '&', ':')['aaa']
    > from tmp.tmp_jzl_20140725_test11;
OK
11
```
## 行转列 lateral view
lateral view用于和split、explode等UDTF一起使用的，能将一行数据拆分成多行数据，在此基础上可以对拆分的数据进行聚合，lateral view首先为原始表的每行调用UDTF，UDTF会把一行拆分成一行或者多行，lateral view在把结果组合，产生一个支持别名表的虚拟表。

单个LATERAL VIEW:

源表（table1）数据{A:string B:array<BIGINT> C:string}
```sh
A                         B                                C

190     [1030,1031,1032,1033,1190]      select id
191     [1030,1031,1032,1033,1190]      select id
```
希望的结果是：
```sh
190    1030  select id

190    1031  select id

190    1032  select id

190    1033  select id

190    1190  select id

191    1030  select id

191    1031  select id

191    1032  select id

191    1033  select id

191    1190  select id
```
故使用`select A,B,C from table_1 LATERAL VIEW explode(B) table1 as B`得到上述结果

多个LATERAL VIEW的介绍：

LATERAL VIEW clauses are applied in the order that they appear. For example with the following base table:


Array<int> col1 | Array<string> col2
---|---
[1, 2] | [a", "b", "c"]
[3, 4] | [d", "e", "f"]

```sql
SELECT myCol1, myCol2 FROM baseTable
LATERAL VIEW explode(col1) myTable1 AS myCol1
LATERAL VIEW explode(col2) myTable2 AS myCol2;
```
结果如下：
```sh

int myCol1 | string myCol2
---|---
1 | "a"
1 | "b"
...
```
复杂方式：
```sh
select * from tb_split;

20141018  aa|bb 7|9|0|3

20141019  cc|dd  6|1|8|5
```
使用方式：
```sh
select datenu,des,type from tb_split 

lateral view explode(split(des,"//|")) tb1 as des

lateral view explode(split(type,"//|")) tb2 as type
```

>实际使用中并不建议使用|这样要转义的分隔符，因为对于有些解析器来说，转义双斜杠`\\`可能会被漏掉一个，这样导致问题排查复杂。建议使用`=等于:冒号,逗号`这样的分隔符，并不需要转义。