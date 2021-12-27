/**
 * @file   ngx_http_flagship_sdk_module.c
 * @author Chadi LAOULAOU <chadi.laoulaou@abtasty.com>
 * @date   ***********
 *
 * @brief  A Flagship SDK module for Nginx.
 *
 * @section LICENSE
 *
 * Copyright (C) 2021
 *
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "libflagship.h"

#define SDK_INIT "sdk init"
#define FLAGSHIP_SDK_ENABLED 1

static char *ngx_http_fs_sdk(ngx_conf_t *cf, void *post, void *data);
static ngx_int_t ngx_http_fs_sdk_handler(ngx_http_request_t *r);
static ngx_conf_post_handler_pt ngx_http_fs_p = ngx_http_fs_sdk;

typedef struct
{
    ngx_str_t env_id;
    //ngx_str_t api_key;
    //ngx_array_t *my_keyval;
} ngx_http_fs_sdk_init_loc_conf_t;

static void *
ngx_http_fs_sdk_init_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_fs_sdk_init_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_fs_sdk_init_loc_conf_t));
    if (conf == NULL)
    {
        return NULL;
    }

    //conf->my_keyval = NULL;

    return conf;
}
#if FLAGSHIP_SDK_ENABLED
static int flagship_sdk_initialized = 0;
/* static char *env_id = NULL;
static char *api_key = NULL;
static int polling_interval = 0;
static char *log_level = NULL;
static char *visitor_id = NULL;
static char *context = NULL; */
static void (*init_flagship)(char *, char *, int, char *);
static char *(*get_all_flags)(char *, char *);
#endif

/**
 * This module provided directive: fs_init.
 *
 */
static ngx_command_t ngx_http_fs_sdk_commands[] = {

    {ngx_string("fs_init"),                                /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                   /* location context and takes
                                            no arguments*/
     ngx_conf_set_str_slot,                             /* configuration setup function */
     NGX_HTTP_LOC_CONF_OFFSET,                             /* No offset. Only one context is supported. */
     offsetof(ngx_http_fs_sdk_init_loc_conf_t, env_id), /* No offset when storing the module configuration on struct. */
     &ngx_http_fs_p},

    ngx_null_command /* command termination */
};

static ngx_str_t env_id_string;
//static ngx_str_t api_key_string;

//static ngx_array_t * my_keyval_string;

/* The sdk init string. */
static u_char ngx_sdk_init[] = SDK_INIT;
//static u_char ngx_it_works[] = IT_WORKS;

/* The module context. */
static ngx_http_module_t ngx_http_fs_sdk_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    ngx_http_fs_sdk_init_create_loc_conf, /* create location configuration */
    NULL                                  /* merge location configuration */
};

/* Module definition. */
ngx_module_t ngx_http_fs_sdk_module = {
    NGX_MODULE_V1,
    &ngx_http_fs_sdk_module_ctx, /* module context */
    ngx_http_fs_sdk_commands,    /* module directives */
    NGX_HTTP_MODULE,             /* module type */
    NULL,                        /* init master */
    NULL,                        /* init module */
    NULL,                        /* init process */
    NULL,                        /* init thread */
    NULL,                        /* exit thread */
    NULL,                        /* exit process */
    NULL,                        /* exit master */
    NGX_MODULE_V1_PADDING};

#if FLAGSHIP_SDK_ENABLED
static void initialize_flagship_sdk(char *sdk_path, ngx_http_request_t *r)
{

    printf("initilize flagship !");

    if (flagship_sdk_initialized == 1)
    {
        return;
    }

    /* if (env_id == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no env_id defined");
        exit(1);
    }

    if (api_key == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no api_key defined");
        exit(1);
    }

    if (polling_interval == 0)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no polling_interval defined");
        exit(1);
    }

    if (log_level == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no log_level defined");
        exit(1);
    }

    if (visitor_id == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no visitor_id defined");
        exit(1);
    }

    if (context == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no context defined");
        exit(1);
    } */

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "initalizing the flagship sdk");

    void *handle;
    char *error;

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "calling dlopen()");
    handle = dlopen(sdk_path, RTLD_LAZY);
    if (!handle)
    {
        fputs(dlerror(), stderr);
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, dlerror());
        exit(1);
    }

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "dlopen succeded");

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "looking up flagship_sdk_initialization");
    init_flagship = dlsym(handle, "initFlagship");
    if ((error = dlerror()) != NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, error);
        exit(1);
    }

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "looking up flagship_sdk_get_all_flags");
    get_all_flags = dlsym(handle, "getAllFlags");
    if ((error = dlerror()) != NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, error);
        exit(1);
    }

    //create the sdk init handle
    init_flagship((char *)env_id_string.data, "BsIK86oh7c12c9G7ce4Wm1yBlWeaMf3t1S0xyYzI", 4000, "ERROR");
    //init_flagship((char *)env_id_string.data, (char *)api_key_string.data, 4000, "ERROR");
    flagship_sdk_initialized = 1;
}
#endif

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t ngx_http_fs_sdk_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;
    u_char *message;

    /* Set the Content-Type header. */
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *)"text/plain";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL)
    {
        return NGX_ERROR;
    }

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

#if FLAGSHIP_SDK_ENABLED

    char *flags;
    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "checking on feature");

    printf("calling initilize flagship !");

    initialize_flagship_sdk("/usr/local/nginx/sbin/libflagship.so", r);

    printf("after calling initilize flagship !");

    flags = get_all_flags("my_visitor", "browser:Chrome");
    if (flags)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "getting VIP");

        message = (u_char *)flags;
        //message = (u_char *)cglcf->env_id.data;
    }
    else
    {
        message = ngx_sdk_init;
    }
#else
    message = ngx_sdk_init;
#endif

    ngx_buf_t *msg = ngx_palloc(r->pool, ngx_strlen(message));
    if (msg == NULL)
    {
        return NGX_ERROR;
    }
    ngx_memcpy(msg, message, ngx_strlen(message));

    if (message != ngx_sdk_init)
    {
        free(message);
    }

    b->pos = (u_char *)msg;                                              /* first position in memory of the data */
    b->last = (u_char *)msg + ((long int)ngx_strlen((const char *)msg)); /* last position in memory of the data */

    b->memory = 1;   /* content is in read-only memory */
    b->last_buf = 1; /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = ((long int)ngx_strlen((const char *)msg));
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
} /* ngx_http_fs_sdk_handler */

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf
 *   Module configuration structure pointer.
 * @param cmd
 *   Module directives structure pointer.
 * @param conf
 *   Module configuration structure pointer.
 * @return string
 *   Status of the configuration setup.
 */
static char *ngx_http_fs_sdk(ngx_conf_t *cf, void *post, void *data)
{
    ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    /* Install the fs sdk handler. */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_fs_sdk_handler;

    ngx_str_t *env_id = data; // i.e., first field of ngx_fs_sdk_loc_conf_t

    /* ngx_array_t *my_keyval = data;

    ngx_str_t *value = my_keyval->elts; */

    if (ngx_strcmp(env_id->data, "") == 0) {
        return NGX_CONF_ERROR;
    }
    env_id_string.data = env_id->data;
    env_id_string.len = ngx_strlen(env_id_string.data);

    /* 
    if (ngx_strcmp(value[0].data, "") == 0)
    {
        return NGX_CONF_ERROR;
    }

    if (ngx_strcmp(value[0].data, "") == 0)
    {
        return NGX_CONF_ERROR;
    }
    env_id_string.data = value[0].data;
    env_id_string.len = ngx_strlen(value[0].data);

    api_key_string.data = value[1].data;
    api_key_string.len = ngx_strlen(value[1].data);
 */
    return NGX_CONF_OK;
} /* ngx_http_fs_sdk */
