FROM ubuntu

LABEL Author_name="Chadi LAOULAOU" Author_email="chadi.laoulaou@abtasty.com"

ENV TZ=Europe/London \
    DEBIAN_FRONTEND=noninteractive\
    NGINX_VERSION=1.20.2

# Update aptitude with new repo
RUN apt-get update

RUN apt-get update -y --fix-missing

RUN apt-get upgrade -y

# Install software 
RUN apt-get install -y --allow-downgrades build-essential libbsd-dev wget libc-dev libunwind-dev make gcc libc6 zlib1g=1:1.2.11.dfsg-2ubuntu1.2 liblzma-dev liblzma5=5.2.4-1ubuntu1 libpcre3 libpcre3-dev libpcre++-dev zlib1g-dev libbz2-dev libxslt1-dev libxml2-dev libgeoip-dev libgoogle-perftools-dev libperl-dev libssl-dev libcurl4-openssl-dev libatomic-ops-dev

ENV TINI_VERSION v0.19.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

RUN mkdir work_directory

WORKDIR /work_directory

# Download nginx-1.18 package from source
RUN wget https://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz
# Extract the tar
RUN tar zxf nginx-${NGINX_VERSION}.tar.gz

COPY ./src/libflagship.so /usr/local/nginx/sbin/libflagship.so

RUN cd nginx-${NGINX_VERSION} && ./configure && make && make install

COPY ./src /work_directory/ngx_http_fs_sdk_module

RUN cd nginx-${NGINX_VERSION} && ./configure --with-compat --add-dynamic-module=../ngx_http_fs_sdk_module --with-pcre

RUN export NGINX_VERSION=${NGINX_VERSION}

COPY ./watcher.sh ./watcher.sh
RUN chmod +rwx ./watcher.sh

ENTRYPOINT ["/tini", "-g", "--", "./watcher.sh"]