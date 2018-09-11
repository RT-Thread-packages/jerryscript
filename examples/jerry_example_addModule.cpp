#include <rtthread.h>
#include <jerry_util.h>

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

const static jerry_object_native_info_t circle_info =
{
    circle_free_callback
};

DECLARE_HANDLER(getCircumference)
{
    void* native_pointer = RT_NULL;
    jerry_get_object_native_pointer(this_value,&native_pointer,RT_NULL);
    if(native_pointer)
    {
        Circle* circle = (Circle*)native_pointer;
        jerry_value_t js_circumference = jerry_create_number(circle->getCircumference());
        return js_circumference;
    }
    return jerry_create_undefined();
};

jerry_value_t Circle_init()
{
    jerry_value_t js_circle = jerry_create_object();
    Circle* circle = new Circle(5);
    jerry_set_object_native_pointer(js_circle,circle,&circle_info);
    jerry_value_t js_radius = jerry_create_number(circle->getRadius());
    js_set_property(js_circle,strdup("radius"),js_radius);
    jerry_release_value(js_radius);
    REGISTER_METHOD(js_circle,getCircumference);
    return js_circle;
}

JS_MODULE(Circle,Circle_init);