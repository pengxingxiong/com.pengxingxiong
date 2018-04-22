# Utils类

## asyncLoop()

Convenience method used when only the function and name suffix are given.

当仅仅给出方法的代码、名称前缀，就能够方便的使用该方法

```java
/**
     * Convenience method used when only the function and name suffix are given.
     * @param afn the code to call on each iteration每次线程迭代时调用的代码
     * @param threadName a suffix to be appended to the thread name给线程加一个前缀名
     * @return the newly created thread 创建的新线程
     * @see Thread
     */
public static SmartThread asyncLoop(final Callable afn, String threadName, final Thread.UncaughtExceptionHandler eh) {
  return asyncLoop(afn, false, eh, Thread.NORM_PRIORITY, false, true,
                   threadName);
}
```

另外可以看到还传入了一个异常类UncaughtExceptionHandler，再看方法内部的asyncLoop：

```java
/**
     * Creates a thread that calls the given code repeatedly, sleeping for an
     * interval of seconds equal to the return value of the previous call.
     *<p>创建一个重复调用给定代码的线程，休眠的时间间隔等于上一次调用的返回值。</p>
     * The given afn may be a callable that returns the number of seconds to
     * sleep, or it may be a Callable that returns another Callable that in turn
     * returns the number of seconds to sleep. In the latter case isFactory.
     *<p>给定的afn可能是一个可调用的函数，它返回睡眠的秒数，也可能是一个Callable返回另一个Callable，然后返回睡眠的秒数。 在后者的情况下是工厂。</p>
     * @param afn the code to call on each iteration
     * @param isDaemon whether the new thread should be a daemon thread 是否守护进程
     * @param eh code to call when afn throws an exception 当调用函数出现异常时希望做的处理
     * @param priority the new thread's priority 新线程的优先级
     * @param isFactory whether afn returns a callable instead of sleep seconds
     *                  是否一个callable返回另外一个callable而不是睡眠时间
     * @param startImmediately whether to start the thread before returning
     *                         是否在返回之前启动线程
     * @param threadName a suffix to be appended to the thread name
     * @return the newly created thread
     * @see Thread
     */
public static SmartThread asyncLoop(final Callable afn,
            Boolean isDaemon, final Thread.UncaughtExceptionHandler eh,
            int priority, final Boolean isFactory, Boolean startImmediately,
            String threadName) {
    SmartThread thread = new SmartThread(new Runnable() {
        public void run() {
            Object s;
            try {
                Callable fn = isFactory ? (Callable) afn.call() : afn;
                while ((s = fn.call()) instanceof long) {
                    Time.sleepSecs((long) s);
                }
            }
            catch (Throwable t) {
                if (exceptionCauseIsInstanceOf(
                                            InterruptedException.class, t)) {
                    LOG.info("Async loop interrupted!");
                    return;
                }
                LOG.error("Async loop died!", t);
                throw new RuntimeException(t);
            }
        }
    }
    );
    if (eh != null) {
        thread.setUncaughtExceptionHandler(eh);
    } else {
        thread.setUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
            public void uncaughtException(Thread t, Throwable e) {
                LOG.error("Async loop died!", e);
                exitProcess(1, "Async loop died!");
            }
        }
        );
    }
    thread.setDaemon(isDaemon);
    thread.setPriority(priority);
    if (threadName != null && !threadName.isEmpty()) {
        thread.setName(thread.getName() +"-"+ threadName);
    }
    if (startImmediately) {
        thread.start();
    }
    return thread;
}
```

