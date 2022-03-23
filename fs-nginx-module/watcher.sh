#!/bin/bash

export SHELL=/bin/bash

reload_module () {
    echo "Building the module ..."
    cd /work_directory/nginx-${NGINX_VERSION:-1.18.0}/ && make modules && cp objs/ngx_http_fs_sdk_module.so /work_directory/nginx_modules/
    cd ..
}

reload_module
