# Flagship Module for Nginx

## Introduction

Flagship IO perform the operation of feature management at the front-end level (JavaScript) or back-end level (web application languages such as Java, Android, iOS, PHP, TypeScript, etc ...). Many clients however employ significant caching strategies for performance reasons. Using this strategy prevent the HTTP requests to reach the back-end application servers, because a front-facing web server responds directly with a cached version in nginx of the requested page. Unfortunately in this case, the back-end SDKs cannot be used, since the logic of feature management that happens in the back-end application won't execute anyway. To put it another way, code that execute feature management techniques will never be executed.

To solve this issue, we created the Flagship nginx module for one of the most popular web servers on the market today to provide a solution for this problem.
At the web server level, this allows to determine which feature will be utilized by each visitor using Go wrapper for C, so that we can perform feature management in web sever level.
As a result, the server can respond with a cached version of the proper variation using table key/value provided by nginx, allowing regular caching schemes to function normally.

In brief, we introduce web experimentation at the web server level with these new modules.
This is a new level that sits in between the front-end and the nginx web server.

## Usage

### General concepts

Since nginx is written in C language, we wrote our module flagship in C to achieve maximal performance, using native module and built-in module developement kit.
The wrapper contains 2 major functions :

- init_sdk which is the function that initialize the wrapper with environment_id, api_key, polling_interval, log_level, tracking_enabled
- get_all_flags which is the function that return all flags based on specific visitor using visitor_id and visitor_context, and return a string that form flag_key:flag_value combination.

The idea is to store the flag combination in cache table as a key, that refer to the page cached, so each time the visitor trigger a http request he will get the cached page version which corresponds to him.

### Installation

To link statically against nginx, cd to nginx source directory and execute:

    ./configure --with-compat --add-module=/path/to/flagship-nginx-module --with-pcre

To compile as a dynamic module (nginx 1.8.0+), cd to nginx source directory and execute:

    ./configure --with-compat --add-dynamic-module=/path/to/flagship-nginx-module --with-pcre

In this case, the `load_module` directive should be used in nginx.conf to load the module.

### Configuration

#### fs_init

- **syntax**: `fs_init 'env_id' 'api_key' 'polling_interval' 'log_level' 'tracking_enabled';`
- **default**: `none`
- **context**: `server`

Initialize the SDK with 5 arguments : Environment id, Api Key, Polling Interval, Log level and Tracking enabled.
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

Execute the function getAllFlags that return the flags based on visitor id and context but does not return it in nginx it rather store it in variable named `fs_flags` that you can use as a key/value in nginx cache table for feature management. (The variable is accessible with $fs_flags).
Note that you have to initialize the sdk and set the visitor id and context before running get_all_flags directive.

### Running

### Sample configuration

```
http {

	server {

        fs_init 'env_id' 'api_key' 'polling_interval' 'log_level' 'tracking_enabled';

        location /variation1 {
            fs_visitor_id 'visitor_id_variation1';
	        fs_visitor_context 'visitor_context_variation1';
	        fs_get_all_flags;

            #Warning Flagship module don't come with built-in echo module, make sure to include one.
            echo $fs_flags;
        }

        location /variation2 {
            fs_visitor_id 'visitor_id_variation2';
	        fs_visitor_context 'visitor_context_variation2';
	        fs_get_all_flags;

            #Warning Flagship module don't come with built-in echo module, make sure to include one.
            echo $fs_flags;
        }
}
```

this module uses Go wrapper for C, which is based on the Go SDK that implement bucketing mode.

so the return on /variation1 if we set visitor_id_variation1 & visitor_context_variation1 match IsVip: false, the variable fs_flags will be : IsVip: false

and the return on /variation2 if we set visitor_id_variation2 & visitor_context_variation2 match IsVip: true, the variable fs_flags will be :
IsVip:true;

## Running the example

To run the example

```
./example/run.sh
```

## Reference

## Copyright & License
