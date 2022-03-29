<p align="center">

<img  src="https://mk0abtastybwtpirqi5t.kinstacdn.com/wp-content/uploads/picture-solutions-persona-product-flagship.jpg"  width="211"  height="182"  alt="flagship-nginx-module"  />

</p>

# Flagship Module for Nginx

## Introduction

Flagship IO perform the operation of feature management at the front-end level (JavaScript) or back-end level (web application languages such as Java, Android, iOS, PHP, TypeScript, etc ...). Many clients however employ significant caching strategies for performance reasons. Using this strategy prevent the HTTP requests to reach the back-end application servers, because a front-facing web server responds directly with a cached version in nginx of the requested page. Unfortunately in this case, the back-end SDKs cannot be used, since the logic of feature management that happens in the back-end application won't execute anyway. To put it another way, code that execute feature management techniques will never be executed.

To solve this issue, we created the Flagship nginx module for one of the most popular web servers on the market today to provide a solution for this problem.
At the web server level, this allows to determine which feature will be utilized by each visitor using Go wrapper for C, so that we can perform feature management in web sever level.
As a result, the server can respond with a cached version of the proper variation using table key/value provided by nginx, allowing regular caching schemes to function normally.

In brief, we introduce web experimentation at the web server level with this new module.
This is a new level that sits in between the front-end and the nginx web server.

## Usage

### General concepts

Since nginx is written in C language, we wrote our module flagship in C to achieve maximal performance, using native module and built-in module developement kit.
The wrapper contains 2 major functions :

- init_sdk which is the function that initialize the wrapper with environment_id, api_key, polling_interval, log_level, tracking_enabled
- get_all_flags which is the function that return all flags based on specific visitor using visitor_id and visitor_context, and return a string that form flag_key:flag_value combination.

The idea is to store the flag combination in cache table as a key, that refer to the page cached, so each time the visitor trigger a http request he will get the cached page version which corresponds to him.

### Installation

There are 3 ways to install your nginx module

**Warning**

- All these ways require to have the file libflagship.so in your system at this exact path:

```
/usr/local/nginx/sbin/
```

- In this case, the `load_module` directive should be used in nginx.conf to load the module.
  For instance, in our case it's:

```
load_module modules/ngx_http_fs_sdk_module.so;
```

#### Building from nginx source

To build the flagship module share object file you have to download the nginx source code in addition to some libraries in order to compile the C file into SO file that can be be used directly to your running nginx server ! [Here's a small tuto](https://dev.to/armanism24/how-to-build-nginx-from-source-code-on-ubuntu-20-04-31e5)

To link statically against nginx, cd to nginx source directory and execute:

```
./configure --with-compat --add-module=/path/to/flagship-nginx-module --with-pcre
```

To compile as a dynamic module (nginx 1.18.0+), cd to nginx source directory and execute:

```
./configure --with-compat --add-dynamic-module=/path/to/flagship-nginx-module --with-pcre
```

#### Building from docker

We have build and published a docker image that you can pull from [Dockerhub](https://hub.docker.com/repository/docker/flagshipio/nginx-module-builder) or simply run the command

```
docker run --rm -t --name nginx-module-builder -e "NGINX_VERSION=1.21.6" \
 -v $(pwd)/fs-nginx-module/out:/usr/lib/flagship-module \
 flagshipio/nginx-module-builder
```

The docker will run a container that include all the dependecing that are needed to generate the module as a shared object file, then you can export it using volumes.
Note that you are required to specify the nginx version you want to generate as an environment variable.
**Warning** If the pipeline fails, please check that your version support nginx development kit !

#### Building from Github action

In order to run github action pipeline, you will have to fork the project and run it with your own runners. The pipeline generate an artifacts that include the shared object to inject in your nginx running server.
Note that you have to choose your version in the dropbox or check the box to insert the nginx version that you want (Example: 1.8.6)
**Warning** If the pipeline fails, please check that your version support nginx development kits !

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

To run the example that include running nginx server that implement caching system with flagship module and a simple nodejs web app, clone the project and cd to the project folder and run:

```
./example/dev/run.sh
```

To run the example that include running nginx server that implement caching system with flagship module and return the flag you can pull our image from [Dockerhub](https://hub.docker.com/repository/docker/flagshipio/nginx) and run:

```
docker run --rm -it -p 8080:80 -t --name nginx-standalone \
 flagshipio/nginx:nginx-1.21.6
```

**Or** run:

```
./example/standalone/run.sh
```

**Note:** You have to change the default.conf with the example here:

```
##
# You should look at the following URL's in order to grasp a solid understanding
# of Nginx configuration files in order to fully unleash the power of Nginx.
# https://www.nginx.com/resources/wiki/start/
# https://www.nginx.com/resources/wiki/start/topics/tutorials/config_pitfalls/
# https://wiki.debian.org/Nginx/DirectoryStructure
#
# In most cases, administrators will remove this file from sites-enabled/ and
# leave it as reference inside of sites-available where it will continue to be
# updated by the nginx packaging team.
#
# This file will automatically load configuration files provided by other
# applications, such as Drupal or Wordpress. These applications will be made
# available underneath a path with that package name, such as /drupal8.
#
# Please see /usr/share/doc/nginx-doc/examples/ for more detailed examples.
##

# Default server configuration
#

proxy_cache_path /var/cache/nginx levels=1:2 keys_zone=my_cache:10m;


map $http_user_agent $browser_type {
    default                                 "Edge";
    "~Mozilla.*Firefox*"             "Firefox";
    "~Chrome*"                       "Chrome";
}

server {

	listen 80 default_server;
	listen [::]:80 default_server;

	# SSL configuration
	#
	# listen 443 ssl default_server;
	# listen [::]:443 ssl default_server;
	#
	# Note: You should disable gzip for SSL traffic.
	# See: https://bugs.debian.org/773332
	#
	# Read up on ssl_ciphers to ensure a secure configuration.
	# See: https://bugs.debian.org/765782
	#
	# Self signed certs generated by the ssl-cert package
	# Don't use them in a production server!
	#
	# include snippets/snakeoil.conf;

	root /var/www/html;

	# Add index.php to the list if you are using PHP
	#index index.html index.htm index.nginx-debian.html;

	server_name _;
	error_log /var/log/nginx/error.log debug;

	fs_init 'c0n48jn5thv01k0ijmo0' 'BsIK86oh7c12c9G7ce4Wm1yBlWeaMf3t1S0xyYzI' '500' 'ERROR' '0';

	location /with_module {

		fs_visitor_id $request_id;
		fs_visitor_context browser:$browser_type;
		fs_get_all_flags;

		set $visitor_context browser:$browser_type;

		proxy_buffering on;
		proxy_cache my_cache;
		proxy_cache_valid any 10m;

		add_header X-Proxy-Cache $upstream_cache_status;
		add_header X-Flags $fs_flags;
		add_header X-Browser_type $browser_type;

		proxy_set_header X-Flagship-Flags $fs_flags;
		proxy_set_header X-Visitor-Id $request_id;
		proxy_set_header X-Visitor-Context $visitor_context;

		proxy_cache_key $fs_flags;

		default_type text/html;

		return 200 $fs_flags;
	}

	location /without_module {

		proxy_buffering on;
		proxy_cache my_cache;
		proxy_cache_valid any 10m;

		add_header X-Proxy-Cache $upstream_cache_status;

		set $visitor_context browser:$browser_type;

		proxy_set_header X-Visitor-Id $request_id;
		proxy_set_header X-Visitor-Context $visitor_context;

		default_type text/html;

		return 200 $fs_flags;

	}

	# pass PHP scripts to FastCGI server
	#
	#location ~ \.php$ {
	#	include snippets/fastcgi-php.conf;
	#
	#	# With php-fpm (or other unix sockets):
	#	fastcgi_pass unix:/var/run/php/php7.4-fpm.sock;
	#	# With php-cgi (or other tcp sockets):
	#	fastcgi_pass 127.0.0.1:9000;
	#}

	# deny access to .htaccess files, if Apache's document root
	# concurs with nginx's one
	#
	#location ~ /\.ht {
	#	deny all;
	#}
}


# Virtual Host configuration for example.com
#
# You can move that to a different file under sites-available/ and symlink that
# to sites-enabled/ to enable it.
#
#server {
#	listen 80;
#	listen [::]:80;
#
#	server_name example.com;
#
#	root /var/www/example.com;
#	index index.html;
#
#	location / {
#		try_files $uri $uri/ =404;
#	}
#}

```

And include module in nginx.conf by adding the line:

```
load_module modules/ngx_http_fs_sdk_module.so;
```

Then restart nginx service.

## Reference

- [Nginx development guide](http://nginx.org/en/docs/dev/development_guide.html)
- [Nginx module development](https://www.evanmiller.org/nginx-modules-guide.html)
- [Echo module](https://github.com/openresty/echo-nginx-module)
- [Extending nginx](https://www.nginx.com/resources/wiki/extending/)

## Copyright & License
