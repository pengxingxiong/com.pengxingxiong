## 解决包冲突
例如在com.web下有个log包，但是我想用新版本的log包，而不想更新com.web依赖，可以使用exclusions关键字
```xml
<dependency>
    <groupId>cn.nubia.boot.framework</groupId>
    <artifactId>nubia-boot-starter-web</artifactId>
    <version>1.1.2.RELEASE</version>
    <exclusions>
        <exclusion>
            <groupId>com.nubia.commons</groupId>
            <artifactId>nubia-log-common</artifactId>
        </exclusion>
    </exclusions>
</dependency>
<!--nubia日志工具-->
<dependency>
    <groupId>com.nubia.commons</groupId>
    <artifactId>nubia-log-common</artifactId>
    <version>1.0.4.RELEASE</version>
</dependency>
```
## 加入时间信息
```xml
<properties>
<maven.build.timestamp.format>yyyyMMddHHmmss</maven.build.timestamp.format>
</properties>
<!--在打包plugin中引用该属性-->
<finalName>
  ${project.artifactId}-${project.version}_${maven.build.timestamp}
</finalName>
```

