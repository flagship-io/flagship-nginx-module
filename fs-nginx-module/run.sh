#!/bin/bash

docker build -f Dockerfile.build -t flagshipio/nginx-module-builder .
docker run --rm -t --name nginx-module-builder -e "NGINX_VERSION=1.20.0" \
 -v $(pwd)/fs-nginx-module/out:/usr/lib/flagship-module \
 flagshipio/nginx-module-builder