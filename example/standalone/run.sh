#!/bin/bash

docker build -f Dockerfile.standalone --build-arg NGINX_VERSION=1.16.1 -t flagshipio/nginx:nginx-1.16.1 .
docker run --rm -it -p 8080:80 -t --name nginx-standalone \
 -v $(pwd)/example/standalone/config/nginx-conf:/etc/nginx/sites-available/nginx-conf.example \
 -v $(pwd)/example/standalone/config/fs_module.conf:/etc/nginx/modules-enabled/fs.conf \
 flagshipio/nginx:nginx-1.16.1