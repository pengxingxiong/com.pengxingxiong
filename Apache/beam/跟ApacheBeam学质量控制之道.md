在学习、开发Apache Beam源码过程中，除了它精妙的设计(通过几个简单的概念抽象把实时和离线的计算逻辑模型统一了起来)，庞大的代码量(Java 33万行, Python9万行)，还有一个比较大的感受是它的质量控制做得特别好，比之前参与过的其它一些开源项目都要好，这可能跟Google的工程质量高于业界有关。但是这里面也没什么什么奇技淫巧，只是善用了一些插件而已，我在这里想把我在Apache Beam里面看到、学到的一些质量实践分享给大家。代码质量提升其实没有什么太多的捷径，这里要分享的也不是多么高大上的道理，每个小点都是很零碎的一个小技巧，但是所有这些`小技巧`组合起来，会让你对代码的质量更有信心。

## 我们代码里面的一些常见问题

事物的好坏是对比出来的，我们先来看看我们代码里面一些问题。

### 随意的JavaDoc

比如:

```java
/**
 * UserService
 *
 * @author <somebody>
 * @version $Id: UserService.java, v 0.1 2014年8月28日 上午11:04:58 <somebody> Exp $
 */
public interface UserService extends ExtUserService {
```

再比如:

```java
/**
 * DataProjectService
 *
 * @author <somebody>
 * @since created on 2015-10-28 22:27
 */
public interface DataProjectService {
```

我们代码很多没有javadoc, 或者好一点的有javadoc，但是不符合javadoc规范(用javadoc实际去生成会报格式不对)。

### 超长的代码

下面截取一段CAP里面的代码:

```java
    /**
     * // 看这里
     * @see com.alipay.cap.core.service.DataProjectAccessKeyService#updateExcuteAKById(java.lang.Long, java.lang.String, java.lang.String)
     */
    @Override
    public void updateExcuteAKByUnionKey(String namespaceId, String dataProjectKey, String accessId, String accessKey, String operator) {
        CapNamespaceProjectAkCriteria example = new CapNamespaceProjectAkCriteria();
        example.createCriteria()
            .andDataProjectKeyEqualTo(dataProjectKey)
            .andNamespaceIdEqualTo(namespaceId)
            .andIsDeletedEqualTo(false);
        List<CapNamespaceProjectAkDO> capNamespaceProjectAkDOs = capNamespaceProjectAkDAO.selectByExample(example);
        // .....
    }
```

比如这里这行代码长达138，与一般的推荐配置(80, 最多100)长多了，已经超出一屏了，看起来很难受。

### 重复的、未使用的maven依赖

用依赖分析工具分析一下CAP的代码依赖，可以发现，里面有很多没用的依赖以及用到了但是没有声明的依赖。

```shell
[INFO] --- maven-dependency-plugin:2.8:analyze (default-cli) @ cap-web-openapi ---
[WARNING] Used undeclared dependencies found:
[WARNING]    com.alipay.cap:cap-common-client:jar:1.0.20140703:compile
[WARNING]    com.alipay.cap:cap-common-conf-core:jar:1.0.20170512:compile
[WARNING]    com.alipay.cap:cap-common-dal:jar:1.1:compile
[WARNING]    com.google.code.findbugs:annotations:jar:2.0.3:compile
[WARNING]    com.alibaba.alimonitor:alimonitor-jmonitor:jar:1.0.4:compile
[WARNING]    org.slf4j:slf4j-api:jar:1.7.14:compile
[WARNING]    com.alipay.dpc:smartmw-cache:jar:2.0.20161227:compile
[WARNING]    org.springframework:org.springframework.context:jar:3.0.5.RELEASE:compile
[WARNING]    org.springframework:spring-modules-validation:jar:0.9:compile
[WARNING]    org.springframework:org.springframework.core:jar:3.0.5.RELEASE:compile
[WARNING]    com.alibaba:fastjson:jar:1.2.8.sec01:compile
[WARNING]    com.alibaba.toolkit.common:toolkit-common-lang:jar:1.1.1:compile
[WARNING]    com.alipay.sofa:sofa-runtime-api:jar:4.5.2:compile
[WARNING] Unused declared dependencies found:
[WARNING]    com.alipay.sofa.service:sofa-service-api:jar:3.2.4.1:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-sofa-env-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-alipay-toolbox-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-toolbox-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-validation-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-tair-session-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-widget-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-uisvr-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-json-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-resource-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-alipay-security-plugin:jar:4.1.6:compile
[WARNING]    com.alipay.sofa.web.mvc:mvc-alipay-auth-plugin:jar:4.1.6:compile
[WARNING]    commons-beanutils:commons-beanutils:jar:1.7.0:compile
[WARNING]    com.alipay.cap:cap-biz-service-impl:jar:1.0:compile
[WARNING]    org.springframework:spring-webmvc:jar:3.2.2:compile
```

这些每个问题单个看起来都不是大问题，但是堆积起来会让代码看起来有点`脏`。

## Beam里面的一些小实践

下面我介绍一下我在Beam里面看到的质量控制相关的一些小技巧，主要是一些maven插件的使用和一些测试库的使用，以及一些思想上的意识。

### Dependency插件

Dependency插件的作用是帮我们扫描项目的依赖问题，它可以把我们实际要到了，但是没有声明的依赖；或者实际没用到，但是声明了的依赖都给找出来，帮你保持代码的纯洁, 下面是一个扫描报错的例子:

```shell
[INFO] --- maven-dependency-plugin:3.0.1:analyze-only (default) @ beam-dsls-sql ---
[WARNING] Unused declared dependencies found:
[WARNING]    com.google.auto.value:auto-value:jar:1.4.1:compile
[INFO] ------------------------------------------------------------------------
[INFO] BUILD FAILURE
[INFO] ------------------------------------------------------------------------
[INFO] Total time: 27.409 s
[INFO] Finished at: 2017-06-16T11:33:03+08:00
[INFO] Final Memory: 50M/401M
[INFO] ------------------------------------------------------------------------
[ERROR] Failed to execute goal org.apache.maven.plugins:maven-dependency-plugin:3.0.1:analyze-only (default) on project beam-dsls-sql: Dependency problems found -> [Help 1]
[ERROR]
```

这里我们声明了一个没有用到的，但是声明了的`auto-value`的依赖，因此编译失败了。有了这个插件的保证，我们可以很有自信的知道我们引入的每个依赖都是有用的，有意义的。

## Checkstyle

> Checkstyle is a development tool to help programmers write Java code that adheres to a coding standard. It automates the process of checking Java code to spare humans of this boring (but important) task. This makes it ideal for projects that want to enforce a coding standard.

它能做的一些典型检查包括:

- FallThrough: 如果你的switch/case里面没有写break，它会自动检测出来。
- CustomImportOrder: 检查import的顺序符合指定样式。
- LineLength: 检查代码行数不要超长。
- MethodLength: 检查方法行数不要超长。

下面是我最近在写Beam代码的时候编译时候报的几个Checkstyle错误:

```shell
[bash] There are 5 errors reported by Checkstyle 6.19 with beam/checkstyle.xml ruleset.
[ERROR] src/main/java/org/apache/beam/dsls/sql/meta/Column.java:[19](javadoc) JavadocType: Missing a Javadoc comment.
[ERROR] src/main/java/org/apache/beam/dsls/sql/meta/DefaultMetaStore.java:[9](javadoc) JavadocParagraph: Empty line should be followed by <p> tag on the next line.
[ERROR] src/main/java/org/apache/beam/dsls/sql/meta/Table.java:[21](javadoc) JavadocType: Missing a Javadoc comment.
[ERROR] src/main/java/org/apache/beam/dsls/sql/meta/Table.java:[35](javadoc) JavadocParagraph: Empty line should be followed by <p> tag on the next line.
[ERROR] src/main/java/org/apache/beam/dsls/sql/rel/BeamValuesRel.java:[60](sizes) LineLength: Line is longer than 100 characters (found 115).
[INFO] ------------------------------------------------------------------------
[INFO] BUILD FAILURE
[INFO] ------------------------------------------------------------------------
[INFO] Total time: 27.517 s
[INFO] Finished at: 2017-06-19T14:49:28+08:00
[INFO] Final Memory: 49M/478M
[INFO] ------------------------------------------------------------------------
[ERROR] Failed to execute goal org.apache.maven.plugins:maven-checkstyle-plugin:2.17:check (default) on project beam-dsls-sql: You have 5 Checkstyle violations. -> [Help 1]
```

# Enforcer插件

Enforcer是另一比有意思的maven插件, 它可以帮你指定你代码所需的运行环境，比如需要的maven版本(maven不同版本之间有时候行为差异还是很大的，比如我们采云间的代码基本都是用3.x版本编译的，但是开发机上默认装的是2.x的，这样编译的时候就会报很奇怪的错误，而enforcer插件则可以把这个需求明确化，如果你的maven的版本不对，它会明确告诉你maven版本不对，而不是其它的诡异错误)。它能做到的一些检查包括:

- Maven的版本
- JDK的版本
- OS的版本
- 检查指定的属性(property)是否存在

> 在我看来maven-enforcer-plugin有点像java语言里面assert，assert如果失败了，说明运行环境有问题，直接失败。

下面是一个Maven版本不对的错误:

```shell
[INFO] --- maven-enforcer-plugin:1.4.1:enforce (enforce) @ beam-dsls-sql ---
[WARNING] Rule 2: org.apache.maven.plugins.enforcer.RequireMavenVersion failed with message:
Detected Maven Version: 3.1.1 is not in the allowed range [3.2,).
```

## Google AutoValue

最近这些年函数式编程的思维开始流行起来，很多非函数式的语言也开始在语言层面支持某些函数式的特征，比如Java 8里面的`lambda`, `stream`等等，Google AutoValue也是类似的目的，它的目的是让你的POJO编程readonly的(只有Getter), 从而实现函数式语言里面Immutable的特性。

它最大的贡献在于，它让你只需要定义你需要的字段:

```java
import com.google.auto.value.AutoValue;

@AutoValue
abstract class Animal {
  static Animal create(String name, int numberOfLegs) {
    // See "How do I...?" below for nested classes.
    return new AutoValue_Animal(name, numberOfLegs);
  }

  abstract String name();
  abstract int numberOfLegs();
}
```

这里我们定义了我们需要两个字段: `name`和`numberOfLegs`以及一个用来构建`Animal`对象的`create`方法，其它的则都由AutoValue自动生成，其中`AutoValue_Animal`就是AutoValue自动生成的类。在AutoValue_Animal里面，它帮我们自动实现了`hashCode`, `equals`, `toString`等等这些重要的方法。这些方法的特征是实现的过程基本都一样，但是容易出错(由于粗心), 那么不如交给框架去自动产生。

如果你的类字段比较多，那么AutoValue还支持Builder模式:

```java
import com.google.auto.value.AutoValue;

@AutoValue
abstract class Animal {
  abstract String name();
  abstract int numberOfLegs();

  static Builder builder() {
    return new AutoValue_Animal.Builder();
  }

  @AutoValue.Builder
  abstract static class Builder {
    abstract Builder setName(String value);
    abstract Builder setNumberOfLegs(int value);
    abstract Animal build();
  }
}
```

这样我们就可以一步一步渐进地把对象构造出来了。

## Api Surface Test

所谓的API Surface是指我们一个系统暴露给另外一个系统一个SDK的时候，我们到底应该把哪些类，哪些package暴露给用户，这个问题很重要，因为考虑向后兼容性的话，暴露的package越少，将来修改的时候，破坏向后兼容性的可能性就越小，SDK就越稳定。我们平常的时候对于这种事情可能都是通过人肉分析、review，在Beam里面它直接编写成了一个单元测试:

```java
  @Test
  public void testSdkApiSurface() throws Exception {

    @SuppressWarnings("unchecked")
    final Set<String> allowed =
        ImmutableSet.of(
            "org.apache.beam",
            "com.fasterxml.jackson.annotation",
            "com.fasterxml.jackson.core",
            "com.fasterxml.jackson.databind",
            "org.apache.avro",
            "org.hamcrest",
            // via DataflowMatchers
            "org.codehaus.jackson",
            // via Avro
            "org.joda.time",
            "org.junit");

    assertThat(
        ApiSurface.getSdkApiSurface(getClass().getClassLoader()), containsOnlyPackages(allowed));
  }
```

这个测试表明，Beam的SDK向外暴露的package就是上面列出的这些，下次来个新手贡献的代码如果破坏了这个约定，那么这个单元测试就直接报错，无法提交merge。还是那句话，凡是能自动化检测的东西不要依靠人肉。这里涉及到的主要技术是:

1. 通过扫描classpath对指定包里面所有类的方法，参数，返回值进行检查。
2. `hamcrest`这个支持`matcher`的单元测试的库，它让你不只可以`assertTrue`, `assertFalse`, `assertEquals`, 而是可以自由指定需要满足的条件。

## 总结

给Beam贡献代码的时候最大感受在于，你不需要去问其它Commiter或者看什么文档去确认你的代码风格是否符合，是否用对了Maven版本，JDK的版本对不对，JavaDoc需不需要写等等，你只要编译下代码，maven会告诉你的代码是否OK。只要通过了编译，代码风格、一些小的错误等等基本不会有了，那么代码review的时候主要就集中在代码设计层面了。有用的maven插件还有很多，这里只是举了几个例子，我觉得比每个插件本身更重要的是:

1. 对代码质量的重视的意识。
2. 使用工具/代码（而不是文档、流程）来解决保障代码质量。