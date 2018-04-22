# 对于有自增长主键的表进行插入
MySQL数据库表中有自增主键ID，当用SQL插入语句中插入语句带有ID列值记录的时候；

1. 如果指定了该列的值，则新插入的值不能和已有的值重复，而且必须大于其中最大的一个值；
2. 也可以不指定该列的值，只将其他列的值插入，让ID还是按照MySQL自增自己填；


具体：

1.创建数据库
```sql
create table if not exists userInfo (
id int PRIMARY KEY AUTO_INCREMENT,
name varchar(50) NOT NULL,
password varchar(50) NOT NULL
);
```
2.插入数据记录
```sql
insert into userInfo values(1,'aaa','1212');
```
## 当数据表中有自增长主键时，当用SQL插入语句中插入语句带有ID列值记录的时候；
(1)如果指定了该列的值，则新插入的值不能和已有的值重复，而且必须大于其中最大的一个值；

(2)也可以不指定该列的值，只将其他列的值插入，让ID还是按照MySQL自增自己填；

这种情况在进行插入的时候，两种解决方法： 
- 可以把id的值设置为null或者0，这样子mysql都会自己做处理 

```sql
insert into userInfo values(null,'ddf','8979');
insert into userInfo values(0,'ddf','8979');
```
# 对于插入值有单引号的处理
例如我要在一个字段中插入一条sql语句，但是sql语句中有大量单引号，如下：
```sql
INSERT overwrite TABLE com_nubia_stat_report.aftersale_rom_info partition(ds)
SELECT
xcontext['device_info_model'] AS phone_model,
xcontext['device_info_rom_version'] AS rom_version,
ds
FROM DEFAULT.profile
WHERE xcontext['device_info_model'] IS NOT NULL
AND xcontext['device_info_model']<>''
AND xcontext['device_info_model']<>'unKnow'
and xcontext['device_info_rom_version'] IS NOT NULL
AND xcontext['device_info_rom_version']<>''
AND xcontext['device_info_rom_version']<>'unKnow'
AND ds=date_sub(CURRENT_DATE,1);
```
这个时候就需要转义了，使用反斜杠\
```sql
INSERT INTO `engine_baseetl`
VALUES(
null,
'aftersale_rom_info',
'ROM版本管理',
'INSERT overwrite TABLE com_nubia_stat_report.aftersale_rom_info partition(ds)
SELECT
xcontext[\'device_info_model\'] AS phone_model,
xcontext[\'device_info_rom_version\'] AS rom_version,
ds
FROM DEFAULT.profile
WHERE xcontext[\'device_info_model\'] IS NOT NULL
AND xcontext[\'device_info_model\']<>\'\'
AND xcontext[\'device_info_model\']<>\'unKnow\'
and xcontext[\'device_info_rom_version\'] IS NOT NULL
AND xcontext[\'device_info_rom_version\']<>\'\'
AND xcontext[\'device_info_rom_version\']<>\'unKnow\'
AND ds=date_sub(CURRENT_DATE,1);',
'2017-11-01 19:00:01', 
'2017-11-01 19:00:01',
'0'
);
```
可以看到这个方法既解决了自增长主键的问题，也解决了单引号转义的问题。

> 需要注意的是 字段的起始单引号要和内容有连接，而不是内容放在单引号的下一行。

