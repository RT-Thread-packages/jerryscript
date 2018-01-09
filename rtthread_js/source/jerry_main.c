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

#include <rtthread.h>
#include <finsh.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"

#include "jerry_run.h"
#include "jerry_extapi.h"

extern void persimmon_executor_register(void);
extern void native_led_register();

static const char js_file_main[] = "sd/main.js";
static const char js_file_blink[] = "sd/blink.js";
// static const char test_js_file_name[] = "sd/pm_class.js";

const char *script_file_names[] =
{
	js_file_main,
	js_file_blink
};

size_t out_source_code_size = 0;
char *out_source_code_buffer = RT_NULL;

static void jerry_thread_entry(void *parameter)
{
	jerry_init(JERRY_INIT_EMPTY);

	// register native C function, Prototype in jerry_extapi.c
	js_register_functions();

	// register native led api
	native_led_register();

	rt_kprintf("sizeof(script_file_names)/sizeof(char*) = %d \n", sizeof(script_file_names) / sizeof(char *));

	// Read js files source
	out_source_code_buffer = read_script_sources(script_file_names, sizeof(script_file_names) / sizeof(char *), &out_source_code_size);

	/* run main.js */
	int retcode = js_entry(out_source_code_buffer, out_source_code_size);
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

int jerry_main(void)
{
	rt_kprintf("\nJerryScript in RT-Thread\n");
	rt_kprintf("Version: \t%d.%d\n\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION);

	rt_thread_t thread;

	thread = rt_thread_create("jerry_main_thread", jerry_thread_entry, RT_NULL, 2048 * 20, RT_THREAD_PRIORITY_MAX / 3, 20);

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
// INIT_APP_EXPORT(jerry_main);
MSH_CMD_EXPORT(jerry_main, jerry_main test);
