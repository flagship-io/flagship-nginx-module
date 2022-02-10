#!/bin/bash

case $1 in

    help)
        echo "help is here"
        ;;

    add_module)
        echo "add the module to nginx dev kit"
        cd nginx-1.18.0 && ./configure --with-compat --add-dynamic-module=../ --with-pcre
        ;;
    
    remove_module)
        echo "Removing module ..."
        rm -rf /usr/lib/nginx/modules/ngx_http_fs_sdk_module.so
        echo "Module removed !"
        ;;
    
    build_module)
        echo "Building the module ..."
        cd nginx-1.18.0/ && make modules
        echo "Module built !"
        ;;

    copy_module)
        echo "Copying module to nginx lib..."
        cd nginx-1.18.0/ && cp objs/ngx_http_fs_sdk_module.so /usr/lib/nginx/modules/
        echo "Module copied !"
        ;;

    *)
        echo "unknown argument! Please rerun the script with argument help"
        ;;

esac