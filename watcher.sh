#!/bin/bash

export SHELL=/bin/bash

reload_module () {
    echo "Building the module ..."
    cd nginx-1.18.0/ && make modules && cp objs/ngx_http_fs_sdk_module.so /usr/lib/nginx/modules/
    cd ..
    echo "Module built and copied to nginx !"
    service nginx stop
    service nginx start
}

reload_module

inotifywait -e close_write,moved_to,create -m ./ngx_http_fs_sdk_module |
while read -r directory events filename; do
  if [ "$filename" = "ngx_http_fs_sdk_module.c" ]; then
    reload_module
  fi
done