FROM node:12.18.3

WORKDIR /app
USER root
RUN apt-get update -y && cd /tmp \
    && yes N | apt-get -y install ruby-full \
    && curl -ssLO http://prdownloads.sourceforge.net/quickfix/quickfix-1.15.1.tar.gz \
    && tar -xzf quickfix-1.15.1.tar.gz \
    && cd quickfix \
    && ./bootstrap \
    && ./configure \
    && make \
    && make install \
    && mkdir -p /usr/local/include/quickfix/ \
    && cp config.h /usr/local/include/quickfix/

ENV LD_LIBRARY_PATH=/usr/local/lib/

RUN cd /tmp && wget https://www.python.org/ftp/python/3.9.4/Python-3.9.4.tgz \
 && tar xzf Python-3.9.4.tgz  \
 && cd Python-3.9.4 \
 && ./configure --enable-optimizations \
 && make -j7 install \
 && npm install -g node-gyp

ADD package.json /app
RUN npm install
ADD . /app
RUN cd /app && node-gyp configure && node-gyp build
