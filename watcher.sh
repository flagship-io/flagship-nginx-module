#!/bin/bash

export SHELL=/bin/bash

reload_module () {
    echo "Building the module ..."
    cd /work_directory/nginx-1.18.0/ && make modules && cp objs/ngx_http_fs_sdk_module.so /usr/lib/nginx/modules/
    cd ..
    echo "Module built and copied to nginx !"
    echo "Deleting nginx cache !"
    rm -rf /var/cache/nginx
    service nginx stop
    service nginx start
}

run_node () {
    echo "Running the node app ..."
    cd /work_directory/nginx-node-demo-vol/ && cp index.js ../nginx-node-demo && pm2 start /work_directory/nginx-node-demo/index.js
    cd ..
    echo "Deleting nginx cache !"
    rm -rf /var/cache/nginx
    echo "App started sucessfully"
}

reload_node () {
    echo "Rerunning the node app ..."
    cd /work_directory/nginx-node-demo-vol/ && cp index.js ../nginx-node-demo && pm2 restart /work_directory/nginx-node-demo/index.js
    cd ..
    echo "Deleting nginx cache !"
    rm -rf /var/cache/nginx
    echo "App restarted sucessfully"
}

run_node
reload_module

inotifywait -e close_write,moved_to,create -m ./ngx_http_fs_sdk_module -m ./nginx-node-demo-vol |
while read -r directory events filename; do
  if [ "$filename" = "ngx_http_fs_sdk_module.c" ]; then
    reload_node
    reload_module
  fi
  if [ "$filename" = "index.js" ]; then
    reload_node
    reload_module
  fi
done
