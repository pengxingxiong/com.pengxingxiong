# Dockerfile

```dockerfile
FROM centos:7.5.1804
MAINTAINER pengxingxiong@ruijie.com.cn

# install wget
RUN yum install -y wget
# replace yum repos
RUN mv /etc/yum.repos.d/CentOS-Base.repo /etc/yum.repos.d/CentOS-Base.repo.backup \
    && wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo \
    && enabled=0 \
    && yum clean all \
    && yum makecache

# Install c/c++ development tools
RUN yum install -y centos-release-scl 
RUN yum install -y devtoolset-3-toolchain
RUN scl enable devtoolset-3 bash
RUN printf "\nsource scl_source enable devtoolset-3\n" >> /root/.bashrc
RUN printf "\nsource scl_source enable devtoolset-3\n" >> /home/$BUILD_USER/.bashrc
ENV PATH /opt/rh/devtoolset-3/root/usr/bin:$PATH
RUN yum install -y make \
    && yum install -y tk-devel \
    && yum install -y openssl-devel \
    && yum install -y bzip2-devel \
    && yum install -y expat-devel \
    && yum install -y gdbm-devel \
    && yum install -y readline-devel \
    && yum install -y sqlite-devel
# Install Python 3.7.X for Umpire
ADD Python-3.5.2.tgz /usr/local
RUN cd /usr/local/Python-3.5.2/ \
    && ./configure --prefix=/usr/local \
    && make \
    && make install
# sshd
RUN yum install -y openssh-server passwd openssl
RUN mkdir /var/run/sshd
RUN ssh-keygen -q -t rsa -b 2048 -f /etc/ssh/ssh_host_rsa_key -N '' \
    && ssh-keygen -q -t ecdsa -f /etc/ssh/ssh_host_ecdsa_key -N '' \
    && ssh-keygen -t dsa -f /etc/ssh/ssh_host_ed25519_key -N ''
RUN echo 'root:rJ1#iBns' | chpasswd
RUN sed -ri 's/^PermitRootLogin\s+.*/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -ri 's/UsePAM yes/#UsePAM yes/g' /etc/ssh/sshd_config
EXPOSE 22
COPY ssh-start.sh /usr/local/
RUN chmod +x /usr/local/ssh-start.sh
# pip install
COPY docker-entrypoint.sh /tmp
RUN chmod +x /tmp/docker-entrypoint.sh && /tmp/docker-entrypoint.sh
```

