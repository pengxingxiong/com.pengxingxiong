FROM harbor.mig.ruijie.net/rgibns-snapshot/spark:0.9-2.3.1
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
# pip ungrade to 18.0
RUN pip3 install --upgrade pip
# pip install pyspark
RUN cp  -r $SPARK_HOME/python/pyspark /usr/local/lib/python3.5/site-packages
ENV PYSPARK_PYTHON=/usr/local/bin/python3 
COPY requirements.txt /tmp
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple -r /tmp/requirements.txt
