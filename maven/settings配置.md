这里给出一种能够快速找到镜像的方案，目的是为了快速从maven仓库中拉取jar。同时也给出了锐捷私有仓库的配置，之前的com.microsoft.z3.jar需要本地配置，这个时候只要稍微改改pom文件即可。

### 步骤1

修改settings.xml文件

```xml
<?xml version="1.0" encoding="UTF-8"?>
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
          xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 http://maven.apache.org/xsd/settings-1.0.0.xsd">
    <localRepository>C:\Users\peng\.m2\repository</localRepository>
    <mirrors>
        <mirror>
            <id>alimaven</id>
            <name>aliyun maven</name>
            <url>http://maven.aliyun.com/nexus/content/groups/public/</url>
            <mirrorOf>central</mirrorOf>
        </mirror>
        <mirror>
            <id>central</id>
            <name>Maven Repository Switchboard</name>
            <url>http://repo1.maven.org/maven2/</url>
            <mirrorOf>central</mirrorOf>
        </mirror>
        <mirror>
            <id>repo2</id>
            <mirrorOf>central</mirrorOf>
            <name>Human Readable Name for this Mirror.</name>
            <url>http://repo2.maven.org/maven2/</url>
        </mirror>
        <mirror>
            <id>ibiblio</id>
            <mirrorOf>central</mirrorOf>
            <name>Human Readable Name for this Mirror.</name>
            <url>http://mirrors.ibiblio.org/pub/mirrors/maven2/</url>
        </mirror>
        <mirror>
            <id>jboss-public-repository-group</id>
            <mirrorOf>central</mirrorOf>
            <name>JBoss Public Repository Group</name>
            <url>http://repository.jboss.org/nexus/content/groups/public</url>
        </mirror>
        <!-- 中央仓库在中国的镜像 -->
        <mirror>
            <id>maven.net.cn</id>
            <name>oneof the central mirrors in china</name>
            <url>http://maven.net.cn/content/groups/public/</url>
            <mirrorOf>central</mirrorOf>
        </mirror>
        <!-- 锐捷私有仓库 -->
        <mirror>
            <id>central</id>
            <name>RGONC Proxy Central</name>
            <url>http://nexus.mig.ruijie.net:8081/nexus/content/repositories/RGONC-Proxy-Central/</url>
            <mirrorOf>central</mirrorOf>
        </mirror>
    </mirrors>

    <profiles>
        <profile>
            <id>rgonc-repo</id>
            <repositories>
                <repository>
                    <id>RGONC-Group</id>
                    <name>RGONC Group</name>
                    <url>http://nexus.mig.ruijie.net:8081/nexus/content/groups/RGONC-Group/</url>
                </repository>
            </repositories>

            <pluginRepositories>
                <pluginRepository>
                    <id>RGONC-Group</id>
                    <name>RGONC Group</name>
                    <url>http://nexus.mig.ruijie.net:8081/nexus/content/groups/RGONC-Group/</url>
                </pluginRepository>
            </pluginRepositories>
        </profile>
    </profiles>

    <activeProfiles>
        <activeProfile>rgonc-repo</activeProfile>
    </activeProfiles>


</settings>
```

### 步骤2 

修改pom.xml文件

现在已经将com.microsoft.z3.jar放入到锐捷私有的maven仓库中了。

```xml
<dependency>
  <groupId>cn.com.ruijie.ibns.common</groupId>
  <artifactId>com.microsoft.z3</artifactId>
  <version>1.0.0</version>
</dependency>
```

