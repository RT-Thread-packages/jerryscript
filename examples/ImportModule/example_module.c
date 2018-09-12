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
    circle_free_callback
};

DECLARE_HANDLER(setRadius)
{
    if(args_cnt!=1 || !jerry_value_is_number(args[0]))
        return jerry_create_undefined();
    
    void* native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);
    
    if(native_pointer)
    {
        struct circle_t* circle = (struct circle_t*)native_pointer;
        
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
        struct circle_t* circle = (struct circle_t*)native_pointer;
       
        jerry_value_t js_radius = jerry_create_number(circle->radius);
        return js_radius;
    }
    
    return jerry_create_undefined();
};

jerry_value_t Circle_init()
{
    jerry_value_t js_circle = jerry_create_object();
    
    struct circle_t* circle = (struct circle_t*)malloc(sizeof(struct circle_t));
    memset(circle,0,sizeof(struct circle_t));
    
    jerry_set_object_native_pointer(js_circle,circle,&circle_info);

    REGISTER_METHOD(js_circle,setRadius);
    REGISTER_METHOD(js_circle,getRadius);
    return js_circle;
}

JS_MODULE(Circle,Circle_init);
