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
#include <rthw.h>
#include <string.h>
#include <rtthread.h>

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"

/**
 * Signal the port that jerry experienced a fatal failure from which it cannot
 * recover.
 *
 * @param code gives the cause of the error.
 *
 * Note:
 *      Jerry expects the function not to return.
 *
 * Example: a libc-based port may implement this with exit() or abort(), or both.
 */
void jerry_port_fatal (jerry_fatal_code_t code)
{
    rt_kprintf("jerryScritp fatal...\n");
    rt_hw_interrupt_disable();
    while (1);
}

/*
 *  I/O Port API
 */
#define RT_JS_CONSOLEBUF_SIZE   256
static char rt_log_buf[RT_JS_CONSOLEBUF_SIZE];

/**
 * Display or log a debug/error message. The function should implement a printf-like
 * interface, where the first argument specifies the log level
 * and the second argument specifies a format string on how to stringify the rest
 * of the parameter list.
 *
 * This function is only called with messages coming from the jerry engine as
 * the result of some abnormal operation or describing its internal operations
 * (e.g., data structure dumps or tracing info).
 *
 * It should be the port that decides whether error and debug messages are logged to
 * the console, or saved to a database or to a file.
 *
 * Example: a libc-based port may implement this with vfprintf(stderr) or
 * vfprintf(logfile), or both, depending on log level.
 */
void jerry_port_log (jerry_log_level_t level, const char *format, ...)
{
    va_list args;
    rt_size_t length;

    va_start(args, format);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, format, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
#ifdef RT_USING_DEVICE
    rt_kprintf("%s", rt_log_buf);
#else
    rt_hw_console_output(rt_log_buf);
#endif
    va_end(args);
}

/**
 * Default implementation of jerryx_port_handler_print_char. Uses 'printf' to
 * print a single character to standard output.
 */
void jerryx_port_handler_print_char (char c) /**< the character to print */
{
    rt_kprintf("%c", c);
} /* jerryx_port_handler_print_char */

/*
 * Date Port API
 */

/**
 * Get timezone and daylight saving data
 *
 * @return true  - if success
 *         false - otherwise
 */
bool jerry_port_get_time_zone (jerry_time_zone_t *tz_p)
{
    tz_p->offset = 0;
    tz_p->daylight_saving_time = 0;
    return true;
}

/**
 * Get system time
 *
 * @return milliseconds since Unix epoch
 */
double jerry_port_get_current_time (void)
{
    return rt_tick_get() * RT_TICK_PER_SECOND / 1000;
}
