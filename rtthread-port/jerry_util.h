#ifndef JERRY_UTIL_H__
#define JERRY_UTIL_H__

#include <jerryscript-ext/handler.h>

#define DECLARE_HANDLER(NAME) \
    static jerry_value_t \
    NAME ## _handler (const jerry_value_t func_value, \
                        const jerry_value_t this_value, \
                        const jerry_value_t args[], \
                        const jerry_length_t args_cnt )

#define REGISTER_HANDLER(NAME) \
    jerryx_handler_register_global ( (jerry_char_t *)# NAME, NAME ## _handler)
#define REGISTER_METHOD(OBJ, NAME) \
    js_add_function (OBJ, # NAME, NAME ## _handler)

#ifdef __cplusplus
extern "C"
{
#endif

void js_set_property(const jerry_value_t obj, const char *name,
    const jerry_value_t prop);

jerry_value_t js_get_property(const jerry_value_t obj, const char *name);

void js_add_function(const jerry_value_t obj, const char *name, 
    jerry_external_handler_t func);

void js_value_dump(const jerry_value_t value);

#ifdef __cplusplus
}
#endif

#endif
