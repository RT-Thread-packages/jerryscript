/*
* File      : pm_class.h
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2009-2018 RT-Thread Develop Team
*/

#ifndef __PM_CLASS_H__
#define __PM_CLASS_H__

#include <rtthread.h>

#include <string.h>

class PMJS
{

public:
	PMJS();
    PMJS(int id);
    virtual ~PMJS();

    void setId(int id);
    int getId(void);

    void setText(char *str)
    {
        if (NULL != text)
            rt_free(text);
        text = (char *)rt_calloc(1, sizeof(char) * strlen(str));
    }
    char* getText(void)
    {
        return text;
    }

private:
    int id;
    char *text;

};

extern "C" {
    // #include "jerry_extapi.h"
void registry_pm_class();
void registry_global_Window();
}

#endif