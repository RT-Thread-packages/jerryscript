/*
* File      : jerry_eval_hello.c
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2009-2018 RT-Thread Develop Team
*/

#include <rtthread.h>
#include <finsh.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"
#include "jerryscript-ext/handler.h"

#include "jerry_run.h"
#include "jerry_extapi.h"

int jerry_eval_main(int argc, char *argv[])
{
    const jerry_char_t script[] = "print ('Hello, World!');";
    size_t script_size = strlen((const char *)script);

    rt_kprintf("jerry engine init...\n");
    /* Initialize engine */
    jerry_init(JERRY_INIT_EMPTY);

    // register "print" function to global object, for using in javascript file.
    register_js_function("print", jerryx_handler_print);    

    rt_kprintf("jerry parse...\n");
    /* Setup Global scope code */
    jerry_value_t parsed_code = jerry_parse(script, script_size, false);
    
    rt_kprintf("jerry run...\n");
    if (!jerry_value_has_error_flag(parsed_code))
    {
        rt_kprintf("jerry parse return success!\n");
        /* Execute the parsed source code in the Global scope */
        jerry_value_t ret_value = jerry_run(parsed_code);

        if(jerry_value_has_error_flag(ret_value))
        {
            rt_kprintf("jerry run return error!\n");
        }

        /* Returned value must be freed */
        jerry_release_value(ret_value);
    }

    /* Parsed source code must be freed */
    jerry_release_value(parsed_code);

    /* Cleanup engine */
    jerry_cleanup();

    return 0;
}

MSH_CMD_EXPORT(jerry_eval_main, jerry_eval_main demo);