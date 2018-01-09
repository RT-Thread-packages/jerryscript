/*
* File      : simple_run_hello.c
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2009-2018 RT-Thread Develop Team
*/

#include <string.h>
#include <rtthread.h>
#include <finsh.h>

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"
#include "jerryscript-ext/handler.h"
#include "jerry_extapi.h"

int simple_run_main(int argc, char *argv[])
{
    // can not use 'print' function, because of no definition of 'print' function in global object.
    const jerry_char_t script[] = "var str = 'Hello, World!';";
    size_t script_size = strlen((const char *)script);

    bool ret_value = jerry_run_simple(script, script_size, JERRY_INIT_EMPTY);

    return (ret_value ? 0 : 1);
}
MSH_CMD_EXPORT(simple_run_main, simple_run_main Demo);
