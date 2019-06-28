使用Maven的r ange dependency机制 ：

```xml
<dependency>
  <groupId>org.seleniumhq.selenium</groupId>
  <artifactId>selenium-java</artifactId>
  <version>[2.40.0,)</version>
</dependency>
```

　　以上注意version里的[2.40.0,)表示取2.40.0以上最新版本。当我这样写version之后，我的selenium框架里的jar包就会自动升级了，现在他们自己变成2.41.0版本了。  