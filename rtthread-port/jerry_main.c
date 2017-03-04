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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jerry-api.h"
#include "jerry-port.h"

#include <rtthread.h>
#include <finsh.h>

/**
 * Maximum command line arguments number.
 */
#define JERRY_MAX_COMMAND_LINE_ARGS (16)

/**
 * Standalone Jerry exit codes.
 */
#define JERRY_STANDALONE_EXIT_CODE_OK   (0)
#define JERRY_STANDALONE_EXIT_CODE_FAIL (1)

/**
 * Read source files.
 *
 * @return concatenated source files
 */
static char* read_sources(const char *script_file_names[], int files_count, size_t *out_source_size_p)
{
	int i;
	char* source_buffer = NULL;
	char *source_buffer_tail = NULL;
	size_t total_length = 0;
	FILE *file = NULL;

	for (i = 0; i < files_count; i++)
	{
		const char *script_file_name = script_file_names[i];

		file = fopen(script_file_name, "r");
		if (file == NULL) {
			jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Failed to fopen [%s]\n",
					script_file_name);
			return NULL;
		}

		int fseek_status = fseek(file, 0, SEEK_END);
		if (fseek_status != 0)
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR,
					"Failed to fseek fseek_status(%d)\n", fseek_status);
			fclose(file);
			return NULL;
		}

		long script_len = ftell(file);
		if (script_len < 0)
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR,
					"Failed to ftell script_len(%ld)\n", script_len);
			fclose(file);
			break;
		}

		total_length += (size_t) script_len;

		fclose(file);
		file = NULL;
	}

	if (total_length <= 0)
	{
		jerry_port_log(JERRY_LOG_LEVEL_ERROR, "There's nothing to read\n");
		return NULL;
	}

	source_buffer = (char*) rt_malloc(total_length);

	if (source_buffer == NULL)
	{
		jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Out of memory error\n");
		return NULL;
	}

	memset(source_buffer, 0, sizeof(char) * total_length);
	source_buffer_tail = source_buffer;

	for (i = 0; i < files_count; i++)
	{
		const char *script_file_name = script_file_names[i];
		file = fopen(script_file_name, "r");

		if (file == NULL)
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Failed to fopen [%s]\n",
					script_file_name);
			break;
		}

		int fseek_status = fseek(file, 0, SEEK_END);
		if (fseek_status != 0)
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR,
					"Failed to fseek fseek_status(%d)\n", fseek_status);
			break;
		}

		long script_len = ftell(file);
		if (script_len < 0)
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR,
					"Failed to ftell script_len(%ld)\n", script_len);
			break;
		}

		rewind(file);

		const size_t current_source_size = (size_t) script_len;
		size_t bytes_read = fread(source_buffer_tail, 1, current_source_size,
				file);
		if (bytes_read < current_source_size)
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR,
					"Failed to fread bytes_read(%d)\n", bytes_read);
			break;
		}

		fclose(file);
		file = NULL;

		source_buffer_tail += current_source_size;
	}

	if (file != NULL)
	{
		fclose(file);
	}

	if (i < files_count)
	{
		jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Failed to read script N%d\n",
				i + 1);
		rt_free(source_buffer);
		return NULL;
	}

	*out_source_size_p = (size_t) total_length;

	return source_buffer;
} /* read_sources */

/**
 * JerryScript log level
 */
static jerry_log_level_t jerry_log_level = JERRY_LOG_LEVEL_ERROR;

/**
 * Main program.
 *
 * @return 0 if success, error code otherwise
 */
int jerry_main(int argc, char *argv[])
{
	if (argc > JERRY_MAX_COMMAND_LINE_ARGS)
	{
		jerry_port_log(JERRY_LOG_LEVEL_ERROR,
				"Too many command line arguments. Current maximum is %d\n",
				JERRY_MAX_COMMAND_LINE_ARGS);

		return JERRY_STANDALONE_EXIT_CODE_FAIL;
	}

	const char *file_names[JERRY_MAX_COMMAND_LINE_ARGS];
	int i;
	int files_counter = 0;

	jerry_init_flag_t flags = JERRY_INIT_EMPTY;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp("--mem-stats", argv[i]))
		{
			flags |= JERRY_INIT_MEM_STATS;
		}
		else if (!strcmp("--mem-stats-separate", argv[i]))
		{
			flags |= JERRY_INIT_MEM_STATS_SEPARATE;
		}
		else if (!strcmp("--show-opcodes", argv[i]))
		{
			flags |= JERRY_INIT_SHOW_OPCODES | JERRY_INIT_SHOW_REGEXP_OPCODES;
		}
		else if (!strcmp("--log-level", argv[i]))
		{
			if (++i < argc && strlen(argv[i]) == 1 && argv[i][0] >= '0'
					&& argv[i][0] <= '3')
			{
				jerry_log_level = argv[i][0] - '0';
			}
			else
			{
				jerry_port_log(JERRY_LOG_LEVEL_ERROR,
						"Error: wrong format or invalid argument\n");
				return JERRY_STANDALONE_EXIT_CODE_FAIL;
			}
		}
		else
		{
			file_names[files_counter++] = argv[i];
		}
	}

	if (files_counter == 0)
	{
		jerry_port_console("No input files, running a hello world demo:\n");
		char *source_p =
				"var a = 3.5; print('Hello world ' + (a + 1.5) + ' times from JerryScript')";

		jerry_run_simple((jerry_char_t *) source_p, strlen(source_p), flags);
		return 0;
	}

	size_t source_size;
	char *source_p = read_sources(file_names, files_counter, &source_size);

	if (source_p == NULL)
	{
		jerry_port_log(JERRY_LOG_LEVEL_ERROR,
				"JERRY_STANDALONE_EXIT_CODE_FAIL\n");
		return JERRY_STANDALONE_EXIT_CODE_FAIL;
	}

	bool success = jerry_run_simple((jerry_char_t *) source_p, source_size, flags);

	rt_free(source_p);

	if (!success)
	{
		return JERRY_STANDALONE_EXIT_CODE_FAIL;
	}
	return JERRY_STANDALONE_EXIT_CODE_OK;
} /* main */
MSH_CMD_EXPORT(jerry_main, jerryScript Demo);

/* JerryScript CMD Line Interpreter */
#define DECLARE_HANDLER(NAME) \
	static jerry_value_t \
	NAME ## _handler (const jerry_value_t func_value, \
						const jerry_value_t this_value, \
						const jerry_value_t args[], \
						const jerry_length_t args_cnt )

#define REGISTER_HANDLER(NAME) \
	register_native_function ( # NAME, NAME ## _handler)

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

static bool register_native_function(const char* name, jerry_external_handler_t handler)
{
	jerry_value_t global_object_val = jerry_get_global_object();
	jerry_value_t reg_function = jerry_create_external_function(handler);

	bool is_ok = true;

	if (!(jerry_value_is_function(reg_function) && jerry_value_is_constructor(reg_function)))
	{
		is_ok = false;
		jerry_port_log(JERRY_LOG_LEVEL_ERROR,
				"Error: create_external_function failed !!!\r\n");
		jerry_release_value(global_object_val);
		jerry_release_value(reg_function);
		return is_ok;
	}

	if (jerry_value_has_error_flag(reg_function))
	{
		is_ok = false;
		jerry_port_log(JERRY_LOG_LEVEL_ERROR,
				"Error: create_external_function has error flag! \n\r");
		jerry_release_value(global_object_val);
		jerry_release_value(reg_function);
		return is_ok;
	}

	jerry_value_t jerry_name = jerry_create_string((jerry_char_t *) name);

	jerry_value_t set_result = jerry_set_property(global_object_val, jerry_name, reg_function);

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

void js_register_functions(void)
{
	REGISTER_HANDLER(assert);
}

static void _jerry_thread(void* parameter)
{
	char *source_p;
	size_t source_size;

	jerry_init(JERRY_INIT_EMPTY);
	js_register_functions();

	while (1)
	{
#if 0
		jerry_value_t ret_val = jerry_eval((jerry_char_t *) source_p, source_size, false);

		if (jerry_value_has_error_flag(ret_val))
		{
			jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: jerry_eval failed!\r\n");
		}

		jerry_release_value(ret_val);
#else
		jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: Jerryscript CMD Line Interpreter Not Implementation\r\n");
		rt_thread_delay(RT_TICK_PER_SECOND);
#endif

	}
}

int js_run(int argc, char *argv[])
{
	rt_thread_t thread;

	thread = rt_thread_create("jsth", _jerry_thread, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX / 3, 20);

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
// MSH_CMD_EXPORT(js_run, JerryScript Cmd Interpreter);

