# 加入自定义索引方法

直接在对应的Repository接口中写入新的方法，如下：

```java
@Repository
public interface TerminalIdentifyRecordsRepository extends JpaRepository<TerminalIdentifyRecords, Long> {
    TerminalIdentifyRecords findOneByIp(String ip);
}
```

其中findOneBy是一种格式，可以让springboot自动找到该字段。

# 使用text类型字段的问题

数据库使用了text字段，jhipster模板中配置为TextBlob类型，liquibase中为cblob类型，会导致postgre报错：

```shell
Caused by: org.postgresql.util.PSQLException: 不良的类型值 long :
```

可以在对应的domain类的字段中加入@Lob和@Type注解：

```java
@ApiModelProperty(value = "该终端采集的会话四元组")
@Lob
@Type(type = "org.hibernate.type.TextType")
@Column(name = "jhi_keys")
private String keys;
```



