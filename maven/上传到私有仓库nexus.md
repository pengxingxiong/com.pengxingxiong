## 修改maven的.m2/seting.xml
```xml
<server>
  <id>nexus-release</id>
  <username>admin</username>
  <password>zte@2017</password>
</server>
```
配置好发布的id、账号
## 修改pom.xml
```xml
<distributionManagement>
    <repository>
        <id>nexus-release</id>
        <url>http://10.206.19.205:8081/nexus/content/repositories/releases/</url>
    </repository>
</distributionManagement>
```
注意两个位置的id要保持一致
## 准备打包命令
只使用普通的package命令即可，不用将依赖都打包
```sh
clean package
```
## 确认上传的坐标
在pom.xml中修改坐标值groupId、artifactId以及version，最主要的是version
```xml
<groupId>com.nubia.commons</groupId>
<artifactId>nubia-log-common</artifactId>
<version>1.0.4.RELEASE</version>
```
## 上传命令
idea编译器：deploy
## 重复上传
当再次上传相同版本的包时，需要到仓库中删除原有包
例如上面配置中的：http://10.206.19.205:8081/nexus
选中jar包，然后删除即可再次上传。

## 报错
上传最好不要有父模块依赖。
## 其他上传
```xml
<!--配置生成source包-->
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-source-plugin</artifactId>
    <executions>
        <execution>
            <id>attach-sources</id>
            <goals>
                <goal>jar</goal>
            </goals>
        </execution>
    </executions>
</plugin>
<!--配置生成Javadoc包-->
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-javadoc-plugin</artifactId>
    <version>3.0.0</version>
    <configuration>
        <encoding>UTF-8</encoding>
        <charset>UTF-8</charset>
        <encoding>UTF-8</encoding>
        <docencoding>UTF-8</docencoding>
    </configuration>
    <executions>
        <execution>
            <id>javadocs</id>
            <goals>
                <goal>jar</goal>
            </goals>
        </execution>
    </executions>
</plugin>
```