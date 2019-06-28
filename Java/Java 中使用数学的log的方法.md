今天用存储过程向数据库插入了500W的数据，耗时N久，于是就想知道二分查找1000万的效率，但忘了怎么计算.....

Java 的Math.log(double a) 函数是以e(2.71828...)为底来计算的，如果我们想知道log(2)(1000万)的计算结果，应该怎么做呢？

利用换底公式：log(x)(y) =log(e)(x) / log(e)(y)，我们可以这样做：Math.log(1000*10000) / Math.log(2)。

为了方便口算，贴一下log其他公式(其中a^b代表  a的b次幂)：

1. a^(log(a)(b))=b 
2. log(a)(a^b)=b
3. log(a)(MN)=log(a)(M)+log(a)(N); 
4. log(a)(M÷N)=log(a)(M)-log(a)(N); 
5. log(a)(M^n)=nlog(a)(M) 

