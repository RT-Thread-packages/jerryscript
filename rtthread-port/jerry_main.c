#include <stdio.h>
#include <stdlib.h>

#include <rtthread.h>
#include <finsh.h>

#include <jerryscript.h>
#include <jerry_util.h>
#include <jerry_module.h>
#include <jerry_callbacks.h>

#define malloc	rt_malloc
#define free	rt_free

extern void jerry_port_set_default_context(jerry_context_t *context);

static rt_mq_t _js_mq = NULL;

static void *context_alloc(size_t size, void *cb_data_p)
{
    return rt_calloc(1, size);
}

rt_bool_t js_mq_send(void *parameter)
{
    rt_err_t ret = -RT_ERROR;

    if (_js_mq)
    {
        ret = rt_mq_send(_js_mq, parameter, sizeof(void *));
    }

    if (ret == RT_EOK)
    {
        return RT_TRUE;
    }
    else
    {
        return RT_FALSE;
    }
}

static void jerry_thread_entry(void* parameter)
{
    char *script;
    size_t length;

    if (parameter == NULL)
    {
        printf("jerry_thread_entry (parameter == NULL)\n");
        return;
    }

    length = js_read_file((const char*)parameter, &script);
    printf("jerry read file : %s\n", (const char*)parameter);
    if (length > 0)
    {
        /* JERRY_ENABLE_EXTERNAL_CONTEXT */
        jerry_port_set_default_context(jerry_create_context(32 * 1024, context_alloc, NULL));

        /* Initialize engine */
        jerry_init(JERRY_INIT_EMPTY);

        /* Register 'print' function from the extensions */
        jerryx_handler_register_global((const jerry_char_t *)"print", jerryx_handler_print);

        js_util_init();

        /* add __filename, __dirname */
        jerry_value_t global_obj = jerry_get_global_object();
        char *full_path = NULL;
        char *full_dir = NULL;

        full_path = js_module_normalize_path(NULL, (const char*)parameter);
        full_dir = js_module_dirname(full_path);

        js_set_string_property(global_obj, "__dirname", full_dir);
        js_set_string_property(global_obj, "__filename", full_path);
        jerry_release_value(global_obj);

        /* Setup Global scope code */
        jerry_value_t parsed_code = jerry_parse(NULL, 0, (jerry_char_t*)script, length, JERRY_PARSE_NO_OPTS);
        if (jerry_value_is_error(parsed_code))
        {
            printf("jerry parse failed!\n");
        }
        else
        {
            _js_mq = rt_mq_create("js_mq", sizeof(void *), 128, RT_IPC_FLAG_FIFO);
            if (_js_mq)
            {
                js_mq_func_init(js_mq_send);

                /* Execute the parsed source code in the Global scope */
                jerry_value_t ret = jerry_run(parsed_code);
                if (jerry_value_is_error(ret))
                {
                    jerry_value_t err_str_val = jerry_value_to_string(ret);
                    char *err_string = js_value_to_string(err_str_val);
                    if (err_string)
                    {
                        printf("jerry run err : %s\n", err_string);
                        free(err_string);
                    }
                    jerry_release_value(err_str_val);
                }
                else
                {
                    while (1)
                    {
                        void *buffer = 0;

                        if (rt_mq_recv(_js_mq, &buffer, sizeof(void *), RT_WAITING_FOREVER) == RT_EOK)
                        {
                            if (buffer)
                            {
                                struct js_mq_callback *jmc = (struct js_mq_callback *)buffer;
                                js_call_callback(jmc->callback, jmc->args, jmc->size);
                            }
                        }
                        rt_thread_delay(rt_tick_from_millisecond(10));
                    }
                }

                rt_mq_delete(_js_mq);
                js_mq_func_deinit();
                /* Returned value must be freed */
                jerry_release_value(ret);
            }
        }

        /* Parsed source code must be freed */
        jerry_release_value(parsed_code);

        rt_free(script);

        js_util_cleanup();
        /* Cleanup engine */
        jerry_cleanup();

        rt_free((void *)jerry_port_get_current_context());
    }
}

int jerry_main(int argc, char** argv)
{
    rt_thread_t tid;

    if (argc != 2) return -1;

    tid = rt_thread_create("jerry", jerry_thread_entry, (void*)rt_strdup((const char*)argv[1]), 1024 * 32, 20, 10);
    if (tid)
    {
        rt_thread_startup(tid);
    }

    return 0;
}
MSH_CMD_EXPORT(jerry_main, jerryScript Demo);
