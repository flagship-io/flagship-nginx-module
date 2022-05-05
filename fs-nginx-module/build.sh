#!/bin/bash

export SHELL=/bin/bash

reload_module () {
    echo "Downloading nginx from the source ..."
    wget https://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz
    echo "Extracting nginx ..."
    tar zxf nginx-${NGINX_VERSION}.tar.gz
    echo "Configuring nginx ..."
    cd /work_directory/nginx-${NGINX_VERSION} && ./configure && make && make install
    echo "Building the module ..."
    ./configure --with-compat --add-dynamic-module=../ngx_http_fs_sdk_module --with-pcre && make modules
    cp objs/ngx_http_fs_sdk_module.so /usr/lib/flagship-module/
    echo "Flagship module generated and copied successfully"
    cd ..
}

reload_module
