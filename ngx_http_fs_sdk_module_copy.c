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

static char *ngx_http_get_all_flags(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_fs_sdk_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_fs_sdk_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_http_fs_sdk_get_all_flags_handler(ngx_http_request_t *r);
static char *ngx_http_add_params(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_fs_sdk(ngx_conf_t *cf, void *post, void *data);
static ngx_conf_post_handler_pt ngx_http_fs_sdk_p = ngx_http_fs_sdk;

typedef struct
{
    ngx_array_t *params;
    ngx_str_t visitor_id;
    ngx_str_t visitor_context;
    ngx_str_t visitor_flags;

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
static ngx_str_t env_id_string;
static ngx_str_t api_key_string;
static ngx_str_t timeout_string;
static ngx_str_t log_level_string;

static void (*init_flagship)(char *, char *, int, char *);
static char *(*get_all_flags)(char *, char *);
#endif

/**
 * This module provided directive: fs_init.
 *
 */
static ngx_command_t ngx_http_fs_sdk_commands[] = {

    {ngx_string("fs_init"),                             /* directive */
     NGX_HTTP_SRV_CONF | NGX_CONF_TAKE4,                /* location context and takes
                                            no arguments*/
     ngx_http_add_params,                               /* configuration setup function */
     NGX_HTTP_SRV_CONF_OFFSET,                          /* No offset. Only one context is supported. */
     offsetof(ngx_http_fs_sdk_init_loc_conf_t, params), /* No offset when storing the module configuration on struct. */
     NULL},

    {ngx_string("set_visitor_id"),                          /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                    /* location context and takes
                                            no arguments*/
     ngx_conf_set_str_slot,                                 /* configuration setup function */
     NGX_HTTP_LOC_CONF_OFFSET,                              /* No offset. Only one context is supported. */
     offsetof(ngx_http_fs_sdk_init_loc_conf_t, visitor_id), /* No offset when storing the module configuration on struct. */
     &ngx_http_fs_sdk_p},

    {ngx_string("set_visitor_context"),                          /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                         /* location context and takes
                                            no arguments*/
     ngx_conf_set_str_slot,                                      /* configuration setup function */
     NGX_HTTP_LOC_CONF_OFFSET,                                   /* No offset. Only one context is supported. */
     offsetof(ngx_http_fs_sdk_init_loc_conf_t, visitor_context), /* No offset when storing the module configuration on struct. */
     &ngx_http_fs_sdk_p},

    {ngx_string("get_all_flags"),         /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, /* location context and takes
                                            no arguments*/
     ngx_http_get_all_flags,              /* configuration setup function */
     NGX_HTTP_LOC_CONF_OFFSET,            /* No offset. Only one context is supported. */
     0,                                   /* No offset when storing the module configuration on struct. */
     NULL},

    ngx_null_command /* command termination */
};

//static ngx_array_t * my_keyval_string;

/* The sdk init string. */
//static u_char ngx_sdk_init[] = SDK_INIT;
//static u_char ngx_it_works[] = IT_WORKS;

/* The module context. */
static ngx_http_module_t ngx_http_fs_sdk_module_ctx = {
    ngx_http_fs_sdk_add_variables, /* preconfiguration */
    NULL,                          /* postconfiguration */

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

static ngx_http_variable_t ngx_http_fs_sdk_vars[] = {

    {ngx_string("fs_sdk_cache_var"), NULL, ngx_http_fs_sdk_variable, 0, NGX_HTTP_VAR_NOCACHEABLE, 0},

    ngx_http_null_variable

};

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
    //init_flagship((char *)env_id_string.data, "BsIK86oh7c12c9G7ce4Wm1yBlWeaMf3t1S0xyYzI", 4000, "ERROR");
    init_flagship((char *)env_id_string.data, (char *)api_key_string.data, (long int)timeout_string.data, (char *)log_level_string.data);
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

static ngx_int_t ngx_http_fs_sdk_get_all_flags_handler(ngx_http_request_t *r)
{
    ngx_int_t    rc;
    ngx_buf_t *b;
    ngx_chain_t out;

    ngx_http_fs_sdk_init_loc_conf_t *cglcf;

    cglcf = ngx_http_get_module_loc_conf(r, ngx_http_fs_sdk_module);

    /* Set the Content-Type header. */
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *)"text/plain";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL)
    {
        return NGX_ERROR;
    }

    rc = ngx_http_discard_request_body(r);
 
    if (rc != NGX_OK) {
        return rc;
    }

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

/* #if FLAGSHIP_SDK_ENABLED


    
#else
    
#endif */

    b->last = ngx_sprintf(b->last, "Active connections: %s \n", cglcf->visitor_flags.data);

    r->headers_out.status = NGX_HTTP_OK;

    r->headers_out.content_length_n = b->last - b->pos;

    b->last_buf = (r == r->main) ? 1 : 0;

    b->last_in_chain = 1;

    /* b->pos = (u_char *)msg;     */                                          /* first position in memory of the data */
    /* b->last = (u_char *)msg + ((long int)ngx_strlen((const char *)msg)); */ /* last position in memory of the data */

    /* b->memory = 1;  */  /* content is in read-only memory */
    /* b->last_buf = 1; */ /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = ((long int)ngx_strlen((const char *)cglcf->visitor_flags.data));
    ngx_http_send_header(r); /* Send the headers */

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

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

static char *ngx_http_add_params(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *pcf;

    pcf = ngx_http_conf_get_module_srv_conf(cf, ngx_http_core_module);
    pcf->handler = ngx_http_fs_sdk_get_all_flags_handler;

    ngx_str_t *value;
    ngx_array_t **params;

    value = cf->args->elts;

    params = (ngx_array_t **)((char *)pcf + cmd->offset);

    if (*params == NULL)
    {
        return NGX_CONF_ERROR;
    }

    if (value[1].len == 0)
    {
        return NGX_CONF_ERROR;
    }

    if (value[2].len == 0)
    {
        return NGX_CONF_ERROR;
    }

    if (value[3].len == 0)
    {
        return NGX_CONF_ERROR;
    }

    env_id_string.data = value[1].data;
    env_id_string.len = ngx_strlen(env_id_string.data);

    api_key_string.data = value[2].data;
    api_key_string.len = ngx_strlen(api_key_string.data);

    timeout_string.data = value[3].data;
    timeout_string.len = ngx_strlen(timeout_string.data);

    log_level_string.data = value[4].data;
    log_level_string.len = ngx_strlen(log_level_string.data);

    return NGX_CONF_OK;
}

static char *ngx_http_get_all_flags(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    /* Install the hello world handler. */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_fs_sdk_get_all_flags_handler;

    return NGX_CONF_OK;
} /* ngx_http_get_all_flags */

static char *ngx_http_fs_sdk(ngx_conf_t *cf, void *post, void *data)
{

    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_fs_sdk_get_all_flags_handler;

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_fs_sdk_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t *var, *v;
    for (v = ngx_http_fs_sdk_vars; v->name.len; v++)
    {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL)
        {
            return NGX_ERROR;
        }
        var->get_handler = v->get_handler;
        var->data = v->data;
    }
    return NGX_OK;
}

static ngx_int_t ngx_http_fs_sdk_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char *p;
    u_char *value;
    ngx_http_fs_sdk_init_loc_conf_t *cglcf;
    char *flags;

    initialize_flagship_sdk("/usr/local/nginx/sbin/libflagship.so", r);

    cglcf = ngx_http_get_module_loc_conf(r, ngx_http_fs_sdk_module);

    flags = get_all_flags((char *)cglcf->visitor_id.data, (char *)cglcf->visitor_context.data);
    
    p = ngx_pnalloc(r->pool, NGX_ATOMIC_T_LEN);

    if (p == NULL)
    {
        return NGX_ERROR;
    }

    switch (data)
    {

    case 0:

        value = (u_char*)flags;
        break;
    default:

        value = 0;

        break;
    }

    v->len = ngx_sprintf(p, "%s", value) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}