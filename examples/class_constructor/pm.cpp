/*
* File      : pm.cpp
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2009-2018 RT-Thread Develop Team
*/

#include <string.h>

#include "pm_class.h"
#include "wrappers.h"

DECLARE_CLASS_FUNCTION(pmClass, setId)
{
    CHECK_ARGUMENT_COUNT(pmClass, setId, (args_count == 1));

    CHECK_ARGUMENT_TYPE_ALWAYS(pmClass, setId, 0, number);

    uintptr_t native_handle;
    jerry_get_object_native_handle(this_obj, &native_handle);

    int arg0 = jerry_get_number_value(args[0]);

    PMJS *this_pm = (PMJS *)native_handle;
    this_pm->setId(arg0);

    return jerry_create_undefined();
}

DECLARE_CLASS_FUNCTION(pmClass, getId)
{
    CHECK_ARGUMENT_COUNT(pmClass, getId, (args_count == 0));

    uintptr_t native_handle;
    jerry_get_object_native_handle(this_obj, &native_handle);

    PMJS *this_pm = (PMJS *)native_handle;

    int ret = this_pm->getId();

    return jerry_create_number(ret);
}

DECLARE_CLASS_CONSTRUCTOR(pmClass)
{

    PMJS *pm;
    if (args_count == 0)
    {
        rt_kprintf("args_count == 0\n");
        pm = new PMJS();
    }
    else
    {
        CHECK_ARGUMENT_COUNT(pmClass, __constructor, (args_count == 1));
        rt_kprintf("args_count == 1\n");
        CHECK_ARGUMENT_TYPE_ALWAYS(pmClass, __constructor, 0, number);
        pm = new PMJS(jerry_get_number_value(args[0]));
    }

    uintptr_t native_ptr = (uintptr_t)pm;

    // create the jerryscript object
    jerry_value_t js_object = jerry_create_object();
    jerry_set_object_native_handle(js_object, native_ptr, NULL);

    ATTACH_CLASS_FUNCTION(js_object, pmClass, setId);
    ATTACH_CLASS_FUNCTION(js_object, pmClass, getId);

    return js_object;
}

void Window(char* str)
    {
        rt_kprintf("In Window function\n");
    }


DECLARE_GLOBAL_FUNCTION(Window)
{
    rt_kprintf("DECLARE_GLOBAL_FUNCTION(Window) \n");
    CHECK_ARGUMENT_TYPE_ALWAYS(Window, Window, 0, object);

    jerry_value_t keys_array = jerry_get_object_keys (args[0]);

    if (jerry_value_is_array(keys_array))
    {
        rt_kprintf("keys_array is array, length is %d\n", jerry_get_array_length(keys_array));

        jerry_value_t array_string = jerry_value_to_string(keys_array);
        char *str_buf = (char *)rt_calloc(1, 1024);
        jerry_string_to_char_buffer(array_string, (jerry_char_t*)str_buf, 1024);
        rt_kprintf("array string is %s\n", str_buf);
        jerry_release_value(array_string);
    }

    jerry_release_value (keys_array);

    Window(NULL);
    return jerry_create_undefined();
}

extern "C" 
{
void registry_pm_class()
{
    if(REGISTER_CLASS_CONSTRUCTOR(pmClass))
    {
        rt_kprintf("REGISTER_CLASS_CONSTRUCTOR(pmClass) success!\n");
    }
    else{
        rt_kprintf("REGISTER_CLASS_CONSTRUCTOR(pmClass) Failed!\n");
    }
}

void registry_global_Window()
{
    if(REGISTER_GLOBAL_FUNCTION(Window))
    {
        rt_kprintf("REGISTER_GLOBAL_FUNCTION(Window) success!\n");
    }
    else{
        rt_kprintf("REGISTER_GLOBAL_FUNCTION(Window) Failed!\n");
    }
}
}

