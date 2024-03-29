/**
 * @file   ngx_http_fs_sdk_module.c
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
#include <bsd/string.h>
#include "libflagship.h"

#define SDK_INIT "sdk init"
#define FLAGSHIP_SDK_ENABLED 1

static char *ngx_http_get_visitor_id(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_get_visitor_context(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_fs_sdk_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_fs_sdk_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_http_fs_sdk_get_all_flags_handler(ngx_http_request_t *r);
static char *ngx_http_add_params(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

typedef struct
{
    ngx_array_t *params;
    ngx_array_t *visitor_id_lengths;
    ngx_array_t *visitor_id_values;
    ngx_array_t *visitor_context_lengths;
    ngx_array_t *visitor_context_values;

} ngx_http_fs_sdk_init_loc_conf_t;

static void *ngx_http_fs_sdk_init_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_fs_sdk_init_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_fs_sdk_init_loc_conf_t));
    if (conf == NULL)
    {
        return NGX_CONF_ERROR;
    }

    return conf;
}

#if FLAGSHIP_SDK_ENABLED
static int flagship_sdk_initialized = 0;

static ngx_str_t env_id_string;
static ngx_str_t api_key_string;
static ngx_str_t timeout_string;
static ngx_str_t log_level_string;
static ngx_str_t tracking_enabled_string;

static void (*init_flagship)(char *, char *, int, char *, int);
static char *(*get_all_flags)(char *, char *);
#endif

/**
 * This module provided directive: fs_init, fs_visitor_id, fs_visitor_context.
 *
 */
static ngx_command_t ngx_http_fs_sdk_commands[] = {

    {ngx_string("fs_init"),                             /* directive */
     NGX_HTTP_SRV_CONF | NGX_CONF_TAKE5,                /* location context and takes no arguments*/
     ngx_http_add_params,                               /* configuration setup function */
     NGX_HTTP_SRV_CONF_OFFSET,                          /* No offset. Only one context is supported. */
     offsetof(ngx_http_fs_sdk_init_loc_conf_t, params), /* No offset when storing the module configuration on struct. */
     NULL},

    {ngx_string("fs_visitor_id"),               /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,        /* location context and takes no arguments*/
     ngx_http_get_visitor_id,                   /* configuration setup function */
     NGX_HTTP_LOC_CONF_OFFSET,                  /* No offset. Only one context is supported. */
     0,                                         /* No offset when storing the module configuration on struct. */
     NULL},

    {ngx_string("fs_visitor_context"),          /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,        /* location context and takes no arguments*/
     ngx_http_get_visitor_context,              /* configuration setup function */
     NGX_HTTP_LOC_CONF_OFFSET,                  /* No offset. Only one context is supported. */
     0,                                         /* No offset when storing the module configuration on struct. */
     NULL},

    ngx_null_command /* command termination */
};

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

    {ngx_string("fs_flags"), NULL, ngx_http_fs_sdk_variable, 0, NGX_HTTP_VAR_NOCACHEABLE, 0},

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

    if (env_id_string.data == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no env_id defined");
        exit(1);
    }

    if (api_key_string.data == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no api_key defined");
        exit(1);
    }

    if (timeout_string.data == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no polling_interval defined");
        exit(1);
    }

    if (tracking_enabled_string.data == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no tracking enabled defined");
        exit(1);
    }

    if (log_level_string.data == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no log_level defined");
        exit(1);
    }

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
    init_flagship((char *)env_id_string.data, (char *)api_key_string.data, atoi((char*) timeout_string.data), (char *)log_level_string.data, atoi((char*) tracking_enabled_string.data));
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
  return NGX_DONE;
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

#if FLAGSHIP_SDK_ENABLED
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

    if (value[4].len == 0)
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

    tracking_enabled_string.data = value[5].data;
    tracking_enabled_string.len = ngx_strlen(tracking_enabled_string.data);

#endif

    return NGX_CONF_OK;
}


static char *ngx_http_get_visitor_id(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_fs_sdk_init_loc_conf_t *loc_conf;
    ngx_str_t *value;
    ngx_http_script_compile_t script_compile;

    loc_conf = conf;
    value = cf->args->elts;

    ngx_memzero(&script_compile, sizeof(ngx_http_script_compile_t));
    script_compile.cf = cf;
    script_compile.source = &value[1];
    script_compile.lengths = &loc_conf->visitor_id_lengths;
    script_compile.values = &loc_conf->visitor_id_values;
    script_compile.variables = ngx_http_script_variables_count(&value[1]);
    script_compile.complete_lengths = 1;
    script_compile.complete_values = 1;

    if (ngx_http_script_compile(&script_compile) != NGX_OK)
        return NGX_CONF_ERROR;

    return NGX_CONF_OK;
} /* ngx_http_get_visitor_id */


static char *ngx_http_get_visitor_context(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_fs_sdk_init_loc_conf_t *loc_conf;
    ngx_str_t *value;
    ngx_http_script_compile_t script_compile;

    loc_conf = conf;
    value = cf->args->elts;

    ngx_memzero(&script_compile, sizeof(ngx_http_script_compile_t));
    script_compile.cf = cf;
    script_compile.source = &value[1];
    script_compile.lengths = &loc_conf->visitor_context_lengths;
    script_compile.values = &loc_conf->visitor_context_values;
    script_compile.variables = ngx_http_script_variables_count(&value[1]);
    script_compile.complete_lengths = 1;
    script_compile.complete_values = 1;

    if (ngx_http_script_compile(&script_compile) != NGX_OK)
        return NGX_CONF_ERROR;

    return NGX_CONF_OK;
} /* ngx_http_get_visitor_context */

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
    u_char *p = (unsigned char *)malloc(sizeof(p));
    u_char *value = (unsigned char *)malloc(sizeof(value));

#if FLAGSHIP_SDK_ENABLED

    ngx_http_fs_sdk_init_loc_conf_t *cglcf;

    initialize_flagship_sdk("/usr/local/nginx/sbin/libflagship.so", r);

    cglcf = ngx_http_get_module_loc_conf(r, ngx_http_fs_sdk_module);

    ngx_str_t visitor_id;
    ngx_http_script_run(r, &visitor_id, cglcf->visitor_id_lengths->elts, 0, cglcf->visitor_id_values->elts);

    char* visitorId = (char*)malloc(visitor_id.len+1);
    ngx_cpystrn((u_char*)visitorId, (u_char *)visitor_id.data, visitor_id.len+1);
    
    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "visitor ID : %s", visitorId);

    if (visitorId == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no visitor_id defined");
        exit(1);
    }

    ngx_str_t visitor_context;
    ngx_http_script_run(r, &visitor_context, cglcf->visitor_context_lengths->elts, 0, cglcf->visitor_context_values->elts);

    char* visitorContext = (char*)malloc(visitor_context.len+1);
    
    ngx_cpystrn((u_char*)visitorContext, (u_char*)visitor_context.data, visitor_context.len+1);
    
    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "visitor Context : %s", visitorContext);

    if (visitorContext == NULL)
    {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no context defined");
        exit(1);
    }

#else

    value = "Flagship sdk disabled";

#endif

    p = ngx_pnalloc(r->pool, NGX_ATOMIC_T_LEN);

    if (p == NULL)
    {
        return NGX_ERROR;
    }

    switch (data)
    {

    case 0:

        value = (u_char *)get_all_flags(visitorId, visitorContext);

        ngx_free(visitorId); 
        ngx_free(visitorContext); 

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

    ngx_free(value);
    
    return NGX_OK;
}