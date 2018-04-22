本文所阐述的内容均位于storm-server模块下的各包

# healthcheck

## HealthChecker

在storm中supervisor节点通过健康检查来确定本机的硬件情况，检查方式可分为手动检查和自动检查。

手动检查方式通过查看[官方文档](http://storm.apache.org/releases/2.0.0-SNAPSHOT/Setting-up-a-Storm-cluster.html)有如下描述：

Storm provides a mechanism by which administrators can configure the supervisor to run administrator supplied scripts periodically to determine if a node is healthy or not. Administrators can have the supervisor determine if the node is in a healthy state by performing any checks of their choice in scripts located in storm.health.check.dir. If a script detects the node to be in an unhealthy state, it must print a line to standard output beginning with the string ERROR. The supervisor will periodically run the scripts in the health check dir and check the output. If the script’s output contains the string ERROR, as described above, the supervisor will shut down any workers and exit.

If the supervisor is running with supervision **"/bin/storm node-health-check"** can be called to determine if the supervisor should be launched or if the node is unhealthy.

The health check directory location can be configured with:

```yaml
storm.health.check.dir: "healthchecks"
```

The scripts must have execute permissions. The time to allow any given healthcheck script to run before it is marked failed due to timeout can be configured with:

```yaml
storm.health.check.timeout.ms: 5000
```

即使用`/bin/storm node-health-check`shell命令来检测，但是我发现`storm.health.check.dir`目录下并不存在脚本。可以自己创建.sh脚本文件，异常的输出就是以ERROR开头的字符串。方法是在storm.home目录下创建healthchecks文件夹，其中放入一个healthchecks.sh脚本

```bash
#!/usr/bin/env bash
MAX_cpu=70
top_command=`which top`
cpuInfo=`$top_command -b -n 1 | grep "Cpu(s)" | awk '{print $2+$3}'`
Cpu=${cpuInfo/.*}
if [ $Cpu -gt $MAX_cpu ]
then
    echo "ERROR"
else
    echo "SUCCESS"
fi

```

这个脚本很简单，当cpu使用率超过了最大使用率70%，则输出ERROR。然后执行`/bin/storm node-health-check`shell命令或者直接观察supervisor.log都可以发现日志的变化。

如果是supervisor.log，则会显示以下结果：

```bash
2018-03-28 16:29:03.083 o.a.s.c.healthcheck [INFO] ()
2018-03-28 16:34:03.083 o.a.s.c.healthcheck [INFO] ()
2018-03-28 16:39:03.084 o.a.s.c.healthcheck [INFO] ()
2018-03-28 16:44:03.623 o.a.s.c.healthcheck [INFO] (["/opt/apps/apache-storm-1.0.1/healthchecks/healthCheck.sh" :success])
2018-03-28 16:49:04.157 o.a.s.c.healthcheck [INFO] (["/opt/apps/apache-storm-1.0.1/healthchecks/healthCheck.sh" :success])

```

可以看到在创建脚本之前都是输出空，有脚本之后，则输出脚本的结果。

如果是执行`/bin/storm node-health-check`shell命令，则会显示以下结果：

```bash
[root@emr-header-1 apache-storm-1.0.1]# ./bin/storm node-health-check
Running: /usr/lib/jvm/java/bin/java -client -Ddaemon.name= -Dstorm.options= -Dstorm.home=/opt/apps/apache-storm-1.0.1 -Dstorm.log.dir=/opt/apps/apache-storm-1.0.1/logs -Djava.library.path=/usr/local/lib:/opt/local/lib:/usr/lib -Dstorm.conf.file= -cp /opt/apps/apache-storm-1.0.1/lib/reflectasm-1.10.1.jar:/opt/apps/apache-storm-1.0.1/lib/storm-rename-hack-1.0.1.jar:/opt/apps/apache-storm-1.0.1/lib/minlog-1.3.0.jar:/opt/apps/apache-storm-1.0.1/lib/log4j-slf4j-impl-2.1.jar:/opt/apps/apache-storm-1.0.1/lib/clojure-1.7.0.jar:/opt/apps/apache-storm-1.0.1/lib/log4j-api-2.1.jar:/opt/apps/apache-storm-1.0.1/lib/slf4j-api-1.7.7.jar:/opt/apps/apache-storm-1.0.1/lib/asm-5.0.3.jar:/opt/apps/apache-storm-1.0.1/lib/storm-core-1.0.1.jar:/opt/apps/apache-storm-1.0.1/lib/servlet-api-2.5.jar:/opt/apps/apache-storm-1.0.1/lib/log4j-over-slf4j-1.6.6.jar:/opt/apps/apache-storm-1.0.1/lib/objenesis-2.1.jar:/opt/apps/apache-storm-1.0.1/lib/kryo-3.0.3.jar:/opt/apps/apache-storm-1.0.1/lib/disruptor-3.3.2.jar:/opt/apps/apache-storm-1.0.1/lib/log4j-core-2.1.jar:/opt/apps/apache-storm-1.0.1/conf:/opt/apps/apache-storm-1.0.1/bin org.apache.storm.command.healthcheck
3252 [main] INFO  o.a.s.c.healthcheck - (["/opt/apps/apache-storm-1.0.1/healthchecks/healthCheck.sh" :success])

```

该命令通过storm.py的healthcheck函数运行org.apache.storm.command.healthcheck类的main方法来执行HealthChecker类。

现在有个问题，当把这个类的processScript方法模拟一遍：

```java
private String processScript(String script) {
  Thread interruptThread = null;
  try{
    Process process = Runtime.getRuntime().exec(script);
    final long timeout = 5000L;
    final Thread curThread = Thread.currentThread();//当前运行线程
    interruptThread = new Thread(() -> {
      try {
        Thread.sleep(timeout);
        curThread.interrupt();
      } catch (InterruptedException e) {
        //ignore
      }

    });
    interruptThread.start();
    process.waitFor();
    interruptThread.interrupt();
    curThread.isInterrupted();
    LOG.info("process.exitValue() = " + process.exitValue());
    if (process.exitValue() != 0){
      String str;
      InputStream stdin = process.getInputStream();
      BufferedReader reader = new BufferedReader(new InputStreamReader(stdin));
      while ((str = reader.readLine()) != null){
        if (str.startsWith("ERROR")){
          return FAILED;
        }
      }
      return SUCCESS;
    }
    LOG.warn("The healthcheck process {} exited with code {}", script, process.exitValue());
    return FAILED_WITH_EXIT_CODE;
  } catch (IOException | InterruptedException e) {
    LOG.warn("Script:  {} timed out.", script);
    LOG.error(e.getMessage());
    return TIMEOUT;
  } finally {
    if (interruptThread != null)
      interruptThread.interrupt();
  }
}
```

然后编写一个测试方法：

```java
@Test
public void test(){
  String exeFile = "C:\\Program Files (x86)\\python\\python.exe";
  String jarFile = "D:\\workspace\\maventest\\target\\pengxx-test-storm.jar";
  File file = new File(jarFile);
  if (!file.exists()){
    System.out.println("ERROR");
  }else {
    System.out.println("exists");
  }
  String jarCmd = "java -jar "+jarFile;
  System.out.println(processScript(jarCmd));
}
```

测试方法主要运行一个helloword的jar包，运行之后返回结果为：

```bash
exists
2018-03-28 17:45:02,041  INFO [main] (HealthChecker.java:processScript:82) - process.exitValue() = 0
2018-03-28 17:45:02,047  WARN [main] (HealthChecker.java:processScript:94) - The healthcheck process java -jar D:\workspace\maventest\target\pengxx-test-storm.jar exited with code 0
failed_with_exit_code
```

即最终的输出是failed_with_exit_code，不是success。但是结果中显示了process.exitValue()的返回值是0，也就是说执行成功了。**因此对这个processScript方法表示怀疑，执行太快了会报failed_with_exit_code错误，执行太慢了会报timeout错误。**我觉得一个好的方法是对返回值是0的情况也要进行处理。

## Process

这个类主要是用于执行命令行。日常java开发中，有时需要通过java运行其它应用功程序，比如shell命令等。jdk的Runtime类提供了这样的方法。首先来看Runtime类的文档, 从文档中可以看出，每个java程序只会有一个Runtime实例，显然这是一个单例模式。

### Runtime.exec()常见的几种陷阱以及避免方法

#### 陷阱1：IllegalThreadStateException

通过exec执行java命令为例子，最简单的方式如下。执行exec后，通过Process获取外部进程的返回值并输出。

```java
import java.io.IOException;
public class Main {
    public static void main(String[] args) {
        Runtime runtime = Runtime.getRuntime();
        try {
            Process process = runtime.exec("java");
            int exitVal = process.exitValue();
            System.out.println("process exit value is " + exitVal);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

很遗憾的是，我们发现输出结果如下，抛出了IllegalThreadStateException异常

```java
Exception in thread "main" java.lang.IllegalThreadStateException: process has not exited
at java.lang.ProcessImpl.exitValue(ProcessImpl.java:443)
at com.baidu.ubqa.agent.runner.Main.main(Main.java:18)
at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
at java.lang.reflect.Method.invoke(Method.java:497)
at com.intellij.rt.execution.application.AppMain.main(AppMain.java:144)
```

为什么会抛出IllegalThreadStateException异常？

这是因为外部线程还没有结束，这个时候去获取退出码，exitValue()方法抛出了异常。看到这里读者可能会问，为什么这个方法不能阻塞到外部进程结束后再返回呢？确实如此，Process有一个waitFor()方法，就是这么做的，返回的也是退出码。因此，我们可以用waitFor()方法替换exitValue()方法。

### 陷阱2：Runtime.exec()可能hang住，甚至死锁

从这里可以看出，Runtime.exec()创建的子进程公用父进程的流，不同平台上，父进程的stream buffer可能被打满导致子进程阻塞，从而永远无法返回。
针对这种情况，我们只需要将子进程的stream重定向出来即可。

```java
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main {
    public static void main(String[] args) {
        Runtime runtime = Runtime.getRuntime();
        try {
            Process process = runtime.exec("java");
            BufferedReader stdoutReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            BufferedReader stderrReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            String line;
            System.out.println("OUTPUT");
            while ((line = stdoutReader.readLine()) != null) {
                System.out.println(line);
            }
            System.out.println("ERROR");
            while ((line = stderrReader.readLine()) != null) {
                System.out.println(line);
            }
            int exitVal = process.waitFor();
            System.out.println("process exit value is " + exitVal);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
```

### 陷阱3：错把Runtime.exec()的command参数当做命令行

本质上来讲，Runtime.exec()的command参数只是一个可运行的命令或者脚本，并不等效于Shell解器或者Cmd.exe,如果你想进行输入输出重定向，pipeline等操作，则必须通过程序来实现。不能直接在command参数中做。例如，下面的例子

```java
Process process = runtime.exec("java -version > a.txt");
```

这样并不会产出a.txt文件。要达到这种目的，需通过编程手段实现，如下

```java
class StreamGobbler extends Thread {
    InputStream is;
    String type;
    OutputStream os;

    StreamGobbler(InputStream is, String type) {
        this(is, type, null);
    }

    StreamGobbler(InputStream is, String type, OutputStream redirect) {
        this.is = is;
        this.type = type;
        this.os = redirect;
    }

    public void run() {
        try {
            PrintWriter pw = null;
            if (os != null)
                pw = new PrintWriter(os);

            InputStreamReader isr = new InputStreamReader(is);
            BufferedReader br = new BufferedReader(isr);
            String line;
            while ((line = br.readLine()) != null) {
                if (pw != null)
                    pw.println(line);
                System.out.println(type + ">" + line);
            }
            if (pw != null)
                pw.flush();
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }
}

public class Main {
    public static void main(String args[]) {
        try {
            FileOutputStream fos = new FileOutputStream("logs/a.log");
            Runtime rt = Runtime.getRuntime();
            Process proc = rt.exec("cmd.exe /C dir");

            // 重定向输出流和错误流
            StreamGobbler errorGobbler = new StreamGobbler(proc.getErrorStream(), "ERROR");
            StreamGobbler outputGobbler = new StreamGobbler(proc.getInputStream(), "OUTPUT", fos);

            errorGobbler.start();
            outputGobbler.start();
            int exitVal = proc.waitFor();
            System.out.println("ExitValue: " + exitVal);
            fos.flush();
            fos.close();
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
```

参考：[java Runtime.exec()执行shell/cmd命令：常见的几种陷阱与一种完善实现](https://www.jianshu.com/p/af4b3264bc5d)

# event



# security

### Java安全之认证与授权

Java平台提供的认证与授权服务(Java Authentication and Authorization Service (JAAS))，能够控制代码对敏感或关键资源的访问，例如文件系统，网络服务，系统属性访问等，加强代码的安全性。主要包含认证与授权两部分，认证的目的在于可靠安全地确定当前是谁在执行代码，代码可以是一个应用，applet，bean，servlet；授权的目的在于确定了当前执行代码的用户有什么权限，资源是否可以进行访问。虽然JAAS表面上分为了两大部分，而实际上两者是密不可分的，下面看一段代码：

```java
public class App {  
  public static void main(String[] args) {  
    System.out.println(System.getProperty("java.home"));  
  }  
} 
```

非常简单只是输出java.home系统属性，现在肯定是没有任何问题，属性会能正常输出。把上述代码改为如下后：

```java
public class App {  
    public static void main(String[] args) {  
        //安装安全管理器  
        System.setSecurityManager(new SecurityManager());  

        System.out.println(System.getProperty("java.home"));  
    }  
}  
```

抛出了如下异常：java.security.AccessControlException: access denied (“java.util.PropertyPermission” “java.home” “read”)，异常提示没有对java.home的读取权限，系统属性也是一种资源，与文件访问类似；默认情况下对于普通Java应用是没有安装安全管理器，在手动安装安全管理器后，如果没有为应用授权则没有任何权限，所以应用无法访问java.home系统属性。

授权的方式是为安全管理器绑定一个授权策略文件。由于我是在eclipse Java工程中直接运行main方法，所以就在工程根目录下新建一个demo.policy文件，文件内容如下：

```java
grant  {  
    permission java.util.PropertyPermission "java.home", "read";  
};
```

具体的授权可以看[oracle官方例子](https://docs.oracle.com/javase/7/docs/technotes/guides/security/jaas/tutorials/GeneralAcnOnly.html)。

在上述过程中虽然完成了授权，但授权的针对性不强，在程序绑定了该policy文件后，无论是哪个用户执行都将拥有java.home系统属性的读权限，现在我们希望做更加细粒度的权限控制，这里需要用到认证服务了。

认证服务有点“麻烦”，一般情况下主要都涉及到了LoginContext，LoginModule，CallbackHandler，Principal，后三者还需要开发者自己实现。这里先解释一下这几个类的作用： 

1.  LoginContext:认证核心类，也是入口类，用于触发登录认证，具体的登录模块由构造方法name参数指定 
2.  LoginModule:登录模块，封装具体的登录认证逻辑，如果认证失败则抛出异常，成为则向Subject中添加一个Principal 
3.  CallbackHandler:回调处理器，用于搜集认证信息 
4.  Principal:代表程序用户的某一身份，与其密切相关的为Subject，用于代表程序用户，而一个用户可以多种身份，授权时可以针对某用户的多个身份分别授权

下面看一个认证例子：

App.java

```java

import javax.security.auth.Subject;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;
import java.io.File;
import java.security.PrivilegedAction;

/**
 * @author pengxingxiong(0016004591) 2018/3/29
 */
public class App {
    public static void main(String[] args) {
        String rootPath = System.getProperty("user.dir") + File.separator + "src" + File.separator + "main"
                + File.separator + "resources" + File.separator + "security" + File.separator;
        String configPath = rootPath + "demo.config";
        String policyPath = rootPath + "demo.policy";
        System.setProperty("java.security.auth.login.config", configPath);
        System.setProperty("java.security.policy", policyPath);
        System.setSecurityManager(new SecurityManager());
        try {
            //创建登录上下文
            //认证核心类，也是入口类，用于触发登录认证，具体的登录模块由构造方法name参数指定
            // 这个demo上下文需要在policy文件中配置权限
            LoginContext context = new LoginContext("demo", new DemoCallbackHander());
            //进行登录，登录不成功则系统退出
            context.login();
            Subject subject = context.getSubject();
            //该方法调用需要"doAsPrivileged"权限，在policy文件中配置
            Subject.doAsPrivileged(subject, (PrivilegedAction<Object>) () -> {
                System.out.println(System.getProperty("java.home"));
                System.out.println(System.getProperty("user.dir"));
                return null;
            }, null);
        } catch (LoginException | SecurityException le) {
            System.err.println("Cannot create LoginContext. " + le.getMessage());
            le.printStackTrace();
            System.exit(-1);
        }
    }
}

```
DemoLoginModule.java
```java
/**登录模块，封装具体的登录认证逻辑，如果认证失败则抛出异常，成为则向Subject中添加一个Principal */
public class DemoLoginModule implements LoginModule {
    private Subject subject;
    private CallbackHandler callbackHandler;
    private boolean success = false;
    private String user;
    private String password;
    Map<String, ?> options;

    @Override
    public void initialize(Subject subject, CallbackHandler callbackHandler, Map<String, ?> sharedState, Map<String, ?> options) {
        this.subject = subject;
        this.callbackHandler = callbackHandler;
        //从配置文件中读取到了demo这个权限管理必须是配置文件中写入的用户名和密码才行
        System.out.println("DemoLoginModule.initialize().options="+options);
        this.options = options;
        System.out.println("DemoLoginModule.initialize().sharedState="+sharedState);
    }
    @Override
    public boolean login() throws LoginException {
        NameCallback nameCallback = new NameCallback("请输入用户名");
        PasswordCallback passwordCallback = new PasswordCallback("请输入密码", false);
        Callback[] callbacks = new Callback[]{nameCallback, passwordCallback};
        try {
            //执行回调，回调过程中获取用户名与密码
            //回调函数中，对callbacks变量进行了修改
            callbackHandler.handle(callbacks);
            //得到用户名与密码
            user = nameCallback.getName();
            password = new String(passwordCallback.getPassword());
            System.out.println("user:"+ user + "\tpassword:" + password);
            String sslUser = options.get("username").toString();
            String sslPassword = options.get("password").toString();
            if (!sslUser.equals(user) || !sslPassword.equals(password)){
                System.out.println("该用户无权限");
                System.exit(1);
            }
        } catch (IOException | UnsupportedCallbackException e) {
            success = false;
            throw new FailedLoginException("用户名或密码获取失败");
        }
        //为简单起见认证条件写死
        if(user.length()>3 && password.length()>3) {
            success = true;//认证成功
        }
        return true;
    }
    @Override
    public boolean commit() throws LoginException {
        if(!success) {
            return false;
        } else {
            //如果认证成功则得subject中添加一个Principal对象
            //这样某身份用户就认证通过并登录了该应用，即表明了谁在执行该程序
            this.subject.getPrincipals().add(new DemoPrincipal(user));
            return true;
        }
    }
    @Override
    public boolean abort() throws LoginException {
        logout();
        return true;
    }
    @Override
    public boolean logout() throws LoginException {
        //退出时将相应的Principal对象从subject中移除
        Iterator<Principal> iter = subject.getPrincipals().iterator();
        while(iter.hasNext()) {
            Principal principal = iter.next();
            if(principal instanceof DemoPrincipal) {
                if(principal.getName().equals(user)) {
                    iter.remove();
                    break;
                }
            }
        }
        return true;
    }
}
```

DemoCallbackHander.java

```java
/**回调处理器，用于搜集认证信息*/
public class DemoCallbackHander implements CallbackHandler {

    @Override
    public void handle(Callback[] callbacks) throws IOException, UnsupportedCallbackException {
        NameCallback nameCallback = (NameCallback) callbacks[0];
        PasswordCallback passwordCallback = (PasswordCallback) callbacks[1];
        //设置用户名与密码
        nameCallback.setName(getUserFromSomeWhere());
        passwordCallback.setPassword(getPasswordFromSomeWhere().toCharArray());
    }


    //为简单起见用户名与密码写死直接返回，真实情况可以由用户输入等具体获取
    public String getUserFromSomeWhere() {
        return "peng";
    }
    public String getPasswordFromSomeWhere() {
        return "pengxx";
    }
}
```

DemoPrincipal.java

```java
/**代表程序用户的某一身份，与其密切相关的为Subject，用于代表程序用户，而一个用户可以多种身份，授权时可以针对某用户的多个身份分别授权*/
public class DemoPrincipal implements Principal {
    private String name;
    public DemoPrincipal(String name) {
        this.name = name;
    }
    @Override
    public String getName() {
        return this.name;
    }
}
```

使用认证服务时，需要绑定一个认证配置文件，因为在App类中的LoginContext用到了demo名称作为认证上下文。因此我们命名一个demo.config配置文件：

demo.config

```java
demo {
   storm.security.DemoLoginModule required
   username="peng"
   password="pengxx";
};
```

该配置的意思是对于demo认证而言，对应到DemoLoginModule类，需要传入的kv值为名称和密码。这个名称和密码在DemoLoginModule类的initialize方法中可以拿到。也就是说代码中可以写逻辑判断名称和密码，然后和配置文件中的匹配，匹配上了就通过认证。

另外，如果不知道写啥，即kv为空，这种方式为设置虚拟属性可以写成以下形式：

```java
demo {
   storm.security.DemoLoginModule required
   debug=true;
};
```

# thrift

