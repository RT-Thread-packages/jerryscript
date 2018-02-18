#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtthread.h>
#include <finsh.h>

#include "jerry_util.h"
#include "jerry_module.h"

#include <jerryscript-ext/module.h>
#include <ecma-globals.h>

typedef jerry_value_t (*module_init_func_t)(void);

/* load module from file system */
static bool load_module_from_filesystem(const jerry_value_t module_name, jerry_value_t *result)
{
    bool ret = false;
    char *module = js_value_to_string(module_name);

    jerry_size_t size = 256;
    char full_path[size + 9];

    sprintf(full_path, "%s", module);
    full_path[size + 8] = '\0';

    char *str = NULL;
    uint32_t len = js_read_file(full_path, &str);
    if (len == 0) goto __exit;

    (*result) = jerry_eval((jerry_char_t *)str, len, false);
    if (jerry_value_has_error_flag(*result))
        printf("failed to evaluate JS\n");
    else
        ret = true;

__exit:
    free(module);
    free(str);

    return ret;
}

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
#else
    int len = strlen(module) + 7;
    char module_fullname[len];

    snprintf(module_fullname, len, "__jsm_%s", module);
    module_fullname[len - 1] = '\0';

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

static jerryx_module_resolver_t load_filesystem_resolver =
{
    NULL,
    load_module_from_filesystem
};

static jerryx_module_resolver_t load_builtin_resolver =
{
    NULL,
    load_module_from_builtin
};

static const jerryx_module_resolver_t *resolvers[] =
{
    &load_builtin_resolver,
    &load_filesystem_resolver
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

    // Try each of the resolvers to see if we can find the requested module
    char *module = js_value_to_string(args[0]);
    jerry_value_t result = jerryx_module_resolve(args[0], resolvers, 2);
    if (jerry_value_has_error_flag(result))
    {
        printf("Couldn't load module %s\n", module);

        jerry_release_value(result);

        /* create result with error */
        result = jerry_create_error(JERRY_ERROR_TYPE,
                                  (const jerry_char_t *) "Module not found");
    }

    /* set back the module.exports */
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
