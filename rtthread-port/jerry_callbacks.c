

#include <jerry_callbacks.h>

struct js_callback *_js_callback = NULL;
static js_mq_func _js_mq_func = NULL;

static void append_callback(struct js_callback *callback)
{
    struct js_callback *_callback = _js_callback;

    if (_js_callback == NULL)
    {
        _js_callback = callback;
        return;
    }

    while (_callback->next != NULL)
    {
        _callback = _callback->next;
    }

    _callback->next = callback;
}

static void remove_callback(struct js_callback *callback)
{
    struct js_callback *_callback_free, *_callback = _js_callback;

    if (_js_callback == callback)
    {
        _js_callback = _js_callback->next;
        rt_free(_callback);
        return;
    }

    while (_callback->next != NULL)
    {
        if (_callback == callback)
        {
            _callback_free = _callback;
            _callback = _callback->next;
            rt_free(_callback_free);
            break;
        }

        _callback = _callback->next;
    }
}

static rt_bool_t has_callback(struct js_callback *callback)
{
    struct js_callback *_callback = _js_callback;

    if (callback == NULL)
    {
        return RT_FALSE;
    }

    while (_callback != NULL)
    {
        if (_callback == callback)
        {
            return RT_TRUE;
        }

        _callback = _callback->next;
    }

    return RT_FALSE;
}

struct js_callback *js_add_callback(js_callback_func callback)
{
    struct js_callback *cb = (struct js_callback *)rt_calloc(1, sizeof(struct js_callback));
    if (!cb)
    {
        return NULL;
    }

    cb->function = callback;
    cb->next = NULL;

    append_callback(cb);

    return cb;
}

void js_remove_callback(struct js_callback *callback)
{
    remove_callback(callback);
}

void js_remove_all_callbacks(void)
{
    struct js_callback *_callback_free, *_callback = _js_callback;

    while (_callback != NULL)
    {
        _callback_free = _callback;
        _callback = _callback->next;
        rt_free(_callback_free);
    }

    _js_callback = NULL;
}

void js_call_callback(struct js_callback *callback, const void *data, uint32_t size)
{
    if (has_callback(callback))
    {
        if (callback->function)
        {
            callback->function(data, size);
        }
    }
}

void js_send_callback(struct js_callback *callback, const void *args, uint32_t size)
{
    rt_bool_t ret = RT_FALSE;
    struct js_mq_callback *jmc = NULL;

    jmc = (struct js_mq_callback *)rt_calloc(1, sizeof(struct js_mq_callback));
    if (jmc)
    {
        jmc->callback = callback;
        jmc->args = rt_malloc(size);
        if (jmc->args && args)
        {
            memcpy(jmc->args, args, size);
        }
        jmc->size = size;

        if (_js_mq_func)
        {
            ret = _js_mq_func(jmc);
        }

        if (ret == RT_FALSE)
        {
            rt_free(jmc->args);
            rt_free(jmc);
        }
    }
}

void js_mq_func_set(js_mq_func signal)
{
    _js_mq_func = signal;
}
