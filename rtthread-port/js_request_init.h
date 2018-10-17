#ifndef JS_REQUEST_INIT_H__
#define JS_REQUEST_INIT_H__

#include <rtthread.h>
#include <jerry_util.h>
#include <jerry_event.h>
#include <jerry_callbacks.h>

#include "webclient.h"


#include "jerry_buffer.h"
#include "pthread.h"

#define READ_MAX_SIZE 10*1024
#define HEADER_BUFSZ 1024

struct callback_info
{
    jerry_value_t target_value;
    jerry_value_t return_value;
    jerry_value_t data_value;
    char *callback_name;
};

struct webclientClose_info
{
    struct webclient_session* session;
};

struct read_paramater
{
    jerry_value_t target_value;
    struct webclient_session* session;
    struct js_callback* request_callback;
    struct js_callback *close_callback;
    bool isConnected ;
};
#endif