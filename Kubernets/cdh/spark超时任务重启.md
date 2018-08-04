在IBNS安装包中有一个monitor文件夹

```sh
[root@rgibns3 monitor]# ll
total 68
drwxr-xr-x 2 root root 4096 Jul 24 22:58 common
-rwxr-xr-x 1 root root  919 Jul 24 22:26 config.py
-rwxr-xr-x 1 root root  829 Jul 24 22:26 config.pyc
-rwxr-xr-x 1 root root 2870 Jul 24 22:26 control
drwxr-xr-x 6 root root 4096 Jul 24 22:26 depends
-rwxr-xr-x 1 root root    0 Jul 24 22:26 __init__.py
-rwxr-xr-x 1 root root 5150 Jul 24 22:26 kuduMonitor.py
-rwxr-xr-x 1 root root 8466 Jul 24 22:26 monitor.py
-rwxr-xr-x 1 root root   21 Jul 24 22:26 requirements.txt
-rwxr-xr-x 1 root root  190 Jul 24 22:26 run_impala_monitor.sh
drwxr-xr-x 2 root root 4096 Jul 24 22:26 script
-rwxr-xr-x 1 root root  721 Jul 24 22:26 stream_submit.py
drwxr-xr-x 2 root root 4096 Jul 24 22:26 test
drwxr-xr-x 2 root root 4096 Jul 24 22:26 var
-rwxr-xr-x 1 root root 1083 Jul 24 22:26 zk_writer.py
```

其中zk_writer.py负责将配置的任务信息发送到zookeeper上，common文件中的stream.py负责对任务的重启。

stream.py通过\cdh_installer_5.13.1\ruijie-cdh-playbook\roles\after_install\tasks\cron_jobs.yml文件中注册的cron方案来执行

cron_jobs.yml

```yaml
---

- name: Start impala check
  cron:
    name: "check impala"
    minute: "*/10"
    job: "/bin/bash /opt/cdh_installer_5.13.1/monitor/run_impala_monitor.sh"
  when: inventory_hostname in groups['master_servers'][0]


- name: Start yarn application check
  hosts: worker_servers
  cron:
    name: "check yarn application"
    minute: "*/1"
    job: "cd /opt/monitor/common/ && ./stream.py"
```

其中第二个就是每一分钟执行一次stream.py

在cdh_installer_5.13.1\ruijie-cdh-playbook\roles\common\tasks\monitor.yml文件中有一个代码：

```yaml
- name: Install depends
  command: ./install.sh
  args:
    chdir: "/opt/monitor/depends"
  when: inventory_hostname in groups['worker_servers']
```

这个文件安装了需要的jar包。



# 包需求



# 重装cdh注意

需要删除cdh的虚ip，关闭keepalive的三个容器，再重启机器。

