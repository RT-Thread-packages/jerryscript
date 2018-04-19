#ifndef JS_BUFFER_H__
#define JS_BUFFER_H__

typedef struct js_buffer
{
    uint8_t *buffer;
    uint32_t bufsize;
} js_buffer_t;

int js_buffer_init(void);
int js_buffer_cleanup(void);

#endif
