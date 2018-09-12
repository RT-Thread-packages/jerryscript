

#include <jerry_message.h>
#include <jerry_buffer.h>

static jerry_value_t js_message_obj;
static struct js_callback *js_message_cb = NULL;

struct js_message
{
    char *name;
    rt_uint8_t *data;
    rt_uint16_t size;
};

rt_bool_t js_message_send(const char *name, rt_uint8_t *data, rt_uint16_t size)
{
    rt_bool_t ret = RT_FALSE;

    if (js_message_cb)
    {
        struct js_message *msg = rt_malloc(sizeof(struct js_message));
        if (msg)
        {
            msg->name = rt_strdup(name);
            msg->size = size;
            msg->data = rt_malloc(msg->size);
            if (msg->data)
            {
                rt_memcpy(msg->data, data, size);
                ret = js_send_callback(js_message_cb, msg, sizeof(struct js_message));
            }
            else
            {
                rt_free(msg);
            }
        }
    }

    return ret;
}

static void js_callback_message(const void *args, uint32_t size)
{
    struct js_message *msg = (struct js_message *)args;
    if (msg)
    {
        js_buffer_t *buffer;
        jerry_value_t buf_obj = jerry_buffer_create(msg->size, &buffer);
        if (buffer)
        {
            if (buffer->bufsize > 0)
            {
                js_set_property(buf_obj, "length", jerry_create_number(buffer->bufsize));
                rt_memcpy(buffer->buffer, msg->data, msg->size);
                js_emit_event(js_message_obj, msg->name, &buf_obj, 1);
            }
        }
        /* release value */
        jerry_release_value(buf_obj);
        rt_free(msg->name);
        rt_free(msg->data);
        rt_free(msg);
    }
}

DECLARE_HANDLER(Message)
{
    js_message_obj = jerry_create_object();
    js_make_emitter(js_message_obj, jerry_create_undefined());
    js_message_cb = js_add_callback(js_callback_message);
    if (js_message_cb)
    {
        return jerry_acquire_value(js_message_obj);
    }
    else
    {
        jerry_release_value(js_message_obj);
        return jerry_create_undefined();
    }
}

int js_message_init(void)
{
    REGISTER_HANDLER(Message);

    return 0;
}

int js_message_deinit(void)
{
    if (js_message_cb)
    {
        jerry_release_value(js_message_obj);
        js_message_cb = NULL;
    }

    return 0;
}

rt_bool_t js_msg_test(void)
{
    char *data = "jerry message test!!!";
    return js_message_send("test", (rt_uint8_t *)data, rt_strlen(data));
}
MSH_CMD_EXPORT(js_msg_test, jerry message test);
