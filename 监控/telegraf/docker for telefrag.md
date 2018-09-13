# Dockerfile

```dockerfile
FROM telegraf:latest
MAINTAINER pengxingxiong@ruijie.com.cn
LABEL description="Telegraf docker image with custom setup"
USER root
```

# 测试镜像

```sh
docker run -it --rm telegraf:0.9-1.0
```

# k8s部署

```yaml
---
apiVersion: v1
kind: ConfigMap
metadata:
  name: telegraf
  namespace: default
  labels:
#    version: 1.0
    app: telegraf
data:
  telegraf.conf: |+

    [agent]
      collection_jitter = "0s"
      debug = false
      flush_interval = "10s"
      flush_jitter = "0s"
      hostname = "telegraf-polling-service"
      interval = "2s"
      logfile = ""
      # 批量的向输出插件输出telegraf的度量值
      metric_batch_size = 1000
      metric_buffer_limit = 10000
      # 使用主机模式
      omit_hostname = false
      # 精度
      precision = ""
      # 是否以安静模式运行
      quiet = false
      # 是否开启周期，比如在00, :10, :20，每隔10分钟收集
      round_interval = true

    [[outputs.influxdb]]
      database = "telegraf"
      urls = [
        "http://10.68.53.0:8086"
      ]
    [[inputs.cpu]]
      percpu = false
      totalcpu = true
    [[inputs.influxdb]]
      timeout = "5s"
      urls = [
        "http://10.68.53.0:8086/debug/vars"
      ]
    [[inputs.mongodb]]
      servers = [
        "mongodb://172.18.135.11:23001",
        "mongodb://172.18.135.12:23002",
        "mongodb://172.18.135.13:23003"
      ]
      gather_perdb_stats = false
    [[inputs.zookeeper]]
      servers = [
        "172.18.135.11:2181",
        "172.18.135.12:2181",
        "172.18.135.13:2181"
      ]
---
apiVersion: extensions/v1beta1
kind: Deployment
metadata:
  name: telegraf
  namespace: default
spec:
#  revisionHistoryLimit: 3
  replicas: 1
  template:
    metadata:
      labels:
#        version: 1.0
        app: telegraf
    spec:
      nodeSelector:
        node: "2"
      containers:
      - name: telegraf
        image: harbor.mig.ruijie.net/ondp/telegraf:0.9-1.0
        imagePullPolicy: IfNotPresent
        resources:
          limits:
            cpu: 1
            memory: 1Gi
          requests:
            cpu: 0.1
            memory: 256Mi
        volumeMounts:
        - name: config
          mountPath: /etc/telegraf
        - mountPath: /etc/telegraf.d
          name: glusterfs-default-volume
          subPath: telegrafdata
      volumes:
      - name: glusterfs-default-volume
        hostPath:
          path: /home/pengxx      
      - name: config
        configMap:
          name: telegraf
```

