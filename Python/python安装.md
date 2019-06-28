# Python3 中 离线安装

① 生成已安装模块列表，默认存在C:\Users\Administrator下

```shell
pip freeze > requirements.txt
```

② 下载列表中模块安装包，存入C盘下packages文件夹（如无此文件夹，则新建）

```shell
pip download -r requirements.txt -d c:/packages
```

③ requirements及packages文件夹拷贝至离线机，最好是对应目录，进行安装

```shell
$ pip install --no-index --find-links="c:/packages" -r requirements.txt
```

# liunx查看python的site-packages路径

有时候我们在liunx上想修改查看python的包路径可以试试以下命令

```python
from distutils.sysconfig import get_python_lib
print(get_python_lib())
```

运行结果为：

```shell
root@RG-ONC-VIR:/home/pengxx/python3-install# python3
Python 3.4.3 (default, Nov 12 2018, 22:25:49) 
[GCC 4.8.4] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> from distutils.sysconfig import get_python_lib
>>> print(get_python_lib())
/usr/lib/python3/dist-packages
>>> 
```

说明目录在dist-packages中，但是该方案不够准确。实际上可以通过搜索相关的包来发现目录，如下：

```shell
find / -name numpy
```

结果为：

```shell
/usr/local/lib/python3.4/dist-packages/numpy
/usr/local/lib/python3.4/dist-packages/numpy/core/include/numpy
```

这能够说正发生作用的包位置是`/usr/local/lib/python3.4/dist-packages/`

# 实践

## 1.下载get-pip.py文件
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
## 2.安装pip
python3 get-pip.py
会下载三个whl文件：
pip
https://files.pythonhosted.org/packages/5c/e0/be401c003291b56efc55aeba6a80ab790d3d4cece2778288d65323009420/pip-19.1.1-py2.py3-none-any.whl
setuptools
https://files.pythonhosted.org/packages/ec/51/f45cea425fd5cb0b0380f5b0f048ebc1da5b417e48d304838c02d6288a1e/setuptools-41.0.1-py2.py3-none-any.whl
wheel
https://files.pythonhosted.org/packages/2c/03/64c2d74bc381f9b8529151e7dda63921d3209be7b447bf876d96c115e7d3/wheel-0.33.2-py2.py3-none-any.whl

##3.卸载pip
python3 -m pip uninstall pip
##4.安装pip
python3 get-pip.py --no-index --find-links=./
##5.下载whl文件

（在在目标环境的自定义的目录下，因为会下载到当前目录）

pip3 download -i https://pypi.tuna.tsinghua.edu.cn/simple scikit-learn
pip3 download -i https://pypi.tuna.tsinghua.edu.cn/simple elasticsearch==5.5.3
pip3 download -i https://pypi.tuna.tsinghua.edu.cn/simple psycopg2
pip3 download -i https://pypi.tuna.tsinghua.edu.cn/simple typing

这个时候下载的numpy、psycopg2都是压缩文件，因此要将压缩文件转为whl文件。

##6.系统工具安装（目标类似环境）

在能够上网的机器上下载
1）rm -rf /var/cache/apt/archives/*  # 清空缓存目录，这一步也可以不做，用于看清楚哪些包是新下载的
2）安装系统工具
apt-get update

apt-get -d install python34-dev -y

##7.在无网络情况下安装whl文件（目标类似环境）

先将压缩文件转为whl文件
pip3 wheel ./lib/numpy-1.16.3.zip
pip3 wheel ./lib/psycopg2-2.8.2.tar.gz
pip3 install --no-index  -f ./lib ./lib/elasticsearch-5.5.3-py2.py3-none-any.whl
pip3 install --no-index  -f ./lib ./lib/numpy-1.16.3-cp34-cp34m-linux_x86_64.whl
pip3 install --no-index  -f ./lib ./lib/scipy-1.2.1-cp34-cp34m-manylinux1_x86_64.whl
pip3 install --no-index  -f ./lib ./lib/scikit_learn-0.20.3-cp34-cp34m-manylinux1_x86_64.whl
pip3 install --no-index  -f ./lib ./lib/urllib3-1.25.2-py2.py3-none-any.whl
pip3 install --no-index  -f ./lib ./lib/psycopg2-2.8.2-cp34-cp34m-linux_x86_64.whl
pip3 install --no-index  -f ./lib ./lib/typing-3.6.6-py3-none-any.whl

## 8.项目导入

将python项目的模块依赖打入到python3的site-packages中

/usr/local/lib/python3.4/dist-packages/
terminal-smart-terminal.pth
/user/ibnsdata/guard/node-code/smart-terminal

## 9.将python项目拷贝到对应目录
/user/ibnsdata/guard/node-code/smart-terminal

## 10.测试代码：
python3 /user/ibnsdata/guard/node-code/smart-terminal/start.py "{\"pgParams\":{\"postgreHost\":\"172.18.135.11\",\"postgrePort\":25432,\"postgreDbName\":\"terminal_smart_identify\",\"postgreUser\":\"postgresadmin\",\"postgrePassword\":\"admin123\"},\"esParams\":{\"sessionIndex\":\"onc_v1_smart-identify\",\"sessionDocType\":\"doc\",\"restHosts\":\"172.18.135.11:9201\"},\"algParams\":{\"identifyThreshold\":0.7,\"terminalTypeTable\":\"terminal_type\",\"terminalInfoTable\":\"terminal_info\",\"supervisedModelTable\":\"supervised_model\"}}"

## 其他命令
卸载
pip uninstall numpy scipy -y

# 基于微服务的安装

