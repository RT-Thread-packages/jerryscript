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

#include <string.h>
#include <rtthread.h>
#include <finsh.h>

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-core.h"
#include "jerryscript-ext/handler.h"
#include "jerry_extapi.h"

// #include <board.h>
// #include <drv_gpio.h>

// extern void gpio_set_value(enum gpio_port port,enum gpio_pin pin,int value);

void native_device_led(int port, int val)
{
    rt_kprintf("In native_device_led func, port[%d]:%d \n", port, val);
    // gpio_set_value(BLINK_LED0_PORT,BLINK_LED0_PIN, val);
}

DECLARE_HANDLER(userLed)
{
    jerry_value_t ret_val;

    if (args_cnt < 2)
    {
        ret_val = jerry_create_boolean(false);
        jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: invalid arguments number!\n");
        return ret_val;
    }

    if (!(jerry_value_is_number(args[0]) && jerry_value_is_number(args[1])))
    {
        ret_val = jerry_create_boolean(false);
        jerry_port_log(JERRY_LOG_LEVEL_ERROR, "Error: arguments must be numbers!\r\n");
        return ret_val;
    }

    int port, value;
    port = (int)jerry_get_number_value(args[0]);
    value = (int)jerry_get_number_value(args[1]);

    if (port >= 0 && port <= 3)
    {
        native_device_led(port, value);
        ret_val = jerry_create_boolean(true);
    }
    else
    {
        ret_val = jerry_create_boolean(false);
    }
    return ret_val;
}

void native_led_register()
{
    REGISTER_HANDLER(userLed);
}