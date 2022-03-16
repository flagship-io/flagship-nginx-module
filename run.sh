#!/bin/bash

docker build -t nginx_module .
docker run --rm -p 8080:80 -t --name nginx_module \
 -v $(pwd)/src:/work_directory/ngx_http_fs_sdk_module \
 -v $(pwd)/nginx-node-demo:/work_directory/nginx-node-demo-vol \
 -v $(pwd)/config/nginx-conf:/etc/nginx/sites-available/default \
 -v $(pwd)/config/fs_module.conf:/etc/nginx/modules-enabled/fs.conf \
 nginx_module   