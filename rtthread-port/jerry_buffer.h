/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 * Copyright (c) 2016-2018, Intel Corporation.
 *
 * ported from zephyr.js
 * COPYRIGHT (C) 2018, RT-Thread Development Team
 */

#ifndef JERRY_BUFFER_H__
#define JERRY_BUFFER_H__

typedef struct js_buffer
{
    uint8_t *buffer;
    uint32_t bufsize;
} js_buffer_t;

int js_buffer_init(void);
int js_buffer_cleanup(void);

#endif
