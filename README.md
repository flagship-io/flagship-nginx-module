# Flagship Module for Nginx

## Introduction

Adds the ability to feature management in web server level.

## Usage

### Installation

To link statically against nginx, cd to nginx source directory and execute:

    ./configure --with-compat --add-module=/path/to/flagship-nginx-module --with-pcre

To compile as a dynamic module (nginx 1.9.11+), use:

    ./configure --with-compat --add-dynamic-module=/path/to/flagship-nginx-module --with-pcre

In this case, the `load_module` directive should be used in nginx.conf to load the module.

### Configuration

#### fs_init

- **syntax**: `fs_init 'env_id' 'api_key' 'polling_interval' 'log_level' 'tracking_enabled';`
- **default**: `none`
- **context**: `server`

Initialize the SDK with 4 arguments : Environment id, Api Key, Polling Interval, Log level and Tracking enabled.
Once the initialization is done the 1st time, the second you execute the script it will bypass the initialization.

#### fs_visitor_id

- **syntax**: `fs_visitor_id 'visitor_id';`
- **default**: `none`
- **context**: `location`

Set the visitor id

#### fs_visitor_context

- **syntax**: `fs_visitor_context 'visitor_context';`
- **default**: `none`
- **context**: `location`

Set the visitor context

#### fs_get_all_flags

- **syntax**: `fs_get_all_flags;`
- **default**: `none`
- **context**: `location`

Execute the function getAllFlags that return the flags based on visitor id and context but does not return it in nginx it rather store it in variable named `fs_flags` that you can use as a key/value in nginx cache table for feature management.
Note that you have to initialize the sdk and setting visitor id and context before running get_all_flags directive.

### Running

### Sample configuration

```
http {

	server {

        fs_init 'env_id' 'api_key' 'polling_interval' 'log_level' 'tracking_enabled';

        location /test {
            fs_visitor_id 'visitor_id_test';
	    fs_visitor_context 'visitor_context_test';
	    fs_get_all_flags;

            echo $fs_flags;
        }

        location /experiment {
            fs_visitor_id 'visitor_id_experiment';
	    fs_visitor_context 'visitor_context_experiment';
	    fs_get_all_flags;

            echo $fs_flags;
        }
}
```

this module uses Go wrapper for C, which is based on the Go SDK that implement bucketing mode.

so the return on /test if we set visitor_id_test & visitor_context_test match IsVip: false, will be:

```
IsVip:false;
```

and the return on /experiment if we set visitor_id_experiment & visitor_context_experiment match IsVip: true, will be:

```
IsVip:true;
```

## Running the example

## Reference

## Copyright & License
