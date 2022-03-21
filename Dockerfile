FROM ubuntu

LABEL Author_name="Chadi LAOULAOU" Author_email="chadi.laoulaou@abtasty.com"

ENV TZ=Europe/London \
    DEBIAN_FRONTEND=noninteractive\
    NGINX_VERSION=1.18.0\
    NODE_VERSION=17.7.1\
    NVM_DIR=/root/.nvm\
    NGX_VERSION=1.18.*

# Update aptitude with new repo
RUN apt-get update

RUN apt-get update -y --fix-missing

RUN apt-get upgrade -y

# Install software 
RUN apt-get install -y --allow-downgrades curl apache2-utils build-essential inotify-tools libbsd-dev wget libc-dev libunwind-dev make gcc libc6 zlib1g=1:1.2.11.dfsg-2ubuntu1.2 liblzma-dev liblzma5=5.2.4-1ubuntu1 libpcre3 libpcre3-dev libpcre++-dev zlib1g-dev libbz2-dev libxslt1-dev libxml2-dev libgeoip-dev libgoogle-perftools-dev libperl-dev libssl-dev libcurl4-openssl-dev libatomic-ops-dev nginx=${NGX_VERSION}

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

COPY ./example/backend /work_directory/nginx-node-demo-vol

COPY ./example/backend /work_directory/nginx-node-demo

RUN curl https://raw.githubusercontent.com/creationix/nvm/master/install.sh | bash 

RUN . "$NVM_DIR/nvm.sh" && nvm install ${NODE_VERSION}
RUN . "$NVM_DIR/nvm.sh" && nvm use v${NODE_VERSION}
RUN . "$NVM_DIR/nvm.sh" && nvm alias default v${NODE_VERSION}
ENV PATH="/root/.nvm/versions/node/v${NODE_VERSION}/bin/:${PATH}"

RUN cd nginx-${NGINX_VERSION} && ./configure --with-compat --add-dynamic-module=../ngx_http_fs_sdk_module --with-pcre

RUN cd nginx-node-demo && npm install pm2 -g && npm install -y 

COPY ./example/watcher.sh ./watcher.sh
RUN chmod +x ./watcher.sh

ENTRYPOINT ["/tini", "-g", "--", "./watcher.sh"]