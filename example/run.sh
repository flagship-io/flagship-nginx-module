#!/bin/bash

docker build -t nginx_module .
docker run --rm -p 8080:80 -t --name nginx_module \
 -v $(pwd)/src:/work_directory/ngx_http_fs_sdk_module \
 -v $(pwd)/example/backend:/work_directory/nginx-node-demo-vol \
 -v $(pwd)/example/config/nginx-conf:/etc/nginx/sites-available/default \
 -v $(pwd)/example/config/fs_module.conf:/etc/nginx/modules-enabled/fs.conf \
 nginx_module   