
#include "js_request_init.h"

void _js_callback_func_request(const void *args, uint32_t size)
{
	struct callback_info* cb_info = (struct callback_info*)args;

    if(cb_info->return_value != NULL)
        js_emit_event(cb_info->target_value, cb_info->callback_name, &cb_info->return_value, 1);
    else
        js_emit_event(cb_info->target_value, cb_info->callback_name, NULL, 0);
    if(cb_info->return_value != RT_NULL)
        jerry_release_value(cb_info->return_value);
    if(cb_info->data_value != RT_NULL)
        jerry_release_value(cb_info->data_value);
    free(cb_info->callback_name);
    free(cb_info);
}


void _js_callback_free_request(const void *args, uint32_t size)
{
    struct read_paramater* rp = (struct read_paramater*)args;
    webclient_close(rp->session);
	js_remove_callback(rp->request_callback);
	js_remove_callback(rp->close_callback);
    if(rp->isConnected == false)
        jerry_release_value(rp->target_value);
    free(rp);
}


bool combine_header(struct webclient_session* session,char* host,char* user_agent,char* content_type)
{
    bool ret = false;
    if(host!=NULL)
    {
        webclient_header_fields_add(session,"Host: %s\r\n",host);
        ret = true;
    }
    if(user_agent!=NULL)
    {
        webclient_header_fields_add(session,"User-Agent: %s\r\n",user_agent);
        ret = true;
    }
    if(content_type!=NULL)
    {
        webclient_header_fields_add(session,"Content-Type: %s\r\n",content_type);
        ret = true;
    }
    return ret;
}

void create_return_header_property (struct webclient_session* session,jerry_value_t header_value)
{
    int enter_index = 0;
    int colon_index = 0;
    int per_enter_index = -1;
    char header_type[64];
    char header_info[128];
    for(int i = 0 ; i < session->header->length ; i++)
    {
        if(session->header->buffer[i] == ':'
            && per_enter_index != enter_index)
        {
            colon_index = i;
            per_enter_index = enter_index;
            memset(header_type,0,64);
            memset(header_info,0,128);
            strncpy(header_type,session->header->buffer+enter_index+1,colon_index-enter_index-1);
            strcpy(header_info,session->header->buffer+colon_index+2);
            jerry_value_t header_info_value = jerry_create_string((jerry_char_t*)header_info);
            js_set_property(header_value,header_type,header_info_value);
            jerry_release_value(header_info_value);
        }
        if(session->header->buffer[i] == '\0')
            enter_index= i;
    }
}

bool get_header(struct webclient_session* session,jerry_value_t header_value)
{
    if(jerry_value_is_object(header_value))
    {
        char *host=NULL,*user_agent=NULL,*content_type=NULL;
        
        jerry_value_t Host_name = jerry_create_string ((const jerry_char_t *) "Host");
        jerry_value_t Host_value = jerry_get_property (header_value, Host_name);
        if(!jerry_value_is_undefined(Host_value))
            host= js_value_to_string(Host_value);
        jerry_release_value(Host_name);
        jerry_release_value(Host_value);
        
        jerry_value_t User_Agent_name = jerry_create_string ((const jerry_char_t *) "User-Agent");
        jerry_value_t User_Agent_value = jerry_get_property (header_value, User_Agent_name);
        if(!jerry_value_is_undefined(User_Agent_value))
            user_agent = js_value_to_string(User_Agent_value);
        jerry_release_value(User_Agent_name);
        jerry_release_value(User_Agent_value);
        
        jerry_value_t Content_Type_name = jerry_create_string ((const jerry_char_t *) "Content-Type");
        jerry_value_t Content_Type_value = jerry_get_property (header_value, Content_Type_name);
        if(!jerry_value_is_undefined(Content_Type_value))
            content_type = js_value_to_string(Content_Type_value);
        jerry_release_value(Content_Type_name);
        jerry_release_value(Content_Type_value);
        
        return combine_header(session, host, user_agent, content_type);
    }else{
        return false;
    }
}

static void read_request_entry(void* p)
{
    struct read_paramater* rp = (struct read_paramater*)p;

    unsigned char *buffer = RT_NULL;
    buffer = (unsigned char *)malloc(READ_MAX_SIZE+1);
    if(!buffer)
    {
        rt_kprintf("no more memory to create read buffer\n");
        return;
    }
    memset(buffer,0,READ_MAX_SIZE+1);

    //create a callback function to free manager and close webClient session
    int ret_read = 0;
    ret_read=webclient_read(rp->session,buffer,READ_MAX_SIZE+1);
    //If file's size is bigger than buffer's, 
    //give up reading and send a fail callback
    if(ret_read > READ_MAX_SIZE)
    {
        rt_kprintf("the received message's size is bigger than buffer's\n");

        struct callback_info *fail_info =(struct callback_info*)malloc(sizeof(struct callback_info));
        fail_info->target_value = rp->target_value;
        fail_info->callback_name = rt_strdup("fail");
        fail_info->return_value = NULL;
        js_send_callback(rp->request_callback, fail_info, sizeof(struct callback_info));
    }
    else
    {
        jerry_value_t return_value = jerry_create_object();

        /*  create the jerry_value_t of res  */
        js_buffer_t * js_buffer;
        jerry_value_t data_value = jerry_buffer_create(ret_read, &js_buffer);
        if(js_buffer)
        {
            js_set_property(data_value, "length", jerry_create_number(js_buffer->bufsize));
            rt_memcpy(js_buffer->buffer,buffer,ret_read);
        }

        js_set_property(return_value,"data",data_value);
        jerry_value_t statusCode_value = jerry_create_number(webclient_resp_status_get(rp->session));
        js_set_property(return_value,"statusCode",statusCode_value);
        jerry_release_value(statusCode_value);
        
        /*** header's data saved as Object ***/
        jerry_value_t header_value = jerry_create_object();
        create_return_header_property(rp->session,header_value);
        js_set_property(return_value,"header",header_value);        
        jerry_release_value(header_value);
        
        //do success callback
        struct callback_info* success_info  =(struct callback_info*)malloc(sizeof(struct callback_info));
        memset(success_info,0,sizeof(struct callback_info));
        success_info->target_value =rp->target_value;
        success_info->callback_name= rt_strdup("success");
        success_info->return_value = return_value;
        success_info->data_value = data_value;
		js_send_callback(rp->request_callback, success_info, sizeof(struct callback_info));     
    }
    
    //do complete callback
    struct callback_info* complete_info  =(struct callback_info*)malloc(sizeof(struct callback_info));;
    complete_info->target_value = rp->target_value;
    complete_info->callback_name = rt_strdup("complete");
    complete_info->return_value = RT_NULL;
    complete_info->data_value = RT_NULL;
	js_send_callback(rp->request_callback, complete_info, sizeof(struct callback_info));

    free(buffer);
	rp->isConnected = true;
	js_send_callback(rp->close_callback, rp, sizeof(struct read_paramater));
}


DECLARE_HANDLER(request)
{
    if (args_cnt != 1 || !jerry_value_is_object(args[0]))
        return jerry_create_undefined();
    jerry_value_t requestObj = args[0];
    jerry_value_t rqObj = jerry_create_object();
    js_make_emitter(rqObj, jerry_create_undefined());
    struct js_callback* request_callback = js_add_callback(_js_callback_func_request);
    struct js_callback* close_callback = js_add_callback(_js_callback_free_request);

    jerry_value_t success_func = js_get_property(requestObj, "success");
    if(jerry_value_is_function(success_func))
        js_add_event_listener(rqObj, "success", success_func);
    jerry_release_value(success_func);

    jerry_value_t fail_func = js_get_property(requestObj, "fail");
    if(jerry_value_is_function(fail_func))
        js_add_event_listener(rqObj, "fail", fail_func);
    jerry_release_value(fail_func);
    
    jerry_value_t complete_func = js_get_property(requestObj, "complete");
    if(jerry_value_is_function(complete_func))
        js_add_event_listener(rqObj, "complete", complete_func);
    jerry_release_value(complete_func);
 
    char* url = RT_NULL;
    jerry_value_t js_url = js_get_property(requestObj, "url");
    if (jerry_value_is_string(js_url))
        url = js_value_to_string(js_url);
    jerry_release_value(js_url);
    
    char *data = RT_NULL;
    jerry_value_t js_data = js_get_property(requestObj, "data");
    if(jerry_value_is_object(js_data))
    {
        jerry_value_t stringified = jerry_json_stringfy(js_data);
        data = js_value_to_string(stringified);
        jerry_release_value(stringified);
    }
    else if(jerry_value_is_string(js_data))
    {
        data = js_value_to_string(js_data);
    }
    jerry_release_value(js_data);
    
    struct webclient_session *session = webclient_session_create(HEADER_BUFSZ);;
    jerry_value_t js_header = js_get_property(requestObj, "header");
    get_header(session, js_header);
    jerry_release_value(js_header);
    
    int method = WEBCLIENT_GET;
    jerry_value_t method_value = js_get_property(requestObj, "method");  // get/set the value of method
    if(jerry_value_is_string(method_value))
    {
        char *method_str = js_value_to_string(method_value);
        if (method_str)
        {
            if (strcmp(method_str, "POST") == 0)
            {
                method = WEBCLIENT_POST;
            }
            free(method_str);
        }
    }
    jerry_release_value(method_value);

    int response = 0;
    if(session != RT_NULL && method == WEBCLIENT_GET)
    {
        response = webclient_get(session,url);
    }
    else if(session != RT_NULL && method == WEBCLIENT_POST)
    {
        response = webclient_post(session,url,data);
    }
    
    free(data);

    struct read_paramater* rp = (struct read_paramater*)malloc(sizeof(struct read_paramater));
    memset(rp,0,sizeof(struct read_paramater));
    rp->request_callback = request_callback;
    rp->close_callback = close_callback;
    rp->target_value = rqObj;
    rp->session = session;
    
    if(session == RT_NULL || response != 200)
    {
        //do fail callback
        struct callback_info* fail_info  =(struct callback_info*)malloc(sizeof(struct callback_info));;
        fail_info->target_value = rqObj;
        fail_info->callback_name = rt_strdup("fail");
        fail_info->return_value = RT_NULL;
        fail_info->data_value = RT_NULL;
        js_send_callback(request_callback, fail_info, sizeof(struct callback_info));
        //do complete callback
        struct callback_info* complete_info =(struct callback_info*)malloc(sizeof(struct callback_info));;
        complete_info->target_value = rqObj;
        complete_info->callback_name = rt_strdup("complete");        
        complete_info->return_value = RT_NULL;
        complete_info->data_value = RT_NULL;
        js_send_callback(request_callback, complete_info, sizeof(struct callback_info));;
        rp->isConnected =false;
        goto __exit;
    }
    

    rt_thread_t read_request = rt_thread_create("requestRead", read_request_entry, rp, 1536, 20, 5);
    rt_thread_startup(read_request);
    return jerry_acquire_value(rqObj);

    __exit:
        js_send_callback(close_callback,rp,sizeof(struct read_paramater));
        return jerry_create_undefined();
}

jerry_value_t jerry_request_init()
{
    jerry_value_t js_requset = jerry_create_object();
    
    REGISTER_METHOD_NAME(js_requset,"request",request);
    
    return jerry_acquire_value(js_requset);
}

JS_MODULE(http,jerry_request_init)