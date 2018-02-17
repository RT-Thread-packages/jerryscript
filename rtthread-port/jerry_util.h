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
#define REGISTER_HANDLER_ALIAS(NAME, HANDLER) \
    jerryx_handler_register_global ( (jerry_char_t *)# NAME, HANDLER)
#define REGISTER_METHOD(OBJ, NAME) \
    js_add_function (OBJ, # NAME, NAME ## _handler)
#define REGISTER_METHOD_ALIAS(OBJ, NAME, HANDLER) \
    js_add_function (OBJ, # NAME, HANDLER)

#ifdef __cplusplus
extern "C"
{
#endif

void js_set_property(const jerry_value_t obj, const char *name,
    const jerry_value_t prop);

jerry_value_t js_get_property(const jerry_value_t obj, const char *name);

void js_add_function(const jerry_value_t obj, const char *name,
    jerry_external_handler_t func);
jerry_value_t js_call_function(const jerry_value_t obj, const char *name,
    const jerry_value_t args[], jerry_size_t args_cnt);

char *js_value_to_string(const jerry_value_t value);

void js_value_dump(const jerry_value_t value);
int js_read_file(const char* filename, char **script);

int js_util_init(void);

#ifdef __cplusplus
}
#endif

#endif
