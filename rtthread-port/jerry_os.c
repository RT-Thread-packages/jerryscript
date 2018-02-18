#include <rtthread.h>
#include <finsh.h>

#include "jerry_util.h"

DECLARE_HANDLER(ps)
{
    /* list thread information */
#ifdef HOST_BUILD
    printf("list thread\n");
#else
    list_thread();
#endif

    return jerry_create_undefined();
}

jerry_value_t js_module_rtthread_init(void)
{
    printf("to create rtthread module\n");
    jerry_value_t rtthread_obj = jerry_create_object();
    REGISTER_METHOD(rtthread_obj, ps);
    
    return rtthread_obj;
}
FINSH_FUNCTION_EXPORT_ALIAS(js_module_rtthread_init, __jsm_os, NULL);
