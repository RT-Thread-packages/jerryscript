/*
* File      : pm_class.cpp
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2009-2018 RT-Thread Develop Team
*/

#include <rtthread.h>
#include "pm_class.h"

PMJS::PMJS()
{
    rt_kprintf("\n print in helPMJSlo() function! \n");
    id = 100;

    text = NULL;
}

PMJS::PMJS(int id)
{
    rt_kprintf("\n print in PMJS(int id) function, id = %d \n", id);
    this->id = id;

    text = NULL;
    rt_kprintf("end PMJS(int id) %d \n", this->id);
}

PMJS::~PMJS()
{
    if (NULL != text)
        rt_free(text);
    text = NULL;

}

int PMJS::getId(void)
{
    rt_kprintf("\n print in getId() function! \n");
    rt_kprintf("id = %d \n", this->id);
    return this->id;
}

void PMJS::setId(int id)
{
    rt_kprintf("\n print in setId(int id) function, in param id = %d \n", id);
    this->id = id;
    rt_kprintf("id = %d \n", this->id);
}
