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