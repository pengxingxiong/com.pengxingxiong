# 通过k8s的command和args配置Docker入口命令

入口命令给容器创建带来了一些灵活性，如果在k8s中想要覆盖DockerFile中的入口命令，自己重新定义入口命令是否可以呢？答案是肯定的，看下面一段.yaml文件： 

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: command-demo
  labels:
    purpose: demonstrate-command
spec:
  containers:
  - name: command-demo-container
    image: debian
    command: ["printenv"]
    args: ["HOSTNAME", "KUBERNETES_PORT"]
```

这是一个创建Pod的配置,在`containers`节点下，有如下两行 

```yaml
command: ["printenv"]
args: ["HOSTNAME", "KUBERNETES_PORT"]12
```

 这里，从字面意思不难看出，这两行可以实现覆盖DockerFile中的ENTRYPOINT功能。具体的`command`代表ENTRYPOINT的命令行，而`args`代表具体参数。 
 当用户同时写了`command`和`args`的时候自然是可以覆盖DockerFile中ENTRYPOINT的命令行和参数,那么对于具体情况呢，比如仅仅写了`command`或者`args`的时候呢？完整的情况分类如下：

- 如果`command`和`args`均没有写，那么用Docker默认的配置。
- 如果`command`写了，但`args`没有写，那么Docker默认的配置会被忽略而且仅仅执行.yaml文件的`command`（不带任何参数的）。
- 如果`command`没写，但`args`写了，那么Docker默认配置的ENTRYPOINT的命令行会被执行，但是调用的参数是.yaml中的`args`。
- 如果如果`command`和`args`都写了，那么Docker默认的配置被忽略，使用.yaml的配置。

