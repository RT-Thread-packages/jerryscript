# jerryscript on RT-Thread

JerryScript is a lightweight JavaScript engine for resource-constrained devices such as microcontrollers. It can run on devices with less than 64 KB of RAM and less than 200 KB of flash memory.

API Reference ：http://jerryscript.net/api-reference/

首先，不得不说明下JerryScript的基础类型：`jerry_value_t`。一个对象可以设定任意个属性，属性可以是`string`,`function`等，也可以是对象。但每个对象中，都有一个指针：`native_pointer`，这个指针就可用来存放或传递数据。

    jerry_value_t obj
        ├── 属性1
        ├── 属性2
        │   ...
        │   ...
        └── native_pointer    


JerryScript中则提供了获取、设定native_pointer的API。
```C
bool
jerry_get_object_native_pointer (const jerry_value_t obj_val,
                                 void **out_native_pointer_p,
                                 const jerry_object_native_info_t **out_native_info_p)

bool
jerry_set_object_native_pointer (const jerry_value_t obj_val,
                                 void *native_p,
                                 const jerry_object_native_info_t *info_p)
```

## 如何导入C++的API

此Package中，我们一般使用以下的宏在JerryScript中创建一些接口，来使用C++的API：

```C
#define REGISTER_HANDLER(NAME)      \
    jerryx_handler_register_global ( (jerry_char_t *)# NAME, NAME ## _handler)
#define REGISTER_HANDLER_ALIAS(NAME, HANDLER) \
    jerryx_handler_register_global ( (jerry_char_t *)# NAME, HANDLER)
#define REGISTER_METHOD(OBJ, NAME)  \
    js_add_function (OBJ, # NAME, NAME ## _handler)
#define REGISTER_METHOD_ALIAS(OBJ, NAME, HANDLER) \
    js_add_function (OBJ, # NAME, HANDLER)
#define REGISTER_METHOD_NAME(OBJ, NAME, HANDLER)  \
    js_add_function (OBJ, NAME, HANDLER ## _handler)

```

可以看到，前两个宏指向函数`jerryx_handler_register_global`,具体内容如下：

```C
jerry_value_t
jerryx_handler_register_global (const jerry_char_t *name_p, /**< name of the function */
                                jerry_external_handler_t handler_p) /**< function callback */
{
  jerry_value_t global_obj_val = jerry_get_global_object ();
  jerry_value_t function_name_val = jerry_create_string (name_p);
  jerry_value_t function_val = jerry_create_external_function (handler_p);

  jerry_value_t result_val = jerry_set_property (global_obj_val, function_name_val, function_val);

  jerry_release_value (function_val);
  jerry_release_value (function_name_val);
  jerry_release_value (global_obj_val);

  return result_val;
} 
```
后三个宏定义都指向了函数`js_add_function`,具体内容如下：
```C
void js_add_function(const jerry_value_t obj, const char *name,
                     jerry_external_handler_t func)
{
    jerry_value_t str = jerry_create_string((const jerry_char_t *)name);
    jerry_value_t jfunc = jerry_create_external_function(func);

    jerry_set_property(obj, str, jfunc);

    jerry_release_value(str);
    jerry_release_value(jfunc);
}
```

从上述代码中可以看到`jerryx_handler_register_global`和`js_add_function`的实质操作就是在指定对象中加入一个名为`name`的`func`，这个`obj`可以指定为全局obj。而我们就只需要在这个func中添加C++的代码并使用API，并且可以把所需要的数据作为属性加入到指定JerryScript对象中或返回出来。如果想要在JS中传递C++对象，我们就可以利用文章开头所说的native_pointer

此func类型为`jerry_external_handler_t`，具体定义如下。
```C
typedef jerry_value_t (*jerry_external_handler_t) (const jerry_value_t function_obj,
                                                   const jerry_value_t this_val,
                                                   const jerry_value_t args_p[],
                                                   const jerry_length_t args_count);
```
为了方便书写代码，Package中添加了对应的宏定义。
```C
#define DECLARE_HANDLER(NAME)       \
    static jerry_value_t            \
    NAME ## _handler (const jerry_value_t func_value,   \
                        const jerry_value_t this_value, \
                        const jerry_value_t args[],     \
                        const jerry_length_t args_cnt )
```

### 示例代码

以添加`request`接口为例
1. 创建一个初始化API：`js_request_init`，并设定入参变量`obj`。创建好初始化API后，我们就只要在API中执行`REGISTER_METHOD(obj, request)`，但此时函数`request`还未创建，那我们只需要创建它即可。而函数`request`内部，我们就可以添加C++的API，执行我们需要操作或把相关数据转换成JerryScript的数据类型返回出来。

    ```C
    DECLARE_HANDLER(request)
    {
        //此处就能添加对应的C++的API，并执行具体内容。
    }

    extern "C"
    {
        int js_request_init(jerry_value_t obj)
        {
            REGISTER_METHOD(obj, request);
            return 0;
        }
    }
    ```

2. 譬如在此Package中，JerryScript的执行函数为`jerry_main`,那我们就还需要在`jerry_main`中执行下`js_request_init`。

    ```C
    extern int js_request_init(jerry_value_t obj);
    int jerry_main(int argc, char** argv)
    {
        //...
        jerry_value_t global_object = jerry_get_global_object();
        js_request_init(global_object);
        //...
    }
    ```

## 如何添加builtin module

由于JerryScript是一款轻量级的JavaScript的引擎，因此它没有直接提供常见的module。但是开发者可以根据自身需求，去添加相应module。

那如何添加builtin module呢？当相应module的入口建立好后，只需要使用以下finsh命令即可
```
//NAME ： module的名称
//MODULE_INIT ： module的入口函数
JS_MODULE(NAME, MODULE_INIT)
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
```C
var mqtt = require('myModule')
```

## Sample

### C++ 导入示例

```C++
#include <rtthread.h>
#include <jerry_util.h>

/*自定义C++类*/
class Rectangle
{
   private:
        int length;
        int width;
   
   public:
        Rectangle(int length,int width);
        int getSize();
        int getLength();
        int getWidth();
};

Rectangle::Rectangle(int length,int width)
{
    this->length = length;
    this->width = width;
}

int Rectangle::getSize()
{
    return (this->length * this->width);
}

int Rectangle::getLength()
{
    return this->length;
}

int Rectangle::getWidth()
{
    return this->width;
}
/**********/

void rectangle_free_callback(void *native_p)
{
    Rectangle* rect = (Rectangle*)native_p;
    delete(rect);
}

const static jerry_object_native_info_t rectangle_info =
{
    rectangle_free_callback //当js_rect对象释放后，调用该回调
};

DECLARE_HANDLER(getSize)
{
    void *native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);    //获取当前对象中的native_pointer
    
    if(native_pointer)
    {
        /*调用C++对象的API，并将其返回值转换成JS对象*/
        Rectangle* rectangle = (Rectangle*)native_pointer;
        jerry_value_t js_size = jerry_create_number(rectangle->getSize());
        return js_size;
    }
    return jerry_create_undefined();
}

DECLARE_HANDLER(getLength)
{
    void *native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);
    
    if(native_pointer)
    {
        Rectangle* rectangle = (Rectangle*)native_pointer;
        jerry_value_t js_length = jerry_create_number(rectangle->getLength());
        return js_length;
    }
    return jerry_create_undefined();
}

DECLARE_HANDLER(getWidth)
{
    void *native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);
    
    if(native_pointer)
    {
        Rectangle* rectangle = (Rectangle*)native_pointer;
        jerry_value_t js_width = jerry_create_number(rectangle->getWidth());
        return js_width;
    }
    return jerry_create_undefined();
}

DECLARE_HANDLER(rectangle_init)
{
    /*入参判断*/
    if(args_cnt !=2 || !jerry_value_is_number(args[0]) || !jerry_value_is_number(args[1]))
        return jerry_create_undefined();
    
    jerry_value_t js_rect = jerry_create_object(); //创建JS对象 js_rect
    
    Rectangle* rectangle = new Rectangle(jerry_get_number_value(args[0]),jerry_get_number_value(args[1]));      /创建C++对象 rectangle

    jerry_set_object_native_pointer(js_rect, rectangle,&rectangle_info);    //把js_rect的native_pointer指向rectangle
    
    /*调用rectangle的API，并把其返回值转换成JS对象*/
    jerry_value_t js_length = jerry_create_number(rectangle->getLength());      
    jerry_value_t js_width = jerry_create_number(rectangle->getWidth());
    
    /*把js_length 和 js_width设置成js_rect的属性，类Rectangle中lenght与width为私有变量，如要JS对象与C++对象保持对应，此处代码可注释掉*/
    js_set_property(js_rect,"length",js_length);
    js_set_property(js_rect,"width",js_width);
    
    jerry_release_value(js_length);
    jerry_release_value(js_width);
    
    REGISTER_METHOD(js_rect,getSize);       //为js_retc注册一个方法getSize
    REGISTER_METHOD(js_rect,getLength);     //为js_retc注册一个方法getLength
    REGISTER_METHOD(js_rect,getWidth);      //为js_retc注册一个方法getWidth
    return js_rect;
}

extern "C"
{
    int js_example_rect_init(jerry_value_t obj)
    {
        REGISTER_METHOD(obj, rectangle_init); //为指定对象obj注册一个方法rectangle_init，这个对象可以是global_obj
        return 0;
    }
}

```

### builtin module示例

```C++

#include <rtthread.h>
#include <jerry_util.h>

/*自定义的C++类*/
class Circle 
{
    private:
        int radius ;
    public:
        Circle(int radius);
        int getRadius();
        float getCircumference();
};

Circle::Circle(int radius)
{
    this->radius = radius;
}

int Circle::getRadius()
{
    return this->radius;
}

float Circle::getCircumference()
{
    return (2 * 3.1415 *this->radius);
}

void circle_free_callback(void *native_p)
{
    Circle* rect = (Circle*)native_p;
    delete(rect);
}
/**********/

const static jerry_object_native_info_t circle_info =
{
    circle_free_callback //当js_circle对象释放后，调用该回调
};

DECLARE_HANDLER(getCircumference)
{
    /*函数内部具体实现*/
    void* native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);    //获取当前对象中的native_pointer
    if(native_pointer)
    {
        /*调用C++对象的API，并将其返回值转换成JS对象*/
        Circle* circle = (Circle*)native_pointer;
        jerry_value_t js_circumference = jerry_create_number(circle->getCircumference());
        return js_circumference;
    }
    return jerry_create_undefined();
};

jerry_value_t Circle_init()
{
    jerry_value_t js_circle = jerry_create_object();   // 创建JerryScript对象js_circle
    Circle* circle = new Circle(5);     //创建C++对象 Circle
    jerry_set_object_native_pointer(js_circle,circle,&circle_info);     //把js_circle的native_pointer指向circle
    jerry_value_t js_radius = jerry_create_number(circle->getRadius());     //调用Circle的API，并把它转换成JS对象。
    js_set_property(js_circle,strdup("radius"),js_radius);  //把js_radius设置成js_circle的一个属性
    jerry_release_value(js_radius);
    REGISTER_METHOD(js_circle,getCircumference);    //为js_circle注册一个方法 getCircumference
    return js_circle;
}

JS_MODULE(Circle,Circle_init); //注册builtin module，名称：Circle,入口函数：Circle_init
```