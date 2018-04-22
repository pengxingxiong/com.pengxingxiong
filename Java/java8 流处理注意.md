## boxed函数
boxed函数是一种装箱的意思，例如当前是IntStream/DoubleStream等特化类型的流，那么为了在之后使用`Stream<Interge>、Stream<int[]>、Stream<Double>`等Stream类型，需要使用boxed函数将特化流转化为一般流
例子：
```java
/**
 * 数值流应用：勾股数
 * 理解装箱boxed函数和值转换mapToObj函数
 *
 * @author pengxingxiong
 */
@Test
public void gougu() {
    Stream<double[]> result = IntStream.rangeClosed(1, 100)
            .boxed()//int流转换为一般流，因为只有一般流才能进行下一步的各种数据类型变化
            .flatMap(a ->
                    IntStream.rangeClosed(a, 100)
                            .mapToObj(b -> new double[]{a, b, Math.sqrt(a * a + b * b)})//mapToObj将int值转化为obj的值，便于int->double
                            .filter(array -> array[2] % 1 == 0)
            );
    //double->int
    result.map(doubleArray -> new int[]{(int) doubleArray[0], (int) doubleArray[1], (int) doubleArray[2]})
            .forEach(intArray -> System.out.println(intArray[0] + "," + intArray[1] + "," + intArray[2]));
}
```
在这个例子里面，将IntStream类型的特化流使用boxed函数转化为了一般流才能供flatMap使用；下面使用了mapToObj函数，是将int值转化为Object值，为了便于将int顺利转成double[]类型。

无论值转化还是流转化其实都需要满足特殊->一般才能继续使用。

## 文件流中的单词处理
```java
@Test
public void fileStreamStudy() {
    String path = "D:/log/test.txt";
    try {
        Stream<String> lines = Files.lines(Paths.get(path), Charset.forName("UTF-8"));
        long uniqueWords = lines.flatMap(line -> Arrays.stream(line.split(" ")))
                .distinct()
                .count();
        System.out.println(uniqueWords);
    } catch (IOException e) {
        e.printStackTrace();
    }
}
```
对于lines，如果使map则会形成一个流，这个流中有多个数组，不便于之后的操作；但是使用flatMap函数，先将数组变成多个流，flatMap的功能则是将一个流转成多个流映射，而map则是一个流映射成一个流。
