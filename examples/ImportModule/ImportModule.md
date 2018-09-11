# 如何添加builtin module

由于JerryScript是一款轻量级的JavaScript的引擎，因此它没有直接提供常见的builtin module。但是开发者可以根据自身需求，去添加相应module。

## 添加builtin module

当相应module的入口建立好后，只需要使用以下finsh命令即可:

```
JS_MODULE(NAME, MODULE_INIT)    //NAME ： module的名称 , MODULE_INIT ： module的入口函数
```

示例代码：
```C
jerry_value_t myModule_init(void)
{

    jerry_value_t obj = jerry_create_object();
    // ...
    // do something
    // ...
    return obj;
}

JS_MODULE(myModule,myModule_init);
```
以上代码就在JerryScript中创建里一个名为`myModule`的module，其返回值为`obj`。

在JS脚本中，我们都是通过以下方式调用module的。

```JavaScript
var mqtt = require('myModule')
```

## 示例说明

本示例示范了如何添加builtin module。

```C
#include <rtthread.h>
#include <jerry_util.h>

struct circle_t
{
    double radius;
};

void circle_free_callback(void *native_p)
{
    struct circle_t* circle = (struct circle_t*)native_p;   
    free(circle);
}

const static jerry_object_native_info_t circle_info =
{
    circle_free_callback    //当js_circle对象释放后，调用该回调函数
};

DECLARE_HANDLER(setRadius)
{
    /*入参判断*/
    if(args_cnt!=1 || !jerry_value_is_number(args[0]))
        return jerry_create_undefined();
    
    void* native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);    //获取当前对象中的native_pointer
    
    if(native_pointer)
    {
        circle_t* circle = (circle_t*)native_pointer;
        
        double r = jerry_get_number_value(args[0]);
        circle->radius = r;
    }
    
    return jerry_create_undefined();
};

DECLARE_HANDLER(getRadius)
{
    if(args_cnt != 0)
        return jerry_create_undefined();
    
    void* native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);
    
    if(native_pointer)
    {
        circle_t* circle = (circle_t*)native_pointer;
        
        jerry_value_t js_radius = jerry_create_number(circle->radius);  //获取circle->radius，并将其返回值转换成JS对象
        return js_radius;
    }
    
    return jerry_create_undefined();
};

jerry_value_t Circle_init()
{
    jerry_value_t js_circle = jerry_create_object();
    
    circle_t* circle = (circle_t*)malloc(sizeof(circle_t));
    memset(circle,0,sizeof(circle_t));
    
    jerry_set_object_native_pointer(js_circle,circle,&circle_info);     //把js_circle的native_pointer指向circle

    REGISTER_METHOD(js_circle,setRadius);   //为js_circle注册一个方法 setRadius
    REGISTER_METHOD(js_circle,getRadius);   //为js_circle注册一个方法 getRadius
    return js_circle;
}

JS_MODULE(Circle,Circle_init); //注册builtin module，名称：Circle,入口函数：Circle_init
```