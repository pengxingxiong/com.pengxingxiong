# tf.expand_dims()使用

```python
tf.expand_dims(
    input,
    axis=None,
    name=None,
    dim=None
)
```

他所实现的功能是给定一个input，在axis轴处给input增加一个为1的维度。 举个栗子：

```python
# 't2' is a tensor of shape [2, 3, 5]
tf.shape(tf.expand_dims(t2, 0))  # [1, 2, 3, 5]
```

因为axis=0所以矩阵的维度变成1*2*3*5。

同理如果axis=2，矩阵就会变为2*3*5*1。

0其实代表的第一维度，那么1代表第二维度，2代表第三维度。增加后该位置的维度为1，以此类推。例如

```python
# 't' is a tensor of shape [2]
shape(expand_dims(t, 0)) ==> [1, 2]
shape(expand_dims(t, 1)) ==> [2, 1]
shape(expand_dims(t, -1)) ==> [2, 1]

# 't2' is a tensor of shape [2, 3, 5]
shape(expand_dims(t2, 0)) ==> [1, 2, 3, 5]
shape(expand_dims(t2, 2)) ==> [2, 3, 1, 5]
shape(expand_dims(t2, 3)) ==> [2, 3, 5, 1]
```

举例说明：

```	python
import tensorflow as tf

a = tf.constant([1, 2, 3, 4, 5, 6], shape=[2, 3])
b = tf.expand_dims(a, 0)
sess = tf.Session(config=tf.ConfigProto(log_device_placement=True))
# 运行这个op.
print(sess.run(a))
print(sess.run(b))
```

输出结果如下：

```shell
[[1 2 3]
 [4 5 6]]

[[[1 2 3]
  [4 5 6]]]
```

可以看到，在什么级别加维度，就是在对应级别加括号。

# tf.concat()详解

tensorflow中用来拼接张量的函数tf.concat()，用法:

```python
tf.concat([tensor1, tensor2, tensor3,...], axis)
```

先给出tf源代码中的解释:

```python
t1 = [[1, 2, 3], [4, 5, 6]]
t2 = [[7, 8, 9], [10, 11, 12]]
tf.concat([t1, t2], 0)  # [[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12]]
tf.concat([t1, t2], 1)  # [[1, 2, 3, 7, 8, 9], [4, 5, 6, 10, 11, 12]]

# tensor t3 with shape [2, 3]
# tensor t4 with shape [2, 3]
tf.shape(tf.concat([t3, t4], 0))  # [4, 3]
tf.shape(tf.concat([t3, t4], 1))  # [2, 6]
```

这里解释了当axis=0和axis=1的情况，怎么理解这个axis呢？其实这和numpy中的**np.concatenate()**用法是一样的。

axis=0     代表在第0个维度拼接

axis=1     代表在第1个维度拼接 

对于一个二维矩阵，第0个维度代表最外层方括号所框下的子集，第1个维度代表内部方括号所框下的子集。维度越高，括号越小。

对于这种情况，我可以再解释清楚一点: 

对于[ [ ], [ ]]和[[ ], [ ]]，低维拼接等于拿掉最外面括号，高维拼接是拿掉里面的括号(保证其他维度不变)。注意：tf.concat()拼接的张量只会改变一个维度，其他维度是保存不变的。比如两个shape为[2,3]的矩阵拼接，要么通过axis=0变成[4,3]，要么通过axis=1变成[2,6]。改变的维度索引对应axis的值。

这样就可以理解多维矩阵的拼接了，可以用axis的设置来从不同维度进行拼接。 

对于三维矩阵的拼接，自然axis取值范围是[0, 1, 2]。

**对于axis等于负数的情况**

负数在数组索引里面表示倒数(countdown)。比如，对于列表ls = [1,2,3]而言，ls[-1] = 3，表示读取倒数第一个索引对应值。

axis=-1表示倒数第一个维度，对于三维矩阵拼接来说，axis=-1等价于axis=2。同理，axis=-2代表倒数第二个维度，对于三维矩阵拼接来说，axis=-2等价于axis=1。

一般在维度非常高的情况下，我们想在最'高'的维度进行拼接，一般就直接用countdown机制，直接axis=-1就搞定了。