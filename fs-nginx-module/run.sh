#!/bin/bash

docker build -t make_nginx_module .
docker run --rm -t --name make_nginx_module \
 -v $(pwd)/src:/work_directory/ngx_http_fs_sdk_module \
 -v $(pwd)/out:/work_directory/nginx_modules \
 make_nginx_module