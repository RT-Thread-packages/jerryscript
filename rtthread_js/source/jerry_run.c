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

#include "jerry_extapi.h"
#include "jerry_run.h"

/**
 * Read javascript source files.
 *
 * @return concatenated source files
 */
char *read_script_sources(const char *script_file_names[], int files_count, size_t *out_source_size_p)
{
    int i;
    char *source_buffer = NULL;
    char *source_buffer_tail = NULL;
    size_t total_length = 0;
    FILE *file = NULL;

    for (i = 0; i < files_count; i++)
    {
        const char *script_file_name = script_file_names[i];

        file = fopen(script_file_name, "r");
        if (file == NULL)
        {
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

        total_length += (size_t)script_len;

        fclose(file);
        file = NULL;
    }

    if (total_length <= 0)
    {
        jerry_port_log(JERRY_LOG_LEVEL_ERROR, "There's nothing to read\n");
        return NULL;
    }

    source_buffer = (char *)rt_malloc(total_length);

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

        const size_t current_source_size = (size_t)script_len;
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

    *out_source_size_p = (size_t)total_length;

    return source_buffer;
} /* read_script_sources */


/**
 * Read js function entry.
 *
 * @return status
 */
int js_entry(const char *source_p, const size_t source_size)
{
    const jerry_char_t *jerry_src = (const jerry_char_t *)source_p;

    uint8_t ret_code = 0;

    jerry_value_t parsed_code = jerry_parse(jerry_src, source_size, false);

    if (!jerry_value_has_error_flag(parsed_code))
    {
        rt_kprintf("before jerry run \n");
        jerry_value_t ret_value = jerry_run(parsed_code);
        rt_kprintf("end jerry run \n");
        if (jerry_value_has_error_flag(ret_value))
        {
            rt_kprintf("Error: ret_value has an error flag! %d\n", jerry_get_number_value(ret_value));
            jerry_release_value(ret_value);
            jerry_release_value(parsed_code);
            return ret_code = -1;
        }

        jerry_release_value(ret_value);
    }
    else
    {
        rt_kprintf("Error: jerry_parse failed!\n");
        ret_code = -1;
    }

    jerry_release_value(parsed_code);

    return ret_code;
} /* js_entry */


/**
 * js_eval.
 *
 * @return status
 */
int js_eval(const char *source_p, const size_t source_size)
{
    int status = 0;

    jerry_value_t ret_val = jerry_eval((jerry_char_t *)source_p,
                                       source_size,
                                       false);

    if (jerry_value_has_error_flag(ret_val))
    {
        rt_kprintf("Error: jerry_eval failed!\n");
        status = -1;
    }

    jerry_release_value(ret_val);

    return status;
} /* js_eval */

void js_exit(void)
{
    jerry_cleanup();
}
