/*
* File      : jscallc.c
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2009-2018 RT-Thread Develop Team
*/

#include <string.h>
#include <rtthread.h>
#include <finsh.h>
#include <math.h>
#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"
#include "jerryscript-ext/handler.h"
#include "jerry_extapi.h"

static int16_t my_c_function()
{
    srand((unsigned int)rt_tick_get());
    int16_t i = rand();
    printf("Hello i = %d", i);
    return i;
}

static jerry_value_t
my_c_function_handler(const jerry_value_t func,
                      const jerry_value_t thiz,
                      const jerry_value_t *args,
                      const jerry_length_t argscount)
{
    my_c_function();

    return jerry_create_undefined();
}

int js_call_c_main()
{
    const jerry_char_t script[] = "print('In javascript file...');\
                                    myCFunction();";

    //   const jerry_char_t script[] = "print ('Hello, World!');";
    size_t script_size = strlen((const char *)script);

    /* Setup Global scope code */
    jerry_value_t parsed_code = jerry_parse(script, script_size, false);

    if (!jerry_value_has_error_flag(parsed_code))
    {
        /* Execute the parsed source code in the Global scope */
        jerry_value_t ret_value = jerry_run(parsed_code);

        /* Returned value must be freed */
        jerry_release_value(ret_value);
    }

    /* Parsed source code must be freed */
    jerry_release_value(parsed_code);

    return 0;
}

int jerry_call_cfunc(int argc, char *argv[])
{
    /* Initialize engine */
    jerry_init(JERRY_INIT_EMPTY);

    register_js_function("print", jerryx_handler_print);

    // Create a JavaScript function from a C function wrapper
    jerry_value_t func = jerry_create_external_function(my_c_function_handler);

    bool is_ok = true;

    if (!(jerry_value_is_function(func) && jerry_value_is_constructor(func)))
    {
        is_ok = false;
        jerry_port_log(JERRY_LOG_LEVEL_ERROR,
                       "Error: create_external_function failed !!!\r\n");
        jerry_release_value(func);
        return is_ok;
    }

    if (jerry_value_has_error_flag(func))
    {
        is_ok = false;
        jerry_port_log(JERRY_LOG_LEVEL_ERROR,
                       "Error: create_external_function has error flag! \n\r");
        jerry_release_value(func);
        return is_ok;
    }

    // Create a JavaScript string to use as method name
    jerry_value_t prop = jerry_create_string((const jerry_char_t *)"myCFunction");

    // Get JerryScript's global object (what you would call window in a browser)
    jerry_value_t global = jerry_get_global_object();

    // Bind the 'func' function inside the 'global' object to the 'prop' name
    jerry_value_t result = jerry_set_property(global, prop, func);

    /* Check the return value, which is a JavaScript value */
    if (jerry_value_has_error_flag(result))
    {
        is_ok = false;
        jerry_port_log(JERRY_LOG_LEVEL_ERROR,
                       "Error: register_native_function failed: \r\n");
    }

    // Release all temporaries
    jerry_release_value(global);
    jerry_release_value(prop);
    jerry_release_value(func);
    jerry_release_value(result);

    if(js_call_c_main())
        rt_kprintf("js_call_c_main return false\n");

    /* Cleanup engine */
    jerry_cleanup ();

    return 0;
}

MSH_CMD_EXPORT(jerry_call_cfunc, jerry_call_cfunc: No param);
