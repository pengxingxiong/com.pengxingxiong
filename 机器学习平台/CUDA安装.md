基本命令

命令行搜索集显和独显

```shell
lspci | grep VGA     # 查看集成显卡
lspci | grep NVIDIA  # 查看NVIDIA显卡
```



查看显卡情况

```shell
nvidia-smi
```

NVIDIA提供了一个切换显卡的命令：

```shell
sudo prime-select nvidia # 切换nvidia显卡
sudo prime-select intel  # 切换intel显卡
sudo prime-select query  # 查看当前使用的显卡
```

驱动状态

```shell
systemctl status nvidia-persistenced
systemctl enable nvidia-persistenced
```

查看安装列表

```shell
cat /var/lib/apt/lists/*cuda*Packages | grep "Package:"
```

# Python测试

安装相关依赖库

```python
pip3 install nvidia-ml-py3
```

测试代码：

```python
from pynvml import *
nvmlInit()
#显卡驱动版本
print("Driver Version:", nvmlSystemGetDriverVersion())
#几块显卡
deviceCount = nvmlDeviceGetCount()
 #具体是什么显卡
for i in range(deviceCount):
  handle = nvmlDeviceGetHandleByIndex(i)
  print("Device", i, ":", nvmlDeviceGetName(handle))
nvmlShutdown()
```

# tensorflow源码编译

```shell
pip3 install -U --user pip six numpy wheel mock
pip3 install -U --user keras_applications --no-deps
pip3 install -U --user keras_preprocessing --no-deps
```

## 升级pip后出现ImportError: cannot import name main

在Ubuntu中，升级了pip,再次使用pip 安装相关的python包的时候就出现以下错误

ImportError: cannot import name main
解决：pip文件在usr/bin目录下，cd进去，进行以下修改

把下面的三行

```python
from pip import main
if name == 'main':
    sys.exit(main())
```

换成下面的三行

```python
from pip import main
if name == 'main':
    sys.exit(__main__._main())
```

然后问题就解决了。

配置记录

```shell
root@rgibns1:/home/pengxx/Tensorflow/tensorflow# ./configure
WARNING: --batch mode is deprecated. Please instead explicitly shut down your Bazel server using the command "bazel shutdown".
You have bazel 0.26.1 installed.
Please specify the location of python. [Default is /usr/bin/python]: /usr/bin/python3.5


Found possible Python library paths:
  /usr/local/lib/python3.5/dist-packages
  /usr/lib/python3/dist-packages
Please input the desired Python library path to use.  Default is [/usr/local/lib/python3.5/dist-packages]

Do you wish to build TensorFlow with XLA JIT support? [Y/n]: N
No XLA JIT support will be enabled for TensorFlow.

Do you wish to build TensorFlow with OpenCL SYCL support? [y/N]: N
No OpenCL SYCL support will be enabled for TensorFlow.

Do you wish to build TensorFlow with ROCm support? [y/N]: B
Invalid selection: B
Do you wish to build TensorFlow with ROCm support? [y/N]: N
No ROCm support will be enabled for TensorFlow.

Do you wish to build TensorFlow with CUDA support? [y/N]: y
CUDA support will be enabled for TensorFlow.

Do you wish to build TensorFlow with TensorRT support? [y/N]: N
No TensorRT support will be enabled for TensorFlow.

Found CUDA 10.1 in:
    /usr/local/cuda/lib64
    /usr/local/cuda/include
Found cuDNN 7 in:
    /usr/lib/x86_64-linux-gnu
    /usr/local/cuda/include


Please specify a list of comma-separated CUDA compute capabilities you want to build with.
You can find the compute capability of your device at: https://developer.nvidia.com/cuda-gpus.
Please note that each additional compute capability significantly increases your build time and binary size, and that TensorFlow only supports compute capabilities >= 3.5 [Default is: 7.5]: 


Do you want to use clang as CUDA compiler? [y/N]: y    
Clang will be used as CUDA compiler.

Do you wish to download a fresh release of clang? (Experimental) [y/N]: y
Clang will be downloaded and used to compile tensorflow.

Do you wish to build TensorFlow with MPI support? [y/N]: N
No MPI support will be enabled for TensorFlow.

Please specify optimization flags to use during compilation when bazel option "--config=opt" is specified [Default is -march=native -Wno-sign-compare]: 


Would you like to interactively configure ./WORKSPACE for Android builds? [y/N]: 
Not configuring the WORKSPACE for Android builds.

Preconfigured Bazel build configs. You can use any of the below by adding "--config=<>" to your build command. See .bazelrc for more details.
	--config=mkl         	# Build with MKL support.
	--config=monolithic  	# Config for mostly static monolithic build.
	--config=gdr         	# Build with GDR support.
	--config=verbs       	# Build with libverbs support.
	--config=ngraph      	# Build with Intel nGraph support.
	--config=numa        	# Build with NUMA support.
	--config=dynamic_kernels	# (Experimental) Build kernels into separate shared objects.
Preconfigured Bazel build configs to DISABLE default on features:
	--config=noaws       	# Disable AWS S3 filesystem support.
	--config=nogcp       	# Disable GCP support.
	--config=nohdfs      	# Disable HDFS support.
	--config=noignite    	# Disable Apache Ignite support.
	--config=nokafka     	# Disable Apache Kafka support.
	--config=nonccl      	# Disable NVIDIA NCCL support.
Configuration finished
root@rgibns1:/home/pengxx/Tensorflow/tensorflow# 

```

# 容器版本

容器命令

```shell
nvidia-docker run -it --rm --name tf-gpu-pengxx tensorflow/tensorflow:1.8.0-devel-gpu-py3
```

使用挂载

```shell
nvidia-docker run -it -v /home/pengxx:/home/pengxx --rm --name tf-gpu-pengxx tensorflow/tensorflow:1.8.0-devel-gpu-py3
```

