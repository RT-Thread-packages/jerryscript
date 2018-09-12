#include <rtthread.h>
#include <jerry_util.h>

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

void rectangle_free_callback(void *native_p)
{
    Rectangle* rect = (Rectangle*)native_p;
    delete(rect);
}

const static jerry_object_native_info_t rectangle_info =
{
    rectangle_free_callback
};

DECLARE_HANDLER(getSize)
{
    void *native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);
    
    if(native_pointer)
    {
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

DECLARE_HANDLER(rectangle)
{   
    if(args_cnt !=2 || !jerry_value_is_number(args[0]) || !jerry_value_is_number(args[1]))
        return jerry_create_undefined();
    
    jerry_value_t js_rect = jerry_create_object();
    
    Rectangle* rectangle = new Rectangle(jerry_get_number_value(args[0]),jerry_get_number_value(args[1]));
    jerry_set_object_native_pointer(js_rect, rectangle,&rectangle_info);
    
    jerry_value_t js_length = jerry_create_number(rectangle->getLength());
    jerry_value_t js_width = jerry_create_number(rectangle->getWidth());
    
    js_set_property(js_rect,"length",js_length);
    js_set_property(js_rect,"width",js_width);
    
    jerry_release_value(js_length);
    jerry_release_value(js_width);
    
    REGISTER_METHOD(js_rect,getSize);
    REGISTER_METHOD(js_rect,getLength);
    REGISTER_METHOD(js_rect,getWidth);
    
    return js_rect;
}

extern "C"
{
    int js_example_rect_init(jerry_value_t obj)
    {
        REGISTER_METHOD(obj, rectangle);
        return 0;
    }
}
