#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtthread.h>
#include <finsh.h>

#include "jerry_util.h"
#include "jerry_module.h"
#include <dfs.h>

#include <jerryscript-ext/module.h>
#include <ecma-globals.h>

typedef jerry_value_t (*module_init_func_t)(void);

#ifdef RT_USING_DFS
char *js_module_dirname(char *path)
{
    size_t i;
	char *s = NULL;

    if (!path || !*path) return NULL;

	s = rt_strdup(path);
	if (!s) return NULL;

    i = strlen(s)-1;
    for (; s[i]=='/'; i--) if (!i) { s[0] = '/'; goto __exit; }
    for (; s[i]!='/'; i--) if (!i) { s[0] = '.'; goto __exit; }
    for (; s[i]=='/'; i--) if (!i) { s[0] = '/'; goto __exit; }

__exit:
    s[i+1] = 0;
    return s;
}

/* load module from file system */
static bool load_module_from_filesystem(const jerry_value_t module_name, jerry_value_t *result)
{
    bool ret = false;
    char *str = NULL;	
    char *module = js_value_to_string(module_name);

    char *dirname  = NULL;
    char *filename = NULL;

    jerry_value_t dirname_value  = ECMA_VALUE_UNDEFINED;
    jerry_value_t filename_value = ECMA_VALUE_UNDEFINED;
    jerry_value_t global_obj     = ECMA_VALUE_UNDEFINED;

    char *full_path = NULL;
	char *full_dir  = NULL;

    global_obj  = jerry_get_global_object();
    dirname_value = js_get_property(global_obj, "__dirname");
    if (jerry_value_is_string(dirname_value))
    {
        dirname = js_value_to_string(dirname_value);
    }
    else
    {
        dirname = NULL;
    }

    if (module[0] != '/') /* is a relative path */
    {
        full_path = dfs_normalize_path(dirname, module);
    }
    else
    {
        full_path = dfs_normalize_path(NULL, module);
    }
	free(dirname);

    uint32_t len = js_read_file(full_path, &str);
    if (len == 0) goto __exit;

    filename_value = js_get_property(global_obj, "__filename");

    /* set new __filename and __dirname */
	full_dir = js_module_dirname(full_path);

    js_set_string_property(global_obj, "__dirname",  full_dir);
    js_set_string_property(global_obj, "__filename", full_path);

    (*result) = jerry_eval((jerry_char_t *)str, len, false);
    if (jerry_value_has_error_flag(*result))
        printf("failed to evaluate JS\n");
    else
        ret = true;

    /* restore __filename and __dirname */
    js_set_property(global_obj, "__dirname",  dirname_value);
    js_set_property(global_obj, "__filename", filename_value);

__exit:
	if (full_dir) free(full_dir);
	if (full_path) free(full_path);

    jerry_release_value(global_obj);
    jerry_release_value(dirname_value);
    jerry_release_value(filename_value);

    free(module);
    free(str);

    return ret;
}
#endif

/* load builtin module */
static bool load_module_from_builtin(const jerry_value_t module_name,
                                     jerry_value_t *result)
{
    bool ret = false;
    jerry_value_t value = ECMA_VALUE_UNDEFINED;
    module_init_func_t module_init;

    char *module = js_value_to_string(module_name);
#ifdef HOST_BUILD
    {
        extern jerry_value_t js_module_rtthread_init(void);
        
        if (strcmp(module, "os") == 0)
        {
            module_init = js_module_rtthread_init;
            *result = module_init();
            ret = true;
        }
    }
#elif defined(RT_USING_FINSH)
    int len = strlen(module) + 7;
    char module_fullname[len];

    snprintf(module_fullname, len, "__jsm_%s", module);
    module_fullname[len - 1] = '\0';

    /* find syscall in shell symbol section */
    struct finsh_syscall* syscall = finsh_syscall_lookup(module_fullname);
    if (syscall)
    {
        module_init = (module_init_func_t)syscall->func;
        *result = module_init();
        ret = true;
    }
#endif

    free(module);

__exit:
    return ret;
}

#ifdef RT_USING_DFS
static jerryx_module_resolver_t load_filesystem_resolver =
{
    NULL,
    load_module_from_filesystem
};
#endif

static jerryx_module_resolver_t load_builtin_resolver =
{
    NULL,
    load_module_from_builtin
};

static const jerryx_module_resolver_t *resolvers[] =
{
    &load_builtin_resolver,
#ifdef RT_USING_DFS
    &load_filesystem_resolver
#endif
};

DECLARE_HANDLER(require)
{
    if (args_cnt == 0)
    {
        printf("No module name supplied\n");
        return jerry_create_null();
    }

    if (jerry_value_is_string(args[0]) == 0)
    {
        printf("No module name supplied as string\n");
        return jerry_create_null();
    }

    /* make new module.exports */
    jerry_value_t global_obj  = jerry_get_global_object();
    jerry_value_t modules_obj = ECMA_VALUE_UNDEFINED;
    jerry_value_t exports_obj = ECMA_VALUE_UNDEFINED;

    modules_obj = js_get_property(global_obj, "module");
    exports_obj = js_get_property(modules_obj, "exports");

    jerry_value_t module_exports_obj = jerry_create_object();
    js_set_property(modules_obj, "exports", module_exports_obj);
    jerry_release_value(module_exports_obj);

    /* Try each of the resolvers to see if we can find the requested module */
    char *module = js_value_to_string(args[0]);
    jerry_value_t result = jerryx_module_resolve(args[0], resolvers, sizeof(resolvers)/sizeof(resolvers[0]));
    if (jerry_value_has_error_flag(result))
    {
        printf("Couldn't load module %s\n", module);

        jerry_release_value(result);

        /* create result with error */
        result = jerry_create_error(JERRY_ERROR_TYPE,
                                  (const jerry_char_t *) "Module not found");
    }

    /* restore the parent module.exports */
    js_set_property(modules_obj, "exports", exports_obj);

    jerry_release_value(global_obj);
    jerry_release_value(modules_obj);
    jerry_release_value(exports_obj);

    free(module);

    return result;
}

int js_module_init(void)
{
    jerry_value_t global_obj = jerry_get_global_object();
    jerry_value_t modules_obj = jerry_create_object();
    jerry_value_t exports_obj = jerry_create_object();

    js_set_property(modules_obj, "exports", exports_obj);
    js_set_property(global_obj, "module", modules_obj);

    REGISTER_HANDLER(require);

    jerry_release_value(global_obj);
    jerry_release_value(modules_obj);
    jerry_release_value(exports_obj);

    return 0;
}
