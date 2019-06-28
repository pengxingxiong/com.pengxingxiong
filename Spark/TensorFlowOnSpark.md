https://github.com/yahoo/TensorFlowOnSpark/issues/248

https://github.com/yahoo/TensorFlowOnSpark/issues/121





运行命令：

```shell
export LIB_HDFS=$HADOOP_HOME/lib/native
export LIB_JVM=$JAVA_HOME/jre/lib/amd64/server

${SPARK_HOME}/bin/spark-submit \
--master spark://172.18.135.131:7077 \
--executor-memory 2G \
--driver-memory 2G \
--driver-cores 2 \
--num-executors 2 \
--conf spark.executorEnv.LD_LIBRARY_PATH=$LIB_JVM:$LIB_HDFS \
--conf spark.cores.max=6 \
--executor-cores 1 \
--py-files /home/pengxx/TensorFlowOnSpark/examples/mnist/spark/mnist_dist.py \
/home/pengxx/TensorFlowOnSpark/examples/mnist/spark/mnist_spark.py \
--cluster_size 3 \
--images examples/mnist/csv/train/images \
--labels examples/mnist/csv/train/labels \
--format csv \
--mode train \
--model mnist_model
```



任务是：

```shell
ll /opt/spark-2.3.1-bin-hadoop2.7/work/app-20190528221822-0005/
```

131环境不存在该文件夹

132环境存在0/1/2个executor文件夹，但是只有0和2文件夹中有executor_id。

132环境存在3/4/5个executor文件夹，但是只有5文件夹中有executor_id。

以下方式能够运行。

```shell
--conf spark.cores.max=3 \
--executor-cores 1 \
--cluster_size 3 \
--num_ps 2 \
```

| spark.cores.max | executor-cores | cluster_size | num_ps | run  | Time |
| --------------- | -------------- | ------------ | ------ | ---- | ---- |
| 3               | 1              | 3            | 1      | F    | 10m  |
| 3               | 1              | 3            | 2      | T    | 48s  |
| 4               | 1              | 4            | 2      | F    | 20m  |
| 4               | 1              | 4            | 3      | T    | 58s  |
| 5               | 1              | 5            | 3      | F    | -    |

从表中各参数对比来看，总共N个executor，那么参数服务器占用N-1个。这个可能和手写体识别算法的参数数量有关，下面是可能的原因。

The bottleneck in distributed training with lots of parameters is often the network bandwidth. If you saturate the network too much, packets get lost and TensorFlow thinks the parameter server is down. By adding more parameter servers you are able to distribute the network load.

分布式训练中参数较多的瓶颈往往是网络带宽。如果网络饱和太多，数据包会丢失，TensorFlow认为参数服务器已关闭。通过添加更多的参数服务器，您可以分配网络负载。

然而从自己编写的随机森林试验中，却可以发现参数服务器可以仅设置一个。

```python
#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
@author:pengxingxiong@ruijie.com.cn
@time: 2019/5/27 17:57 
@desc:
"""
import os
from datetime import datetime

from pyspark import SparkConf
from pyspark.ml.feature import VectorAssembler, StringIndexer
from pyspark.sql import SparkSession
from tensorflow.contrib.tensor_forest.python import tensor_forest
from tensorflowonspark.pipeline import TFEstimator
import tensorflow as tf
from hdfs import Client


class SparkPipeline:
    num_executors = 1
    hadoop_address = "http://172.18.135.131:50070"
    hdfs_address = "hdfs://172.18.135.131:9000"
    hdfs_project_workspace = "/user/spark/tf_randomforest/"
    data_path = hdfs_address + hdfs_project_workspace + "data_per200.csv"
    args_train = True
    args_model_dir = hdfs_address + hdfs_project_workspace + "model"
    args_export_dir = hdfs_address + hdfs_project_workspace + "export"
    args_cluster_size = 3
    args_num_ps = 1
    args_protocol = "grpc"
    args_epochs = 1
    args_batch_size = 100
    args_steps = 50
    args_tensorboard = None
    args_output = hdfs_address + hdfs_project_workspace + "output"
    args_inference_output = 'predictions'

    def get_data(self):
        spark_config = SparkConf()
        spark_config.setAppName("randomForest")
        spark = SparkSession.builder.config(conf=spark_config).getOrCreate()
        spark.sparkContext.setLogLevel("WARN")
        images = spark.read.csv(self.data_path, header=True)
        print("{0} ===== Start".format(datetime.now().isoformat()))
        features = images
        # 提取特征列的名称（最后一列是标签列）
        colNames = images.columns[:-1]
        for colName in colNames:
            features = features.withColumn(colName, features[colName].cast("float"))

        # features.show(10, False)
        labelIndexer = StringIndexer().setInputCol("device_category").setOutputCol("label")
        output0 = labelIndexer.fit(features).transform(features).drop("device_category")
        # output0.show(10, False)
        assembler = VectorAssembler().setInputCols(colNames).setOutputCol("features")
        output = assembler.transform(output0)
        # output.show(10, False)
        return output.select("features", "label")

    def train(self):
        data = self.get_data()
        print("{0} ===== Estimator.fit()".format(datetime.now().isoformat()))

        tf_args = {'initial_learning_rate': 0.045, 'num_epochs_per_decay': 2.0, 'learning_rate_decay_factor': 0.94}
        self.init_hdfs()
        """
        1.其中的InputMapping配置要和model.setInputMapping对应
        2.另外参数服务器的数量可以少于worker数，但是注意的是每个spark的executor只能使用一个CPU核（--executor-cores 1）
        而最大使用cpu数量(spark.cores.max)必须和cluster_size大小一致。
        3. 运行前，删除hdfs上的export和output目录
        """
        estimator = TFEstimator(self.map_fun, tf_args) \
            .setInputMapping({'features': 'features', 'label': 'label'}) \
            .setModelDir(self.args_model_dir) \
            .setExportDir(self.args_export_dir) \
            .setClusterSize(self.args_cluster_size) \
            .setNumPS(self.args_num_ps) \
            .setProtocol(self.args_protocol) \
            .setTensorboard(self.args_tensorboard) \
            .setEpochs(self.args_epochs) \
            .setBatchSize(self.args_batch_size) \
            .setSteps(self.args_steps)
        model = estimator.fit(data)

        model.setModelDir(None)
        model.setExportDir(self.args_export_dir)
        # 创建模型的标记，用于重新加载模型时的检索
        model.setTagSet(tf.saved_model.tag_constants.SERVING)
        # 创建模型签名，用于唯一标识模型
        model.setSignatureDefKey(
            tf.saved_model.signature_constants.DEFAULT_SERVING_SIGNATURE_DEF_KEY)
        # 将数据流图中操作名为“features”和“label”的输出作为模型的输入
        model.setInputMapping({'features': 'features', 'label': 'label'})
        # 将数据流图中的操作名为“prediction”的输出作为模型的输出
        model.setOutputMapping({'prediction': 'prediction'})

        print("{0} ===== Model.transform()".format(datetime.now().isoformat()))
        preds = model.transform(data)
        preds.write.json(self.args_output)

        print("{0} ===== Stop".format(datetime.now().isoformat()))

    def map_fun(self, args, ctx):

        from tensorflowonspark import TFNode

        from datetime import datetime

        import numpy

        import time
        start = time.time()
        worker_num = ctx.worker_num
        job_name = ctx.job_name
        task_index = ctx.task_index
        # TODO 1. Parameters
        num_classes = 11
        num_features = 298
        num_trees = 5
        max_nodes = 1000

        # Delay PS nodes a bit, since workers seem to reserve GPUs more quickly/reliably (w/o conflict)
        if job_name == "ps":
            time.sleep((worker_num + 1) * 5)

        # Parameters
        batch_size = args.batch_size

        # Get TF cluster and server instances
        cluster, server = TFNode.start_cluster_server(ctx, 1, args.protocol == 'rdma')

        def feed_dict(batch):
            # DataFrame转为tensor张量数据
            # print("当前批次的数据中包含的键为："batch.keys())
            features = batch['features']
            labels = batch['label']
            xs = numpy.array(features)
            xs = xs.astype(numpy.float32)
            xs = xs / 255.0
            ys = numpy.array(labels)
            ys = ys.astype(numpy.uint8)
            return xs, ys

        # 在worker中执行算法
        if job_name == "ps":
            server.join()
        elif job_name == "worker":

            # 将操作安排到worker节点，在不同的worker上进行图拷贝
            with tf.device(
                    tf.train.replica_device_setter(worker_device="/job:worker/task:%d" % task_index, cluster=cluster)):
                # 使用占位符先填充x和y_，便于在后面取得数据后的计算
                x = tf.placeholder(tf.float32, [None, num_features], name="x")
                y_ = tf.placeholder(tf.uint8, [None], name="y_")  # n行num_classes列
                # 将标签数据转置为一列，用于和特征进行矩阵计算
                # y_ = tf.reshape(y_, [1, None])
                # 构造随机森林参数对象
                hparams = tensor_forest.ForestHParams(num_classes=num_classes, num_features=num_features,
                                                      num_trees=num_trees, max_nodes=max_nodes).fill()
                # 创建随机森林的基本数据流图
                forest_graph = tensor_forest.RandomForestGraphs(hparams)
                # 获取训练器
                train_op = forest_graph.training_graph(x, y_)
                # 获取损失函数计算器
                loss_op = forest_graph.training_loss(x, y_)
                # 记录当前迭代次数
                global_step = tf.Variable(0, trainable=False)
                # 测试
                # 从特征中获取其预测的值
                infer_op, _, _ = forest_graph.inference_graph(x)
                prediction = tf.argmax(infer_op, 1, name="prediction")
                label = tf.cast(y_, tf.int64, name="label")
                # 判定每行的预测值和标签值是否相等
                # correct_prediction = tf.equal(tf.argmax(infer_op, 1), tf.cast(y_, tf.int64))
                correct_prediction = tf.equal(label, prediction)
                # 计算准确率
                accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32), name="accuracy")
                tf.summary.scalar("acc", accuracy)
                # 算法信息保存
                saver = tf.train.Saver()

                summary_op = tf.summary.merge_all()

                # 初始化全局变量操作
                init_op = tf.global_variables_initializer()

            # 从hdfs上获取模型文件存储位置
            logdir = TFNode.hdfs_path(ctx, args.model_dir)
            print("tensorflow model path: {0}".format(logdir))
            summary_writer = tf.summary.FileWriter("tensorboard_%d" % worker_num, graph=tf.get_default_graph())
            sv = tf.train.Supervisor(is_chief=(task_index == 0), logdir=logdir, init_op=init_op, summary_op=None,
                                     saver=saver, global_step=global_step, stop_grace_secs=300, save_model_secs=10)

            # 在每个会话管理器中处理：拉取数据，将上面创建的train_op，summary_op放入迭代中，使用with是为了安全关闭
            with sv.managed_session(server.target) as sess:
                # 如果需要打印数据流图，用于tensorboard展示，可以开启此处代码
                # writer = tf.summary.FileWriter(self.hdfs_path + "my_graph")
                # writer.add_graph(sess.graph)

                print("{0} session ready".format(datetime.now().isoformat()))
                # 循环，直到监督器停止或者循环了1百万次
                step = 0
                # 创建抓取数据的对象
                tf_feed = TFNode.DataFeed(ctx.mgr, input_mapping=args.input_mapping)
                predicts_list = []
                while not sv.should_stop() and not tf_feed.should_stop() and step < args.steps:
                    # 抓取特征数据和标签数据，放入到一个dict中
                    batch_xs, batch_ys = feed_dict(tf_feed.next_batch(batch_size))
                    feed = {x: batch_xs, y_: batch_ys}
                    # print("######### feed=", feed)
                    if len(batch_xs) > 0:
                        # 训练模型
                        _, _, summary, predicts, step = sess.run(
                            [train_op, loss_op, summary_op, prediction, global_step],
                            feed_dict=feed)
                        # 每一百次迭代，打印一次准确率以及保存checkpoint到HDFS
                        # print("######### predicts=", predicts)
                        predicts_list.append(predicts)
                        if step % 100 == 0:
                            print("{0} step: {1} accuracy: {2}"
                                  .format(datetime.now().isoformat(), step, sess.run(accuracy, feed_dict=feed)))
                        if sv.is_chief:
                            summary_writer.add_summary(summary, step)
                        # graph = tf.get_default_graph()
                        # for op in graph.get_operations():
                        #     print("##### optionsName= ", op.name)
                # 收敛
                if sv.should_stop() or step >= self.args_steps:
                    tf_feed.terminate()
                if sv.is_chief and args.export_dir:
                    ckpt = tf.train.get_checkpoint_state(self.args_model_dir)
                    print("ckpt: {}".format(ckpt))
                    assert ckpt, "Invalid model checkpoint path: {}".format(self.args_model_dir)
                    saver.restore(sess, ckpt.model_checkpoint_path)
                    print("{0} exporting saved_model to: {1}".format(datetime.now().isoformat(), args.export_dir))
                    # exported signatures defined in code
                    print("######### prediction=", predicts_list)
                    signatures = {
                        tf.saved_model.signature_constants.DEFAULT_SERVING_SIGNATURE_DEF_KEY: {
                            'inputs': {'features': x, 'label': y_},
                            'outputs': {'prediction': prediction},
                            'method_name': tf.saved_model.signature_constants.PREDICT_METHOD_NAME
                        }
                    }
                    TFNode.export_saved_model(sess,
                                              args.export_dir,
                                              tf.saved_model.tag_constants.SERVING,
                                              signatures)
                else:
                    # non-chief workers should wait for chief
                    while not sv.should_stop():
                        print("Waiting for chief")
                        time.sleep(5)
            # Ask for all the services to stop.
            print("{0} stopping supervisor".format(datetime.now().isoformat()))
            end = time.time()
            print("time: ", end - start)
            sv.stop()

    def init_hdfs(self):
        """
        初始化hdfs目录，主要是用于防止多次运行时目录已存在等问题，主要删除export和output目录，另外model目录存在大量的checkpoint，这个酌情考虑是否删除
        请注意权限问题：如果是在服务器上运行，则不需要配置；而如果是远程调试，请开发目录权限。hadoop fs -chmod -R 777 /user/spark/tf_randomforest
        """
        # os.environ["HADOOP_USER_NAME"] = "root"
        os.system("hadoop fs -chmod -R 777 /user/spark/tf_randomforest")
        client = Client(self.hadoop_address, root="/", timeout=100, session=False)
        # client.set_permission(self.hdfs_project_workspace, 777)
        client.delete(self.hdfs_project_workspace + "export", recursive=True)
        client.delete(self.hdfs_project_workspace + "output", recursive=True)
        # hdfs = HDFileSystem(host="hdfs://172.18.135.131", port=9000)
        # print(hdfs.ls('/user'))


if __name__ == "__main__":
    pipeline = SparkPipeline()
    pipeline.train()
    # pipeline.init_hdfs()

```

# GPU测试

该配置方法主要是TFNode.start_cluster_server函数。其中可以配置GPU数量。

切换tensorflow版本

```shell
pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple tensorflow-gpu
```

然后运行tensorflow代码，会报如下错误：

ImportError: libcublas.so.10.0: cannot open shared object file: No such file or directory

需要服务器支持GPU，并具备编译库。（如果不具备GPU，可以卸载该版本，并重新安装CPU版本）：

```shell
pip3 uninstall tensorflow-gpu -y
pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple tensorflow
```

# 提交方式

## yarn

使用如下命令

```shell
/home/pengxx/spark-2.4.3-bin-hadoop2.7/bin/spark-submit \
--master yarn \
--executor-memory 2G \
--jars hdfs:///user/root/tensorflow-hadoop-1.10.0.jar \
--driver-library-path=/usr/local/cuda-10.0/lib64 \
--conf spark.executorEnv.LD_LIBRARY_PATH=$JAVA_HOME/jre/lib/amd64/server:$HADOOP_HOME/lib/native:/usr/local/cuda-10.0/lib64 \
--conf spark.executorEnv.CLASSPATH=$(hadoop classpath --glob) \
--conf spark.cores.max=1 \
--conf spark.executor.cores=1 \
/home/pengxx/Tensorflow/spark_pipeline.py
```

在提交之前需要上传tensorflow-hadoop-1.10.0.jar到hdfs。

# FAQ

抓取数据超时问题

```shell
Exception: Timeout while feeding partition
```

需要加入相关的spark配置

```shell
--conf spark.executorEnv.LD_LIBRARY_PATH=$JAVA_HOME/jre/lib/amd64/server:$HADOOP_HOME/lib/native:/usr/local/cuda-10.0/lib64 \
--conf spark.executorEnv.CLASSPATH=$(hadoop classpath --glob) \
```



## 资源找不到

```shell
NotFoundError (see above for traceback): Resource worker/tree-3//N10tensorflow12tensorforest20DecisionTreeResourceE does not exist.
         [[node tree-3/TreeSerialize (defined at /home/pengxx/Tensorflow/spark_pipeline.py:168) ]]
```

从官方的BUG讨论来看，有一个博主说，修改了tensor_flow的示例代码，这里进行对比发现有下面的不同之处：

spark_pipeline.py中使用了如下代码

```python
init_op = tf.global_variables_initializer()
```

而那个博主使用的是如下代码

```python
init_op = tf.group(tf.global_variables_initializer(),
                   resources.initialize_resources(resources.shared_resources()))
```

按照该博主的方法修改后运行，发现错误变为：

```shell
ValueError: Cannot feed value of shape (100, 297) for Tensor 'x:0', which has shape '(?, 298)'
```

这时就能够明确，原来是自己的spark_pipeline.py代码中处理数据的逻辑错误导致，修改占位函数（`tf.placeholder`）中的特征数量后能够正常运行。

参考：

> 1. [Random Forest error for tf 1.4 "Resource localhost/tree-1//N10tensorflow12tensorforest20DecisionTreeResourceE does not exist."](https://github.com/aymericdamien/TensorFlow-Examples/issues/208)
>
> 2. [aymericdamien/TensorFlow-Examples](aymericdamien/TensorFlow-Examples)

##资源等待

```shell
2019-06-12 14:35:56,401 INFO (MainThread-45354) Waiting for TFSparkNodes to start
2019-06-12 14:35:56,401 INFO (MainThread-45354) waiting for 1 reservations
2019-06-12 14:35:57,403 INFO (MainThread-45354) waiting for 1 reservations
```

可能是cpu分配不足，或者gpu不足。

#### cpu不足

从TensorFlowOnSpark官方说明来看，目前spark配置中spark.executor.cores仅能够设置为1。而spark.cores.max的数量则要和TFEstimator类的ClusterSize值相等。例如下面的配置方式：

```shell
${SPARK_HOME}/bin/spark-submit \
--master spark://172.18.130.34:7077 \
--executor-memory 2G \
--conf spark.executorEnv.LD_LIBRARY_PATH=$JAVA_HOME/jre/lib/amd64/server:$HADOOP_HOME/lib/native:/usr/local/cuda-10.0/lib64 \
--conf spark.executorEnv.CLASSPATH=$(hadoop classpath --glob) \
--conf spark.cores.max=4 \
--executor-cores 1 \
--py-files /home/pengxx/Tensorflow/mnist_dist.py \
/home/pengxx/Tensorflow/mnist_spark.py \
--cluster_size 4 \
--num_ps 1 \
--images examples/mnist/csv/train/images \
--labels examples/mnist/csv/train/labels \
--format csv \
--mode train \
--model mnist_model \
--rdma true
```

#### gpu不足

这种问题常常不太明显能够看出，可能系统日志一直持续打印`aiting for 1 reservations`类似的信息，而有时候也会打印`Unable to find available GPUs: requested=1, available=0`类似的信息。因此为了应对这种不太确定的情况，从源码跟踪来看，这个gpu调用主要体现在tensorflowonspark.gpu_info.py文件中的get_gpus()函数。通过层层往上追溯，能够发现tensorflowonspark.TFNode.py文件中的start_cluster_server()函数和tensorflowonspark.TFSparkNode.py文件中的run()函数都使用了get_gpus()函数，代码为：

```python
# tensorflowonspark.TFNode.py
def start_cluster_server(ctx, num_gpus=1, rdma=False)
# tensorflowonspark.TFSparkNode.py
def run(fn, tf_args, cluster_meta, tensorboard, log_dir, queues, background)
```

start_cluster_server()函数用于map_fun()中，例如在手写体识别的代码中就有如下：

```python
cluster, server = ctx.start_cluster_server(1, args.rdma)
```

可以明显的发现第一个参数值为1，就是num_gpus=1，表示在当前worker中默认使用一个gpu。如果使用的是tensorflow库，则不会出现问题，而如果使用的是tensorflow-gpu库，则注意下这个地方。

run()函数的参数中虽然没有出现num_gpus的配置，但是num_gpus是从tf_args中取得，因此最好能够在tf_args中配置num_gpus的值。

> ps:因为考虑到在pyspark编程中DataFrame性能比RDD高，因此在我的代码中主要参考的是mnist_spark_pipeline.py的方式。

## 找不到GPU

错误内容为：

```shell
2019-06-12 21:13:07,644 INFO (MainThread-25484) Feeding partition <itertools.chain object at 0x7f78406687f0> into input queue <multiprocessing.queues.JoinableQueue object at 0x7f78404dc8d0>
2019-06-12 21:13:13,799 INFO (MainThread-25428) Available GPUs: ['0']
2019-06-12 21:13:13,800 INFO (MainThread-25428) Proposed GPUs: ['0']
2019-06-12 21:13:13,800 INFO (MainThread-25428) 0: Using GPU: 0
No server factory registered for the given ServerDef: cluster {
  job {
    name: "worker"
    tasks {
      key: 0
      value: "172.18.130.34:39414"
    }
  }
}
job_name: "worker"
protocol: "grpc+verbs"

The available server factories are: [ GRPC_SERVER ]
2019-06-12 21:13:13,801 ERROR (MainThread-25428) 0: Failed to allocate GPU, trying again...
2019-06-12 21:13:23,865 INFO (MainThread-25428) Available GPUs: ['0']
2019-06-12 21:13:23,865 INFO (MainThread-25428) Proposed GPUs: ['0']
2019-06-12 21:13:23,866 INFO (MainThread-25428) 0: Using GPU: 0
No server factory registered for the given ServerDef: cluster {
```

没有为ServerDef注册的服务工厂。该问题参考[Tensorflow failed to use RDMA on Infiniband network](https://stackoverflow.com/questions/49427448/tensorflow-failed-to-use-rdma-on-infiniband-network)，可以找到解决方案

该问答中指出要想将GPU搭配RDMA使用，则是要在本地编译TensorFlow，而不是使用官方提供的库。因此，这里将RDMA关闭，改用GRPC协议，可以解决该问题。

但是该问题解决后出现新的问题：

```shell
2019-06-12 21:24:48,447 INFO (MainThread-34883) Available GPUs: ['0']
2019-06-12 21:24:48,447 INFO (MainThread-34883) Proposed GPUs: ['0']
2019-06-12 21:24:48,447 INFO (MainThread-34883) Using gpu(s): 0
2019-06-12 21:24:48,448 INFO (MainThread-34883) ===== input_mapping: {'features': 'features', 'label': 'label'}
2019-06-12 21:24:48,448 INFO (MainThread-34883) ===== output_mapping: {'prediction': 'prediction'}
2019-06-12 21:24:48,448 INFO (MainThread-34883) ===== loading meta_graph_def for tag_set (serve) from saved_model: hdfs://172.18.130.34:9000/user/spark/tf_randomforest/export
2019-06-12 21:24:50.630676: I tensorflow/compiler/xla/service/service.cc:150] XLA service 0xe2c23e0 executing computations on platform CUDA. Devices:
2019-06-12 21:24:50.630723: I tensorflow/compiler/xla/service/service.cc:158]   StreamExecutor device (0): Tesla T4, Compute Capability 7.5
2019-06-12 21:24:50.634236: I tensorflow/core/platform/profile_utils/cpu_utils.cc:94] CPU Frequency: 2200180000 Hz
2019-06-12 21:24:50.640729: I tensorflow/compiler/xla/service/service.cc:150] XLA service 0xe3f81a0 executing computations on platform Host. Devices:
2019-06-12 21:24:50.640767: I tensorflow/compiler/xla/service/service.cc:158]   StreamExecutor device (0): , 
2019-06-12 21:24:50.641325: I tensorflow/core/common_runtime/gpu/gpu_device.cc:1433] Found device 0 with properties: 
name: Tesla T4 major: 7 minor: 5 memoryClockRate(GHz): 1.59
pciBusID: 0000:84:00.0
totalMemory: 14.73GiB freeMemory: 14.60GiB
2019-06-12 21:24:50.641359: I tensorflow/core/common_runtime/gpu/gpu_device.cc:1512] Adding visible gpu devices: 0
2019-06-12 21:24:50.642575: I tensorflow/core/common_runtime/gpu/gpu_device.cc:984] Device interconnect StreamExecutor with strength 1 edge matrix:
2019-06-12 21:24:50.642604: I tensorflow/core/common_runtime/gpu/gpu_device.cc:990]      0 
2019-06-12 21:24:50.642616: I tensorflow/core/common_runtime/gpu/gpu_device.cc:1003] 0:   N 
2019-06-12 21:24:50.643090: I tensorflow/core/common_runtime/gpu/gpu_device.cc:1115] Created TensorFlow device (/job:localhost/replica:0/task:0/device:GPU:0 with 14202 MB memory) -> physical GPU (device: 0, name: Tesla T4, pci bus id: 0000:84:00.0, compute capability: 7.5)
2019-06-12 21:24:50,650 INFO (MainThread-34883) ===== restoring from saved_model: hdfs://172.18.130.34:9000/user/spark/tf_randomforest/export
2019-06-12 21:24:50,650 WARNING (MainThread-34883) From /usr/local/lib/python3.5/dist-packages/tensorflowonspark/pipeline.py:527: load (from tensorflow.python.saved_model.loader_impl) is deprecated and will be removed in a future version.
Instructions for updating:
This function will only be available through the v1 compatibility library as tf.compat.v1.saved_model.loader.load or tf.compat.v1.saved_model.load. There will be a new function for importing SavedModels in Tensorflow 2.0.
2019-06-12 21:24:50,780 WARNING (MainThread-34883) From /usr/local/lib/python3.5/dist-packages/tensorflow/python/training/saver.py:1266: checkpoint_exists (from tensorflow.python.training.checkpoint_management) is deprecated and will be removed in a future version.
Instructions for updating:
Use standard file APIs to check for files with this prefix.
2019-06-12 21:24:50,858 INFO (MainThread-34883) Restoring parameters from hdfs://172.18.130.34:9000/user/spark/tf_randomforest/export/variables/variables
2019-06-12 21:24:51,808 INFO (MainThread-34883) input_tensors: ['x:0', 'y_:0']
2019-06-12 21:24:51,809 INFO (MainThread-34883) output_tensors: ['prediction:0']
2019-06-12 21:25:16,424 INFO python.PythonRunner: Times: total = 31215, boot = 5, init = 3190, finish = 28020
2019-06-12 21:25:16,427 INFO executor.Executor: Finished task 0.0 in stage 6.0 (TID 6). 2193 bytes result sent to driver
2019-06-12 21:25:16,736 INFO executor.CoarseGrainedExecutorBackend: Got assigned task 7
2019-06-12 21:25:16,737 INFO executor.Executor: Running task 0.0 in stage 7.0 (TID 7)
2019-06-12 21:25:16,740 INFO broadcast.TorrentBroadcast: Started reading broadcast variable 12
2019-06-12 21:25:16,748 INFO memory.MemoryStore: Block broadcast_12_piece0 stored as bytes in memory (estimated size 112.7 KB, free 910.9 MB)
2019-06-12 21:25:16,750 INFO broadcast.TorrentBroadcast: Reading broadcast variable 12 took 10 ms
2019-06-12 21:25:16,752 INFO memory.MemoryStore: Block broadcast_12 stored as values in memory (estimated size 354.0 KB, free 910.6 MB)
2019-06-12 21:25:16,923 INFO datasources.FileScanRDD: Reading File path: hdfs://172.18.130.34:9000/user/spark/tf_randomforest/data_per200.csv, range: 0-2580929, partition values: [empty row]
2019-06-12 21:25:16,943 INFO codegen.CodeGenerator: Code generated in 12.734257 ms
2019-06-12 21:25:16,955 INFO output.FileOutputCommitter: File Output Committer Algorithm version is 1
2019-06-12 21:25:16,957 INFO datasources.SQLHadoopMapReduceCommitProtocol: Using output committer class org.apache.hadoop.mapreduce.lib.output.FileOutputCommitter
2019-06-12 21:25:17,100 WARNING (MainThread-34883) Unable to find available GPUs: requested=1, available=0
2019-06-12 21:25:47,171 WARNING (MainThread-34883) Unable to find available GPUs: requested=1, available=0
2019-06-12 21:26:47,277 WARNING (MainThread-34883) Unable to find available GPUs: requested=1, available=0
2019-06-12 21:28:17,366 INFO (MainThread-34883) Available GPUs: []
2019-06-12 21:28:17,443 INFO (MainThread-34883) : gpu_uuid, pid, process_name, used_gpu_memory [MiB]
GPU-7a985a8f-1d79-5654-3f77-8b2c17e7fee2, 34883, python3, 14427 MiB

2019-06-12 21:28:17,468 ERROR util.Utils: Aborting task
org.apache.spark.api.python.PythonException: Traceback (most recent call last):
  File "/opt/spark/python/lib/pyspark.zip/pyspark/worker.py", line 377, in main
    process()
  File "/opt/spark/python/lib/pyspark.zip/pyspark/worker.py", line 372, in process
    serializer.dump_stream(func(split_index, iterator), outfile)
  File "/opt/spark/python/lib/pyspark.zip/pyspark/rdd.py", line 2499, in pipeline_func
  File "/opt/spark/python/lib/pyspark.zip/pyspark/rdd.py", line 2499, in pipeline_func
  File "/opt/spark/python/lib/pyspark.zip/pyspark/rdd.py", line 2499, in pipeline_func
  File "/opt/spark/python/lib/pyspark.zip/pyspark/rdd.py", line 2499, in pipeline_func
  File "/opt/spark/python/lib/pyspark.zip/pyspark/rdd.py", line 352, in func
  File "/usr/local/lib/python3.5/dist-packages/tensorflowonspark/pipeline.py", line 471, in 
    rdd_out = dataset.select(input_cols).rdd.mapPartitions(lambda it: _run_model(it, local_args, tf_args))
  File "/usr/local/lib/python3.5/dist-packages/tensorflowonspark/pipeline.py", line 494, in _run_model
    single_node_env(tf_args)
  File "/usr/local/lib/python3.5/dist-packages/tensorflowonspark/pipeline.py", line 581, in single_node_env
    util.single_node_env(num_gpus)
  File "/usr/local/lib/python3.5/dist-packages/tensorflowonspark/util.py", line 32, in single_node_env
    gpus_to_use = gpu_info.get_gpus(num_gpus)
  File "/usr/local/lib/python3.5/dist-packages/tensorflowonspark/gpu_info.py", line 89, in get_gpus
    raise Exception("Unable to find {} free GPU(s)\n{}".format(num_gpu, smi_output))
Exception: Unable to find 1 free GPU(s)
```



## 其他

###使用不同的文件系统

主要是以下几种方式：

- 以`hdfs://`开头的HDFS文件，如`hdfs:/user/spark/tf_randomforest/data.csv`
- 以`file://`开头的单机本地文件系统，如`file:///home/spark/tf_randomforest/data.csv`，需要注意的是这里的三个`\\\`。
- 其他方案还待调研。

