通过kubectl获取k8s的主节点地址和端口

```shell
kubectl cluster-info
```

将该地址作为spark任务提交的master参数。

提交测试：

```shell
bin/spark-submit \
    --master k8s://https://172.18.130.23:8443 \
    --deploy-mode cluster \
    --name spark-pi \
    --class org.apache.spark.examples.JavaSparkPi \
    --conf spark.executor.instances=5 \
    --conf spark.kubernetes.container.image=harbor.mig.ruijie.net/ondp/spark:2.4.3 \
    local:///opt/spark/examples/jars/spark-examples_2.11-2.4.3.jar
```

其中最后的`local://`表示执行jar包在image目录中，而不使用`local://`表示采取服务器的文件目录

测试报错：

Message: Forbidden!Configured service account doesn't have access. Service account may have been revoked. pods "spark-pi-1559532503956-driver" is forbidden: User "system:serviceaccount:default:default" cannot get pods in the namespace "default".

这表示了当前执行用户没有访问API得权限，具体可以参考如下：

>[kubernetes account](https://kubernetes.io/docs/tasks/configure-pod-container/configure-service-account/)
>[kubernetes role](https://kubernetes.io/docs/reference/access-authn-authz/rbac/)

如果不想看，可使用下面得命令进行全部用户赋予管理员权限：

```shell
kubectl create clusterrolebinding permissive-binding \
  --clusterrole=cluster-admin \
  --user=admin \
  --user=kubelet \
  --group=system:serviceaccounts
```

也可以参考这个配置yaml文件：

>[kubernets---kubernetes-client的使用--java中提交argo工作流](https://blog.csdn.net/zzq900503/article/details/88352902)

