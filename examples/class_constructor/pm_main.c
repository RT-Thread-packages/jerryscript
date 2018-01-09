/*
* File      : pm_main.c
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

extern void registry_pm_class();
extern void registry_global_Window();

// pm_class.js file is in class_constructor/js directory
static const char pm_class_js_file[] = "sd/pm_class.js";

static void pm_entry_task(void *parameter)
{
    int retcode = 0;

    size_t out_source_code_size = 0;
    char *out_source_code_buffer = RT_NULL;

    const char *script_files[] =
    {
        pm_class_js_file
    };

	jerry_init(JERRY_INIT_EMPTY);

	// register native C function, Prototype in jerry_extapi.c
    // register "print" function to global object, for using in javascript file.
    register_js_function ("print", jerryx_handler_print);
    registry_pm_class();
    registry_global_Window();    

	// Read js files source, in jerry_run.c
	out_source_code_buffer = read_script_sources(script_files, (int)(sizeof(script_files) / sizeof(char *)), (size_t *)(&out_source_code_size));

	/* run js */
	retcode = js_entry(out_source_code_buffer, out_source_code_size);
	if (retcode != 0)
	{
		rt_kprintf("js_entry failed code(%d)\n", retcode);
		js_exit();
		// while(1);
	}

	rt_kprintf("js entry success!\n");    

_exit:
	/* Cleanup engine */
	jerry_cleanup();
	return;
}

int pm_main(void)
{
	rt_kprintf("\nJerryScript in RT-Thread\n");
	rt_kprintf("Version: \t%d.%d\n\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION);

	rt_thread_t thread = RT_NULL;

	thread = rt_thread_create("jerry_main_thread", pm_entry_task, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX / 3, 20);

	if (thread != RT_NULL)
	{
		rt_thread_startup(thread);

		return 0;
	}
	else
	{
		rt_kprintf("JerryScript Thread Create Failure\n");
		return -1;
	}
}
MSH_CMD_EXPORT(pm_main, pm_main test No param);
