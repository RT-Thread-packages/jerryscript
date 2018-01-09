/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <rtthread.h>

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"
#include "jerryscript-ext/handler.h"

#include "jerry_extapi.h"

/**
 * Default implementation of jerryx_port_handler_print_char. Uses 'printf' to
 * print a single character to standard output.
 */
void jerryx_port_handler_print_char(char c) /**< the character to print */
{
    printf("%c", c);
} /* jerryx_port_handler_print_char */

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

DECLARE_HANDLER(assert)
{
    if (args_cnt == 1 && jerry_value_is_boolean(args[0]) && jerry_get_boolean_value(args[0]))
    {
        jerry_port_log(JERRY_LOG_LEVEL_DEBUG, ">> Jerry assert true\r\n");
        return jerry_create_boolean(true);
    }

    jerry_port_log(JERRY_LOG_LEVEL_ERROR, "ERROR: Script assertion failed\n");

    return jerry_create_boolean(false);
}

bool register_native_function(const char *name,
                              jerry_external_handler_t handler)
{
    jerry_value_t global_object_val = jerry_get_global_object();
    jerry_value_t reg_function = jerry_create_external_function(handler);

    bool is_ok = true;

    if (!(jerry_value_is_function(reg_function) && jerry_value_is_constructor(reg_function)))
    {
        is_ok = false;
        jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: create_external_function failed !!!\r\n");
        jerry_release_value(global_object_val);
        jerry_release_value(reg_function);
        return is_ok;
    }

    if (jerry_value_has_error_flag(reg_function))
    {
        is_ok = false;
        jerry_port_log(JERRY_LOG_LEVEL_ERROR,
                       "Error: create_external_function has error flag! \n");
        jerry_release_value(global_object_val);
        jerry_release_value(reg_function);
        return is_ok;
    }

    jerry_value_t jerry_name = jerry_create_string((jerry_char_t *)name);

    jerry_value_t set_result = jerry_set_property(global_object_val,
                                                  jerry_name,
                                                  reg_function);

    if (jerry_value_has_error_flag(set_result))
    {
        is_ok = false;
        jerry_port_log(JERRY_LOG_LEVEL_ERROR,
                       "Error: register_native_function failed: [%s]\r\n", name);
    }

    jerry_release_value(jerry_name);
    jerry_release_value(global_object_val);
    jerry_release_value(reg_function);
    jerry_release_value(set_result);

    return is_ok;
}


/**
 * Register a JavaScript function in the global object.
 */
void register_js_function(const char *name_p,                 /**< name of the function */
                          jerry_external_handler_t handler_p) /**< function callback */
{
    jerry_value_t result_val = jerryx_handler_register_global((const jerry_char_t *)name_p, handler_p);

    if (jerry_value_has_error_flag(result_val))
    {
        rt_kprintf("Error: register global func err\n");
        jerry_port_log(JERRY_LOG_LEVEL_WARNING, "Warning: failed to register '%s' method.", name_p);
    }

    jerry_release_value(result_val);

} /* register_js_function */

// extern void registry_pm_class();

void js_register_functions(void)
{
    register_js_function("print", jerryx_handler_print);
}
