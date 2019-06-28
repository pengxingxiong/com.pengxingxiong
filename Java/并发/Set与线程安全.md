Set的线程不安全
构建线程安全的Set
一般写法
虽然Set是非线程安全的，但是set的底层是使用Map实现的，顾可以通过ConcurrentHashMap的方式变通实现线程安全的Set

```java
public class SafeSet{
    Set s = new HashSet<String>;
    Map<String,Boolean> m = new ConcurrentHash<>();
    public void add(String st) {
        s.add(st);
        m.put(st);
    }
}
```

上面只是一个case，用意使用线程安全的ConcurrentHashMap生成线程安全的Set；

使用Guava构建ConcurrentHashSet
谷歌的guava其实已经实现了线程安全的ConcurrentHashSet，

```java
Set<String> s = Sets.newConcurrentHashSet();
```

下面是源码：
```java

  public static <E> Set<E> newConcurrentHashSet() {
    return newSetFromMap(new ConcurrentHashMap<E, Boolean>());
  }

  static <E> Set<E> newSetFromMap(Map<E, Boolean> map) {
    return Collections.newSetFromMap(map);
  }
```
这里面用到了java.util中的一个方法，可以从map转化为set，下文会说到；
```java
Collections.newHashSetFromMap
```
这个方法存在java.util.Collections类中，没有考究过究竟是从jdk1. 
5还是1.6出现的； 
其实质还是通过ConcurrentHashMap实现线程安全的：
```
public static <E> Set<E> newSetFromMap(Map<E, Boolean> map) {
        return new SetFromMap<>(map);
}
```

```java
/**
 * @serial include
 */
private static class SetFromMap<E> extends AbstractSet<E>
    implements Set<E>, Serializable
{
    private final Map<E, Boolean> m;  // The backing map
    private transient Set<E> s;       // Its keySet

    SetFromMap(Map<E, Boolean> map) {
        if (!map.isEmpty())
            throw new IllegalArgumentException("Map is non-empty");
        m = map;
        s = map.keySet();
    }

    public void clear()               {        m.clear(); }
    public int size()                 { return m.size(); }
    public boolean isEmpty()          { return m.isEmpty(); }
    public boolean contains(Object o) { return m.containsKey(o); }
    public boolean remove(Object o)   { return m.remove(o) != null; }
    public boolean add(E e) { return m.put(e, Boolean.TRUE) == null; }
    public Iterator<E> iterator()     { return s.iterator(); }
    public Object[] toArray()         { return s.toArray(); }
    public <T> T[] toArray(T[] a)     { return s.toArray(a); }
    public String toString()          { return s.toString(); }
    public int hashCode()             { return s.hashCode(); }
    public boolean equals(Object o)   { return o == this || s.equals(o); }
    public boolean containsAll(Collection<?> c) {return s.containsAll(c);}
    public boolean removeAll(Collection<?> c)   {return s.removeAll(c);}
    public boolean retainAll(Collection<?> c)   {return s.retainAll(c);}
    // addAll is the only inherited implementation

    // Override default methods in Collection
    @Override
    public void forEach(Consumer<? super E> action) {
        s.forEach(action);
    }
    @Override
    public boolean removeIf(Predicate<? super E> filter) {
        return s.removeIf(filter);
    }

    @Override
    public Spliterator<E> spliterator() {return s.spliterator();}
    @Override
    public Stream<E> stream()           {return s.stream();}
    @Override
    public Stream<E> parallelStream()   {return s.parallelStream();}

    private static final long serialVersionUID = 2454657854757543876L;

    private void readObject(java.io.ObjectInputStream stream)
        throws IOException, ClassNotFoundException
    {
        stream.defaultReadObject();
        s = m.keySet();
    }
}
```
