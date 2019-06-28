在业务场景中存在需要创建多实例来完成多线程操作，不同于线程池的是，这些线程都具有指定的线程名称以及对应的缓存桶，例如thread-1只能处理bucket-1的数据。这个就需要创建多个实例。具体方案如下：

在需要多实例的类上加入注解：

```java
@Slf4j
@Component("PacketConverter")
@Scope("prototype")
public class PacketConverter extends Thread {
  ...
}
```

给定Bean的名称以及实例化策略。

然后创建一个ApplicationContextProvider类

```java
/**
 * Created by pengxingxiong@ruijie.com.cn on 2019/4/24 11:58
 * spring上下文处理工具
 */
@Component
public class ApplicationContextProvider implements ApplicationContextAware {

    private static ApplicationContext context;

    private ApplicationContextProvider() {
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        context = applicationContext;
    }

    /**
     * Created by pengxingxiong@ruijie.com.cn on 2019/5/15 11:57
     * 获取某个类的bean，用于创建bean，用于多实例的场景
     */
    public static <T> T getBean(String name, Class<T> aClass) {
        return context.getBean(name, aClass);
    }
}
```

最后在需要的地方调用getBean()实例化即可：

```java
while (i < bucketNum) {
            //TODO 多线程的创建方式
            PacketConverter pct = ApplicationContextProvider.getBean("PacketConverter", PacketConverter.class);
            pct.setName("PacketConverter-" + i);
            SIConfiguration.pcThread.add(pct);
            i++;
        }
```

