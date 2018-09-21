## 运行时异常的抛出
```python
import sys

try:
    f = open('./1foo.txt')
    s = f.readline()
    i = int(s.strip())
except OSError as err:
    print("OS error: {0}".format(err))
except ValueError:
    print("Could not convert data to an integer.")
except:
    print("Unexpected error:", sys.exc_info()[0])
    raise  # 和java中的throw差不多，相当于主动抛出一个运行时异常
```
利用raise就能够主动抛出异常

## 打印异常信息
对于python3.x版本，打印异常需要使用以下方式：
```python
def divide(x, y):
    try:
        result = x / y
    except ZeroDivisionError:
        print("division by zero!")
    except TypeError as e:
        print(e)
    else:
        print("result is", result)
    finally:
        print("executing finally clause")


divide('2', '1')
```
对于python2.x版本，打印异常需要使用以下方式：
```python
def divide(x, y):
    try:
        result = x / y
    except ZeroDivisionError:
        print("division by zero!")
    except TypeError, e:
        print(e)
    else:
        print("result is", result)
    finally:
        print("executing finally clause")


divide('2', '1')
```

# python获取当前目录路径和上级路径

在使用python的时候总会遇到路径切换的使用情况，如想从文件夹test下的`test.py`调用data文件夹下的`data.txt`文件：

```python
.
└── folder
    ├── data
    │   └── data.txt
    └── test
        └── test.py123456
```

一种方法可以在data文件下加入`__init__.py` 然后在`test.py` 中`import data` 就可以调用`data.txt`文件；

另一种方法可以借助python os模块的方法对目录结构进行操作，下面就说一下这种方式的使用：

```python
import os

print '***获取当前目录***'
print os.getcwd()
print os.path.abspath(os.path.dirname(__file__))

print '***获取上级目录***'
print os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
print os.path.abspath(os.path.dirname(os.getcwd()))
print os.path.abspath(os.path.join(os.getcwd(), ".."))

print '***获取上上级目录***'
print os.path.abspath(os.path.join(os.getcwd(), "../.."))12345678910111213
```

输出结果为：

```python
***获取当前目录***
/workspace/demo/folder/test
/workspace/demo/folder/test

***获取上级目录***
/workspace/demo/folder
/workspace/demo/folder
/workspace/demo/folder

***获取上上级目录***
/workspace/demo
```

