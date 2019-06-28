## 安装 matplotlib
进入pyhon的本地安装目录下的Scripts文件夹，shift+鼠标右键进入cmd控制台。执行如下命令
```shell
pip3.6 install matplotlib
```
然后控制台就会自动从网上下载该工具了
## __init__
当用idea编译器等创建python包的时候，就会自动产生一个空的`__init__.py`文件，这个文件可以让它保持着空，它只是用于标识当前文件夹是python的包。

## __name__
例如两个文件
base.py
```python
def print_func( par ):
    print ("Hello : ", par)
    print(__name__)
    return
```
app.py
```python
from test import base
base.print_func("Runoob")
print(__name__)
```
运行app.py文件可以发现打印以下内容
```bash
Hello :  Runoob
test.base
__main__
```
发现对于app.py来说，因为运行的就是它，所以`__name__`默认为`__main__`值，但是app.py引用了base.py，则base.py中打印了`test.base`值，说明`__name__`是一个标识，当它在当前执行文件时表示main，不在当前执行文件则表示当前执行文件引用的模块名。

## ImportError: cannot import name 'base'
例如有个包名为test
两个文件
base.py
```python
def print_func( par ):
    print ("Hello : ", par)
    print(__name__)
    return
```
test.py
```python
from test import base
base.print_func("Runoob")
print(__name__)
```
这个时候就报错了，因为test.py和test包名重复了，就好像是从test.py中找base模块。所以将test.py改名即可。

## main入口函数
由于python是脚本语言，不像c和java那样必须有mian入口函数，所以任何一个.py文件都能够运行。现在有个场景：app.py中有main()函数，但是它无法直接运行，因为是无法单独运行函数。这时候可以加上其他代码即可。
app.py
```python
def print_func( par ):
    print ("Hello : ", par)
    print(__name__)
    return

print_func("word")
```
此时运行app.py时则相当于执行了`print_func("word")`代码。返回结果为：
```bash
Hello :  word
__main__
```
可以看到多了个`__main__`表示当前文件被运行了
也可以在另外一个文件app.py中来执行
```python
from test import base
base.print_func("Runoob")
```
但是结果就有问题了
```bash
Hello :  word
test.base
Hello :  Runoob
test.base
```
发现base模块的main函数被执行了两次，这是因为base模块的`print_func("word")`代码也被执行了。因此需要将base.py修改为以下形式：
```python
def print_func( par ):
    print ("Hello : ", par)
    print(__name__)
    return

if __name__ == "__main__":
    print_func("word")
```
这个时候再运行app.py时就正确了。不仅如此，也能发现idea编译器在`if __name__ == "__main__":`语句的前面产生了运行的按钮。

# pip安装

在无网络、只能使用root权限而无法用root账户的环境中安装pip方案：

主要是使用网络代理，让当前用户能够访问网络：

一般来说在可联网机器上应该配置好了代理账号和密码，例如nubia,nubia123。

则当前用户则直接在命令行加入以下命令即可使得当前用户联网：

```shell
export http_proxy=http://root:nubia@123@10.206.19.180:3128
export https_proxy=http://root:nubia@123@10.206.19.180:3128
# 这里的特殊字符@会导致报错：Could not resolve proxy: 123@10.206.19.180; Unknown error
# 该错误是密码中使用了@，因此需要对密码进行编码为：nubia%40123
export http_proxy=http://root:nubia%40123@10.206.19.180:3128
export https_proxy=http://root:nubia%40123@10.206.19.180:3128
```

然后可以使用curl命令来测试一下网络连接：

```shell
curl "www.baidu.com"
```

能够接受到该网址的html信息表示网络可联通。下面则进行安装pip，在[PyPa官网](https://pip.pypa.io/en/stable/installing/)可以将get-pip.py文件保存到指定位置，然后使用`python get-pip.py`命令即可安装pip，但是由于不是root账户会报错：

```shell
Collecting pip
  Using cached https://files.pythonhosted.org/packages/0f/74/ecd13431bcc456ed390b44c8a6e917c1820365cbebcb6a8974d1cd045ab4/pip-10.0.1-py2.py3-none-any.whl
Installing collected packages: pip
  Found existing installation: pip 10.0.1
    Uninstalling pip-10.0.1:
      Successfully uninstalled pip-10.0.1
  Rolling back uninstall of pip
Could not install packages due to an EnvironmentError: [Errno 13] Permission denied: '/usr/lib/python2.7/site-packages/pip'
Consider using the `--user` option or check the permissions.
```

主要是表示没有权限，因为python命令在`/usr/lib`下，并属于root用户。使用`sudo python get-pip.py`同样也会报错：

```shell
Collecting pip
  Retrying (Retry(total=4, connect=None, read=None, redirect=None, status=None)) after connection broken by 'NewConnectionError('<pip._vendor.urllib3.connection.VerifiedHTTPSConnection object at 0x2798b50>: Failed to establish a new connection: [Errno -2] Name or service not known',)': /simple/pip/
  Retrying (Retry(total=3, connect=None, read=None, redirect=None, status=None)) after connection broken by 'NewConnectionError('<pip._vendor.urllib3.connection.VerifiedHTTPSConnection object at 0x2798d10>: Failed to establish a new connection: [Errno -2] Name or service not known',)': /simple/pip/
  Retrying (Retry(total=2, connect=None, read=None, redirect=None, status=None)) after connection broken by 'NewConnectionError('<pip._vendor.urllib3.connection.VerifiedHTTPSConnection object at 0x2798a50>: Failed to establish a new connection: [Errno -2] Name or service not known',)': /simple/pip/

```

该错误主要是网络连接失败，实际上通过`sudo curl "www.baidu.com"`也能发现该网址无法联通，说明对于root用户来说，无法使用代理。这时尝试使用以下命令来为root创建代理：

```shell
sudo export http_proxy=http://nubia:nubia123@10.206.19.180:3128
sudo export https_proxy=http://nubia:nubia123@10.206.19.180:3128
```

发现报错：

```shell
sudo: export: command not found
```

最终想到一个办法：

```shell
sudo -E easy_install pip
```

`-E`命令表示采用当前用户的配置，也就是说让root用户使用当前有网络连接的方法。easy_install脚本则是linux自带的python已经安装好了的。

然后就能够使用pip了，使用方式为：

```shell
sudo -E pip install numpy
```

## 下载超时

出现pip安装时ReadTimeoutError，一般情况下PIP出现ReadTimeoutError都是因为被GFW给墙了，所以一般遇到这种问题，我们可以选择国内的镜像来解决问题。

```sh
pip install -i https://pypi.tuna.tsinghua.edu.cn/simple pykafka
```

使用该方法可以加快下载，另外一种方式是延长等待时间：

```sh
pip --default-timeout=100 install -U pip
```

## 按照列表下载

为了部署起来更加方便，应该使用`pip install -r requirements.txt方式批量下载`，文档格式如下：

```sh
pystan
fbprophet
pymongo
scikit-learn==0.19
pykafka>=2.7.0
```

# 安装fbprophet

使用pip命令安装fbprophet：

```sh
pip3 install fbprophet
```

报错为：

```sh
UnicodeDecodeError: 'utf8' codec can't decode byte 0xd5
```

并且提示了`C:\Program Files\python3.x\Lib\site-packages\pip\compat\__init.py`第75行的`return s.decode('utf_8')`有问题。解决方式为：

将该代码改成`return s.decode('gbk')`

再次执行安装命令，报错为

![1536832824098](assets/1536832824098.png)

# PySpark问题

报错如下：

```shell
19/05/14 19:15:34 WARN NativeCodeLoader: Unable to load native-hadoop library for your platform... using builtin-java classes where applicable
Exception in thread "main" java.io.IOException: Cannot run program "": error=2, No such file or directory
	at java.lang.ProcessBuilder.start(ProcessBuilder.java:1048)
	at org.apache.spark.deploy.PythonRunner$.main(PythonRunner.scala:100)
	at org.apache.spark.deploy.PythonRunner.main(PythonRunner.scala)
	at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
	at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
	at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
	at java.lang.reflect.Method.invoke(Method.java:498)
	at org.apache.spark.deploy.JavaMainApplication.start(SparkApplication.scala:52)
	at org.apache.spark.deploy.SparkSubmit$.org$apache$spark$deploy$SparkSubmit$$runMain(SparkSubmit.scala:894)
	at org.apache.spark.deploy.SparkSubmit$.doRunMain$1(SparkSubmit.scala:198)
	at org.apache.spark.deploy.SparkSubmit$.submit(SparkSubmit.scala:228)
	at org.apache.spark.deploy.SparkSubmit$.main(SparkSubmit.scala:137)
	at org.apache.spark.deploy.SparkSubmit.main(SparkSubmit.scala)
Caused by: java.io.IOException: error=2, No such file or directory
	at java.lang.UNIXProcess.forkAndExec(Native Method)
	at java.lang.UNIXProcess.<init>(UNIXProcess.java:247)
	at java.lang.ProcessImpl.start(ProcessImpl.java:134)
	at java.lang.ProcessBuilder.start(ProcessBuilder.java:1029)
	... 12 more
19/05/14 19:15:34 INFO ShutdownHookManager: Shutdown hook called
19/05/14 19:15:34 INFO ShutdownHookManager: Deleting directory /tmp/spark-a2817854-055a-460d-b081-29bee61889e8
```

从[StackOverflow网站](https://stackoverflow.com/questions/54570555/exception-in-thread-main-java-io-ioexception-cannot-run-program-error-2)的提问来看解决方案是：导入PYSPARK_PYTHON配置，将自己的python路径导入。

```shell
export PYSPARK_PYTHON= /usr/local/bin/python3
```

