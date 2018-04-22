# 1 注解指南
@Data ：注解在类上；提供类所有属性的 getting 和 setting 方法，此外还提供了equals、canEqual、hashCode、toString 方法

@Setter：注解在属性上；为属性提供 setting 方法

@Getter：注解在属性上；为属性提供 getting 方法

@Log4j ：注解在类上；为类提供一个 属性名为log 的 log4j 日志对象

@NoArgsConstructor：注解在类上；为类提供一个无参的构造方法

@AllArgsConstructor：注解在类上；为类提供一个全参的构造方法

@NonNull：注解在参数上 如果该参数为null 会throw new NullPointerException(参数名);

@Cleanup：注释在引用变量前：自动回收资源 默认调用close方法

　　@Cleanup("dispose") org.eclipse.swt.widgets.CoolBar bar = new CoolBar(parent, 0);

　　@Cleanup InputStream in = new FileInputStream(args[0]);

　　@Cleanup OutputStream out = new FileOutputStream(args[1]);

@Builder：注解在类上；为类提供一个内部的Builder

# 2 详细讲解
## @Data
最常用的注解，编译时自动添加Setter、Getter、toString（）、equals()和hashCode()。

```java
import java.util.Date;

import lombok.Data;

@Data
public class User {
    private Integer id;
    private String userName;
    private String password;
    private String email;
    private Integer age;
    private Date signupTime;
    public static void main(String[] args) {
           User user=new User();
           user.setId(1001);
           user.setUserName("pollyduan");
           user.setPassword("123456");
           user.setEmail("pollyduan@pollyduan.com");
           user.setAge(30);
           user.setSignupTime(new Date());
           System.out.println(user);
       System.out.println(user.getUserName());
       System.out.println(user.hashCode());
        }
}
```
使用场景：

POJO类、hibernate的实体类、json或jaxb的实体类。
## @Value
如果我们需要一个不可变的对象类，那么就用该注解。它在编译是自动添加Getter、toString()、equals()、hashCode()以及一个全参的构造器。

注：没有无参构造器。如果需要，自己添加一个，或者增加一个后面介绍的lombok.NoArgsConstructor注解。
```java
import java.util.Date;

import lombok.Value;

@Value
public class User {
    private Integer id;
    private String userName;
    private String password;
    private String email;
    private Integer age;
    private Date signupTime;

    public static void main(String[] args) {
        /*
         * User user=new User();//The constructor User() is undefined
         * user.setId(1001);//The method setId(int) is undefined for the type
         * User
         */
        User user = new User(1001, "pollyduan", "123456", "pollyduan@pollyduan.com", 30, new Date());
        System.out.println(user);
        System.out.println(user.getUserName());
        System.out.println(user.hashCode());
    }
}
```
如果自定义了自动生成的方法，以自己定义的为准。
## @Builder
它把我们的Bean类包装为一个构建者模式，编译时增加了一个Builder内部类和全字段的构造器。

注：没有Getter、Setter、toString()。如需其他方法，可以自己实现或者配合其他注解。
```java
import java.util.Date;

import lombok.Builder;
import lombok.Data;

@Builder
public class User {
    private Integer id;
    private String userName;
    private String password;
    private String email;
    private Integer age;
    private Date signupTime;

    public static void main(String[] args) {
        /*
         * User user=new User();//The constructor User() is undefined
         */
        User user = new User(1001, "pollyduan", "123456", "pollyduan@pollyduan.com", 30, new Date());
        //或者
        user=User.builder()
                .age(30)
                .userName("pollyduan")
                .build();
        System.out.println(user);
    }
}
```
## 构造器注解
提供了三个构造器注解，分别为:
```java
lombok.AllArgsConstructor 增加全参构造器
lombok.NoArgsConstructor 增加无参构造
lombok.RequiredArgsConstructor 增加必选参数构造器
```
该注解可同时标注，以增加不同的构造器。

可以使用access属性定制访问级别，如:”access = AccessLevel.PROTECTED”

前两个比较简单，必选参数构造器需要配合 lombok.NonNull 注解使用，只有标记了 NonNull 注解的字段才会被纳入 RequiredArgsConstructor 构造器中。
```java
import java.util.Date;

import lombok.NonNull;
import lombok.RequiredArgsConstructor;

@RequiredArgsConstructor
public class User {
    @NonNull
    private Integer id;
    @NonNull
    private String userName;
    @NonNull
    private String password;
    private String email;
    private Integer age;
    private Date signupTime;

    public static void main(String[] args) {
        /*
         * User user=new User();
         * User user = new User(1001, "pollyduan", "123456", "pollyduan@pollyduan.com", 30, new Date());
         * //The constructor User() is undefined
         */
        User user=new User(1001, "pollyduan", "123456");//ok
        System.out.println(user);
    }
}
```
## 定制单个方法
lombok.ToString 这个简单，就是增加toString()方法。

类似的还有：

lombok.EqualsAndHashCode 增加equals() 和 hashCode（）。

lombok.Getter 增加Getter方法

lombok.Setter 增加Setter方法
## @Cleanup
```java
import lombok.Cleanup;

public class MyStream {
    public void close() {
        System.out.println("close.");
    }

    public static void main(String[] args) {
        System.out.println("new a mystream object.");
        @Cleanup
        MyStream ms=new MyStream();
        System.out.println("bye.");
    //退出前会自动调用close()
    }
}
```
## 日志相关注解
import lombok.extern.java.Log;

@Log
public class User {
    public static void main(String[] args) {
        System.out.println(log.getClass());
    log.info("app log.");
    }
}
该组注解包括：
```java
@CommonsLog
    Creates private static final org.apache.commons.logging.Log log = org.apache.commons.logging.LogFactory.getLog(LogExample.class);

@JBossLog
    Creates private static final org.jboss.logging.Logger log = org.jboss.logging.Logger.getLogger(LogExample.class);

@Log
    Creates private static final java.util.logging.Logger log = java.util.logging.Logger.getLogger(LogExample.class.getName());

@Log4j
    Creates private static final org.apache.log4j.Logger log = org.apache.log4j.Logger.getLogger(LogExample.class);

@Log4j2
    Creates private static final org.apache.logging.log4j.Logger log = org.apache.logging.log4j.LogManager.getLogger(LogExample.class);

@Slf4j
    Creates private static final org.slf4j.Logger log = org.slf4j.LoggerFactory.getLogger(LogExample.class);

@XSlf4j
    Creates private static final org.slf4j.ext.XLogger log = org.slf4j.ext.XLoggerFactory.getXLogger(LogExample.class);
```
注：常用的日志处理器都在，一般我们使用log4j或slf4j。

没有logback，请使用slf4j代理logback。
## @Getter(lazy = true) 懒加载
如果Bean的一个字段的初始化是代价比较高的操作，比如加载大量的数据；同时这个字段并不是必定使用的。那么使用懒加载机制，可以保证节省资源。

懒加载机制，是对象初始化时，该字段并不会真正的初始化；而是第一次访问该字段时才进行初始化字段的操作。

一言不合贴代码：
```java
import lombok.Data;
import lombok.Getter;

@Data
public class GetterLazyExample {
    @Getter(lazy = true)
    private final int[] cached = expensive();
    private Integer id;

    private int[] expensive() {
        int[] result = new int[100];
        for (int i = 0; i < result.length; i++) {
            result[i] = i;
            System.out.println(i);
        }
        System.out.println("cached 初始化完成。");
        return result;
    }
    public static void main(String[] args) {
        GetterLazyExample obj=new GetterLazyExample();
        obj.setId(1001);
        System.out.println("打印id："+obj.getId());
        System.out.println("cached 还没有初始化哟。");
        // obj.getCached();
    }
}
```
运行就会发现，cached这个字段并没有初始化，虽然看起来它是final的，并直接赋值使用expensive()进行初始化。

运行结果:
```
打印id：1001
cached 还没有初始化哟
```
## @Synchronized
同步方法注解。添加了该注解的方法，其方法体都会自动包含在一个synchronize块中。如：
```java
import java.util.concurrent.BlockingQueue;

import lombok.AllArgsConstructor;
import lombok.Synchronized;

@AllArgsConstructor
public class SynchronizedExample {
    private BlockingQueue<String> queue;

    @Synchronized("queue")
    public void sync1() throws Exception {
        System.out.println("sync1.");
    }

    @Synchronized("queue")
    public void sync2() throws Exception {
        System.out.println("sync2.");
    }

    @Synchronized
    public void sync3() throws Exception {
        System.out.println("sync3.");
    }
}
```
如果直接指定了value=queue，其中queue为类的一个成员，那么该方法使用该成员queue作为加锁对象，放在同步块中执行。那么本例中，sync1和sync2是互斥的，sync1没有执行完之前，sync2会被挂起，等待sync1执行完成之后才可以执行。

sync3，没有指定注解属性，这时lombok会自动创建一个对象作为锁，这样的结果是sync3自身互斥，多线程中两个线程不能同时执行sync3方法。

sync3等同于：
```java
private final Object $lock = new Object[0];//lombok添加的
public void sync3() throws Exception {
    synchronized($lock){
        System.out.println("sync3.");
    }
}
```
注：因为sync3与sync1使用的不是同一个锁，那么他们没有互斥关系，sync2也一样。

一定要理清楚锁的关系，否则不要轻易使用该注解。
## @SneakyThrows隐藏异常
自动捕获检查异常。

我们知道，java对于检查异常，需要在编码时进行捕获，或者throws抛出。

该注解的作用是将检查异常包装为运行时异常，那么编码时就无需处理异常了。

提示：不过这并不是友好的编码方式，因为你编写的api的使用者，不能显式的获知需要处理检查异常。
```java
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.UnsupportedEncodingException;

import lombok.SneakyThrows;

public class SneakyThrowsExample {
    @SneakyThrows({UnsupportedEncodingException.class})
    public void test(byte[] bytes) {
        String str = new String(bytes, "UTF8");
    }
    @SneakyThrows({UnsupportedEncodingException.class,FileNotFoundException.class})
    public void test2(byte[] bytes) {
        FileInputStream file=new FileInputStream("no_texists.txt");
        String str=new String(bytes, "UTF8");
    }
    @SneakyThrows
    public void test3(byte[] bytes) {
        FileInputStream file=new FileInputStream("no_texists.txt");
        String str=new String(bytes, "UTF8");
    }

}
```
注解接受一个class数组的value属性，如果未指定value属性，默认捕获所有异常。

以上代码相当于：
```java
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.UnsupportedEncodingException;

import lombok.SneakyThrows;

public class SneakyThrowsExample {
    public void test(byte[] bytes) {
        try {
            String str = new String(bytes, "UTF8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }
    public void test2(byte[] bytes) {
        try {
            FileInputStream file=new FileInputStream("no_texists.txt");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        try {
            String str=new String(bytes, "UTF8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }
    public void test3(byte[] bytes) {
        try {
            FileInputStream file=new FileInputStream("no_texists.txt");
            String str=new String(bytes, "UTF8");
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

}
```
注：个人建议，了解即可，非必要不要使用。隐藏了异常细节，你的使用者会骂死你。
## 辅助注解@NonNull
前面已经使用过了，标记在字段上，表示非空字段。

也可以标注在方法参数上，会在第一次使用该参数是判断是否为空。