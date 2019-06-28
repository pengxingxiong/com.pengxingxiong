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

##scala配置

如果是scala代码的话，比较麻烦些

```shell
clean validate scala:compile deploy -f pom.xml
```

如果在末尾追加`-U`命令，则表示不使用本地的缓存，从远程重新拉取。

其pom的配置为

```xml
<build>
  <plugins>
    <!--jdk编译版本-->
    <plugin>
      <groupId>org.apache.maven.plugins</groupId>
      <artifactId>maven-compiler-plugin</artifactId>
      <configuration>
        <source>8</source>
        <target>8</target>
        <encoding>UTF8</encoding>
      </configuration>
    </plugin>
    <!--上传到Nexus时生成source包-->
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
    <!--scala加入包-->
    <plugin>
      <groupId>net.alchim31.maven</groupId>
      <artifactId>scala-maven-plugin</artifactId>
      <executions>
        <execution>
          <id>scala-compile-first</id>
          <phase>process-resources</phase>
          <goals>
            <goal>add-source</goal>
            <goal>compile</goal>
          </goals>
        </execution>
        <execution>
          <id>scala-test-compile</id>
          <phase>process-test-resources</phase>
          <goals>
            <goal>testCompile</goal>
          </goals>
        </execution>
      </executions>
      <configuration>
        <scalaVersion>${scala.version}</scalaVersion>
      </configuration>
    </plugin>
  </plugins>
</build>
<distributionManagement>
  <repository>
    <id>RGONC-Host-Releases</id>
    <name>RGONC Release Repository</name>
    <url>http://nexus.mig.ruijie.net:8081/nexus/content/repositories/RGONC-Host-Releases/</url>
  </repository>
</distributionManagement>
```



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