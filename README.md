# Flagship Module for Nginx

Adds the ability to feature management in web server level.

## Build

To link statically against nginx, cd to nginx source directory and execute:

    ./configure --with compat --add-module=/path/to/flagship-nginx-module --with-pcre

To compile as a dynamic module (nginx 1.9.11+), use:
  
	./configure --with compat --add-dynamic-module=/path/to/flagship-nginx-module --with-pcre

In this case, the `load_module` directive should be used in nginx.conf to load the module.

## Configuration

### fs_init
* **syntax**: `fs_init 'env_id' 'api_key' 'polling_interval' 'log_level';`
* **default**: `none`
* **context**: `server`

Initialize the SDK with 4 arguments : Environment id, Api Key, Polling Interval and Log level.
Once the initialization is done the 1st time, the second you execute the script it will bypass the initialization.

### set_visitor_id
* **syntax**: `set_visitor_id 'visitor_id';`
* **default**: `none`
* **context**: `location`

Set the visitor id

### set_visitor_context
* **syntax**: `set_visitor_context 'visitor_context';`
* **default**: `none`
* **context**: `location`

Set the visitor context

### get_all_flags
* **syntax**: `get_all_flags;`
* **default**: `none`
* **context**: `location`

Execute the function getAllFlags that return the flags based on visitor id and context but does not return it in nginx it rather store it in variable named fs_sdk_cache_var that you can use as a key/value in nginx cache table for feature management.
Note that you have to initialize the sdk and setting visitor id and context before running get_all_flags directive.


## Sample configuration
```
http {
    
	server {
        
        fs_init 'env_id' 'api_key' 'polling_interval' 'log_level';
        
        location /test {
            set_visitor_id 'visitor_id_test';
	    set_visitor_context 'visitor_context_test';
	    get_all_flags;

            echo $fs_sdk_cache_var;
        }

        location /experiment {
            set_visitor_id 'visitor_id_experiment';
	    set_visitor_context 'visitor_context_experiment';
	    get_all_flags;
		    
            echo $fs_sdk_cache_var;
        }
}
```
this module uses Go wrapper for C, which is based on the Go SDK that implement bucketing mode.

so the return on /test if we set visitor_id_test & visitor_context_test match isVip: false, will be:
```
isVip:false;
```
and the return on /experiment if we set visitor_id_experiment & visitor_context_experiment match isVip: true, will be:
```
isVip:true;
```



## Copyright & License

