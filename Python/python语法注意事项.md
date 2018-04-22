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
