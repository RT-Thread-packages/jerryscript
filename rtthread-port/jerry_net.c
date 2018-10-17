#include "jerry_net.h"
/* manage the pipe of socket */
static rt_int32_t pipe_init(struct socket_info *thiz)
{
    char dname[8];
    char dev_name[32];
    static int pipeno = 0;

    if (thiz == RT_NULL)
        return -1;

    snprintf(dname, sizeof(dname), "Socket%d", pipeno++);
    
    thiz->pipe = rt_pipe_create(dname, PIPE_BUFSZ);
    if (thiz->pipe == RT_NULL)
        return -1;
    
    snprintf(dev_name, sizeof(dev_name), "/dev/%s", dname);
    thiz->pipe_read_fd = open(dev_name, O_RDONLY, 0);
    if (thiz->pipe_read_fd < 0)
    {
        rt_kprintf("pipe_read_fd open failed\n");
        return -1;
    }

    thiz->pipe_write_fd = open(dev_name, O_WRONLY, 0);
    if (thiz->pipe_write_fd < 0)
    {
        close(thiz->pipe_read_fd);
        rt_kprintf("pipe_write_fd open failed\n");
        return -1;
    }
    return 0;
}

static rt_int32_t pipe_deinit(struct socket_info *thiz)
{
    if (thiz == RT_NULL)
        return -1;

    close(thiz->pipe_read_fd);
    close(thiz->pipe_write_fd);
    rt_pipe_delete((const char *)thiz->pipe);
    return 0;
}
/* manage the socket_list */
int get_socket_list_count(socket_list** l)
{
    if(*l == RT_NULL)
        return 0;
    int count = 0;   
    socket_list* start = *l;
    do{
        count++;
        *l=(*l)->next;
    }
    while((*l) != start);
    
    return count;
}

void socket_list_init(socket_list** l,jerry_value_t socket)
{
    (*l)->next = (*l)->prev = (*l);
    (*l)->js_socket = socket;
}

void socket_list_insert(socket_list **l,jerry_value_t socket)
{
    if(*l ==  RT_NULL)
    {
        *l = (socket_list*)malloc(sizeof(socket_list));
        socket_list_init(l,socket);
    }else{
        socket_list* n = (socket_list*)malloc(sizeof(socket_list));
        socket_list_init(&n,socket);
        
        (*l)->next->prev = n;
        n->next = (*l)->next;
        (*l)->next = n;
        n->prev =(*l);
    }  
}

void socket_list_remove(socket_list** n)
{
    if((*n)->next == (*n))
    {
        jerry_release_value((*n)->js_socket);
        free((*n));
        (*n) = RT_NULL;
    }else{
        (*n)->next->prev = (*n)->prev;
        (*n)->prev->next = (*n)->next;
        (*n)->next = (*n)->prev = (*n);
        jerry_release_value((*n)->js_socket);
        free(*n);
        (*n) = RT_NULL;
    }
}

bool socket_list_remove_by_socket(socket_list** l ,jerry_value_t socket)
{
    if((*l) == RT_NULL || socket == RT_NULL)
        return false;

    socket_list* start = (*l);
    int index =0;
    do{
        if((*l)->js_socket == socket)
        {
            socket_list* toRemove = *l;
            (*l)=(*l)->next;
            if((*l)!= toRemove)
                socket_list_remove(&toRemove);
            else
                socket_list_remove(l);
            return true;
        }
        (*l)=(*l)->next;
    }
    while((*l) != start);
    return false;
}

/* the callbacks of Net Module */
void get_net_info(void **info,jerry_value_t js_target)
{
    jerry_value_t js_info = js_get_property(js_target,"info");
    jerry_get_object_native_pointer(js_info,info,NULL);
    jerry_release_value(js_info);
}

void js_event_callback_func_net(const void *args, uint32_t size)
{
    struct event_callback_info* cb_info = (struct event_callback_info*)args;

    if(cb_info->js_return != RT_NULL)
        js_emit_event(cb_info->js_target, cb_info->event_name, &cb_info->js_return, 1);
    else
        js_emit_event(cb_info->js_target, cb_info->event_name, NULL, 0);
    if(cb_info->js_return != RT_NULL)
        jerry_release_value(cb_info->js_return);;
    free(cb_info->event_name);
    free(cb_info);
}

void js_callback_func_net(const void *args, uint32_t size)
{
    struct fun_callback_info* cb_info = (struct fun_callback_info*)args;

    jerry_value_t ret = jerry_call_function(cb_info->js_run, cb_info->js_this,(jerry_value_t*)cb_info->args, cb_info->args_cnt);
    
    jerry_release_value(ret);

    free(cb_info);
}
void closeServer(jerry_value_t js_server)
{
    struct server_info* js_server_info = RT_NULL; 
    get_net_info((void**)&js_server_info,js_server);
    /*emit 'close' event*/
    struct event_callback_info* cb_info = (struct event_callback_info*)malloc(sizeof(struct event_callback_info));
    memset(cb_info,0,sizeof(struct event_callback_info));
                    
    cb_info->event_name = strdup("close");
    cb_info->js_target = js_server;
    cb_info->js_return = RT_NULL;
                    
    js_send_callback(js_server_info->event_callback,cb_info,sizeof(struct event_callback_info));
            
    /*do close callback of server*/
    struct close_callback_info* info  = (struct close_callback_info*)malloc(sizeof(struct close_callback_info));
    memset(info, 0, sizeof(struct close_callback_info));
            
    info->js_server = js_server;
    info->js_socket = RT_NULL;
            
    js_send_callback(js_server_info->close_callback,info,sizeof(struct close_callback_info));
}
void js_scoket_close_callback_func_net(const void *args, uint32_t size)
{
    struct close_callback_info* info = (struct close_callback_info*)args;
    
    //GET_SOCKET_INFO(info->js_socket)
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,info->js_socket);
    jerry_value_t js_server = js_socket_info->js_server;
    
    if(js_socket_info->readData_thread != RT_NULL)
        rt_thread_delete(js_socket_info->readData_thread);

    closesocket(js_socket_info->socket_id);
    js_socket_info->socket_id = -1;
    js_remove_callback(js_socket_info->event_callback);
    js_remove_callback(js_socket_info->fun_callback);
    js_remove_callback(js_socket_info->close_callback);
    
    pipe_deinit(js_socket_info);
    rt_sem_delete(js_socket_info->socket_sem);
    
    if(info->js_server != RT_NULL)
    {
        /*get server info*/
        //GET_SERVER_INFO(info->js_server);
        struct server_info* js_server_info = RT_NULL; 
        get_net_info((void**)&js_server_info,js_server);

        rt_sem_take(js_server_info->server_sem,RT_WAITING_FOREVER);
        socket_list_remove_by_socket(&(js_server_info->socket_list),info->js_socket);
        rt_sem_release(js_server_info->server_sem);
        if(js_server_info->server_fd == -1 
            && get_socket_list_count(&(js_server_info->socket_list)) == 0)
            closeServer(js_server);
     }
    jerry_release_value(info->js_socket);
    free(info);
            
}
void js_server_close_callback_func_net(const void *args, uint32_t size)
{
    struct close_callback_info* info = (struct close_callback_info*)args;
    
    //GET_SERVER_INFO(info->js_server);
    struct server_info* js_server_info = RT_NULL; 
    get_net_info((void**)&js_server_info,info->js_server);

    js_remove_callback(js_server_info->event_callback);
    js_remove_callback(js_server_info->fun_callback);
    js_remove_callback(js_server_info->close_callback);
    
    rt_sem_delete(js_server_info->server_sem);
    jerry_release_value(info->js_server);
    free(info);
}
void net_server_free_callback(void *native_p)
{
    struct server_info* server = (struct server_info*)native_p;
    free(server);
}

const static jerry_object_native_info_t net_server_info =
{
    net_server_free_callback
};

void net_socket_free_callback(void *native_p)
{
    struct socket_info* socket = (struct socket_info*)native_p;
    free(socket);
}

const static jerry_object_native_info_t net_socket_info =
{
    net_socket_free_callback
};


/*  the method of socket  */
void readFromSocket(jerry_value_t js_socket)
{
    unsigned char buffer[BUFFER_SIZE];
    memset(&buffer,0,BUFFER_SIZE);
    
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,js_socket);
    int bytesRead = recv(js_socket_info->socket_id, buffer, BUFFER_SIZE+1, 0);
    
    /*set property: data*/
    js_buffer_t * js_buffer;
    jerry_value_t data_value = jerry_buffer_create(bytesRead, &js_buffer);
    if(js_buffer)
    {
        js_set_property(data_value, "length", jerry_create_number(js_buffer->bufsize));
        rt_memcpy(js_buffer->buffer,buffer,bytesRead);
    }
    
    /*emit 'data' event*/
    struct event_callback_info* cb_info = (struct event_callback_info*)malloc(sizeof(struct event_callback_info));
    memset(cb_info,0,sizeof(struct event_callback_info));
    
    cb_info->event_name = strdup("data");
    cb_info->js_target = js_socket;
    cb_info->js_return = data_value;
            
    js_send_callback(js_socket_info->event_callback,cb_info,sizeof(struct event_callback_info));
}
void sendToSocket(struct write_data_info* info,jerry_value_t js_socket)
{
    unsigned char buffer[BUFFER_SIZE];
    memset(&buffer,0,BUFFER_SIZE);
    /*send the data*/
    int hasSent = 0;
    
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,js_socket);
    
    js_buffer_t * js_buffer =jerry_buffer_find(info->js_data);

    if(js_buffer)
    {
        for(int i =0,bytes = js_buffer->bufsize;bytes>0;i++,bytes -=BUFFER_SIZE)
        {
            memset(&buffer,0,BUFFER_SIZE);
                
            if(bytes > BUFFER_SIZE)
            {
                memcpy(buffer,js_buffer->buffer+i*BUFFER_SIZE,BUFFER_SIZE);
                hasSent+=send(js_socket_info->socket_id, buffer, BUFFER_SIZE,0);
            }else
            {
                memcpy(buffer,js_buffer->buffer+i*BUFFER_SIZE,bytes);
                hasSent+=send(js_socket_info->socket_id, buffer, bytes,0);
            }
        }
    }else if(jerry_value_is_string(info->js_data))
    {
        char* js_str=js_value_to_string(info->js_data);
        for(int i =0,bytes = strlen(js_str);bytes>0;i++,bytes -=BUFFER_SIZE)
        {
            memset(&buffer,0,BUFFER_SIZE);
            
            if(bytes > BUFFER_SIZE)
            {
                memcpy(buffer,js_str+i*BUFFER_SIZE,BUFFER_SIZE);
                hasSent+=send(js_socket_info->socket_id, buffer, BUFFER_SIZE,0);
            }else
            {
                memcpy(buffer,js_str+i*BUFFER_SIZE,bytes);
                hasSent+=send(js_socket_info->socket_id, buffer, bytes,0);
            }        
        }
        free(js_str);
    }
    
    /*do callback*/
    if(hasSent && info->js_callback!=RT_NULL)
    {
        struct fun_callback_info* cb_info = (struct fun_callback_info*)malloc(sizeof(struct fun_callback_info));
        memset(cb_info,0,sizeof(struct fun_callback_info));
        
        cb_info->js_run = info->js_callback;
        cb_info->js_this =  js_socket;
        cb_info->args = RT_NULL;
        cb_info->args_cnt = 0;

        js_send_callback(js_socket_info->fun_callback,cb_info,sizeof(struct fun_callback_info));
    }
}
static void socket_readData_entry(void* p)
{
    jerry_value_t js_socket = ((struct read_thread_info*)p)->js_socket;
    free(p);

    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,js_socket);
    int socket = js_socket_info->socket_id;
    int pipe_read_fd = js_socket_info->pipe_read_fd;
    
    int max_fd = 0;
    if(socket > pipe_read_fd)
        max_fd = socket+1;
    else
        max_fd = pipe_read_fd+1;
    
    fd_set readfds;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(socket, &readfds);
        FD_SET(pipe_read_fd, &readfds);
        
        if(js_socket_info->timeout.tv_sec ==0 
            && js_socket_info->timeout.tv_usec ==0 )
            select(max_fd,&readfds,0,0,0);
        else{
            if(select(max_fd,&readfds,0,0, &js_socket_info->timeout)==0) 
            {
                /*emit 'timeout' event*/
                struct event_callback_info* cb_info = (struct event_callback_info*)malloc(sizeof(struct event_callback_info));
                memset(cb_info,0,sizeof(struct event_callback_info));
                    
                cb_info->event_name = strdup("timeout");
                cb_info->js_target = js_socket;
                cb_info->js_return = RT_NULL;
                    
                js_send_callback(js_socket_info->event_callback,cb_info,sizeof(struct event_callback_info));
            }
        }
        if(FD_ISSET(socket,&readfds)
            && js_socket_info->readable == true) 
            readFromSocket(js_socket);
        
        if(FD_ISSET(pipe_read_fd,&readfds))
        {
            rt_sem_take(js_socket_info->socket_sem,RT_WAITING_FOREVER);
            //read from pipe
            static struct write_data_info info;
            memset(&info,0,sizeof(struct write_data_info));
            int bytesRead = read(js_socket_info->pipe_read_fd,&info,sizeof(struct write_data_info));
            sendToSocket(&info,js_socket);
            rt_sem_release(js_socket_info->socket_sem);
        }
    }
}
/*update property : remoteAddress ,remoteFamily,remotePort,localAddress and localPort
start the read_thread of socket */
int socket_updateProperty_and_startRead(jerry_value_t this_value)
{    
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id <0)
        return 0;
    struct sockaddr_in remote_name; //get the infomation of remote address
    socklen_t sin_size = sizeof(struct sockaddr_in);
    getpeername(js_socket_info->socket_id, (struct sockaddr *)&remote_name,&sin_size);
            
    jerry_value_t js_remoteFamily;
    if(remote_name.sin_family == AF_INET)
    {
        js_remoteFamily = jerry_create_string((const jerry_char_t*)"IPv4");
    }else{
        js_remoteFamily = jerry_create_string((const jerry_char_t*)"IPv6");
    }
    js_set_property(this_value,"remoteFamily",js_remoteFamily);
    jerry_release_value(js_remoteFamily);
            
    jerry_value_t js_remotePort = jerry_create_number(ntohs(remote_name.sin_port));
    js_set_property(this_value,"remotePort",js_remotePort);
    jerry_release_value(js_remotePort);

    jerry_value_t js_remoteAddress = jerry_create_string((jerry_char_t *)inet_ntoa(remote_name.sin_addr));
    js_set_property(this_value,"remoteAddress",js_remoteAddress);
    jerry_release_value(js_remoteAddress);
    /*update property : localAddress and localPort*/
            
    struct sockaddr_in local_name; //get the infomation of local address
    getsockname(js_socket_info->socket_id, (struct sockaddr *)&local_name,&sin_size);
            
    jerry_value_t js_localAddress = jerry_create_string((jerry_char_t *)inet_ntoa(local_name.sin_addr));
    js_set_property(this_value,"localAddress",js_localAddress);
    jerry_release_value(js_localAddress);
            
    jerry_value_t js_localPort = jerry_create_number(ntohs(local_name.sin_port));
    js_set_property(this_value,"localPort",js_localPort);
    jerry_release_value(js_localPort);
    
    /*create a thread to read data from server*/
    if(js_socket_info->allowHalfOpen == false)
    {
        struct read_thread_info* info = (struct read_thread_info*)malloc(sizeof(struct read_thread_info));
        memset(info,0,sizeof(struct read_thread_info));
        info->js_socket = this_value;
        js_socket_info->readData_thread = rt_thread_create("socket_Read", socket_readData_entry, info, 2048, 20, 5);
        rt_thread_startup(js_socket_info->readData_thread);
    }
    /*emit 'connect' event*/
    struct event_callback_info* cb_info = (struct event_callback_info*)malloc(sizeof(struct event_callback_info));
    memset(cb_info,0,sizeof(struct event_callback_info));
            
    cb_info->event_name = strdup("connect");
    cb_info->js_target = this_value;
    cb_info->js_return = RT_NULL;
    js_send_callback(js_socket_info->event_callback,cb_info,sizeof(struct event_callback_info));
    return 0;
}

DECLARE_HANDLER(socket_connect)
{
    if(args_cnt==0||args_cnt>3)
        return jerry_create_undefined();

    /*relative informations*/
    int port = -1;
    char* host = RT_NULL;
    int family = 4;
    
    jerry_value_t js_connect_cb = RT_NULL;
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(jerry_value_is_object(args[0]))
    {
        jerry_value_t js_port = js_get_property(args[0],"port");
        if(jerry_value_is_number(js_port))
            port = jerry_get_number_value(js_port);
        jerry_release_value(js_port);
        
        jerry_value_t js_host = js_get_property(args[0],"host");
        if(jerry_value_is_string(js_host))
            host = js_value_to_string(js_host);
        jerry_release_value(js_host);

        jerry_value_t js_family = js_get_property(args[0],"family");
        if(jerry_value_is_number(js_family))
            family = jerry_get_number_value(js_family);
        jerry_release_value(js_family);
    }
    
    if(jerry_value_is_number(args[0]))
        port = jerry_get_number_value(args[0]);

    if(args_cnt >1 && jerry_value_is_string(args[1]))
        host = js_value_to_string(args[1]);

    if(args_cnt >1 && jerry_value_is_function(args[1]))
        js_connect_cb = args[1];
    else if(args_cnt >2 && jerry_value_is_function(args[2]))
        js_connect_cb = args[2];

    /*add connect event listener*/
    if(js_connect_cb!= RT_NULL)
        js_add_event_listener(this_value,"connect",js_connect_cb);

    /*create socket*/
    if(js_socket_info->socket_id == -1 )
    {
        if(family ==4)
            js_socket_info->socket_id = socket(PF_INET,SOCK_STREAM,0);
        #if LWIP_IPV6
        else if(family ==6)
            js_socket_info->server_socket = socket(PF_INET6,SOCK_STREAM,0);
        #endif
        else
        {
            rt_kprintf("the paramater named 'family' is wrong\n");
            return jerry_create_undefined();
        }
    }else
        goto __isConnected;

    /*read to connect*/
    if(port > 0)
    {        
        struct sockaddr_in socket_fd;

        socket_fd.sin_family = AF_INET;

        #if LWIP_IPV6
        if(family == 6)
            socket_fd.sin_family = AF_INET6;
        #endif

        struct hostent *socket_host;
        socket_host = gethostbyname(host);
        socket_fd.sin_addr = *((struct in_addr *)socket_host->h_addr);
        socket_fd.sin_port = htons(port);
        rt_memset(&(socket_fd.sin_zero), 0, sizeof(socket_fd.sin_zero));

        int ret = connect(js_socket_info->socket_id,(struct sockaddr*)&socket_fd,sizeof(struct sockaddr));

        if(ret>=0)
        {
        __isConnected:
            socket_updateProperty_and_startRead(this_value);
        }
    }else
    {
            rt_kprintf("the paramater named 'port' is wrong\n");
            return jerry_create_undefined();
    }

    free(host);
    
    return jerry_acquire_value(this_value);
}

DECLARE_HANDLER(socket_destory)
{
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id == -1)
        return jerry_create_undefined();

    struct close_callback_info* info  = (struct close_callback_info*)malloc(sizeof(struct close_callback_info));
    memset(info, 0, sizeof(struct close_callback_info));

    info->js_server = js_socket_info->js_server;
    info->js_socket = this_value;

    js_send_callback(js_socket_info->close_callback,info,sizeof(struct close_callback_info));

    return jerry_create_undefined();
}

DECLARE_HANDLER(socket_end)
{
    if(args_cnt == 0 || args_cnt > 2)
        return jerry_create_undefined();
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id == -1)
        return jerry_create_undefined();
    
    /*get wirte andy destory function*/
    jerry_value_t js_socket_write = js_get_property(this_value,"write");
    jerry_value_t js_socket_destory = js_get_property(this_value,"destory");
    
    if(jerry_value_is_function(js_socket_write)
        && jerry_value_is_function(js_socket_destory))
    {
        jerry_call_function(js_socket_write,this_value,args,args_cnt);
        jerry_call_function(js_socket_destory,this_value,NULL,0);
    }
    
    jerry_release_value(js_socket_write);
    jerry_release_value(js_socket_destory);
    return this_value;
}

DECLARE_HANDLER(socket_pause)
{
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id == -1)
        return jerry_create_undefined();
    
    if(js_socket_info->readData_thread != 0)
        rt_thread_suspend(js_socket_info->readData_thread);
    
    return jerry_create_undefined();
}

DECLARE_HANDLER(socket_resume)
{
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id == -1)
        return jerry_create_undefined();;
    
    if(js_socket_info->readData_thread != 0)
        rt_thread_resume(js_socket_info->readData_thread);
    
    return jerry_create_undefined();
}

DECLARE_HANDLER(socket_setTimeout)
{
    if(args_cnt == 0 || args_cnt > 2)
        return jerry_create_undefined();
    
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id == -1)
        return jerry_create_undefined();;
    
    int utime =0 ;
    if(jerry_value_is_number(args[0]))
        utime = jerry_get_number_value(args[0]);
    
    if(utime<0)
        return jerry_create_undefined();
    
    js_socket_info->timeout.tv_usec = utime*1000;
    
    if(args_cnt == 2 && jerry_value_is_function(args[1]))
        js_add_event_listener(this_value,"timeout",args[1]);

    return jerry_create_undefined();
}

DECLARE_HANDLER(socket_write)
{
    if(args_cnt==0||args_cnt>2)
        return jerry_create_undefined();
    
    /*get socket_info*/
    //GET_SOCKET_INFO(this_value);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,this_value);
    
    if(js_socket_info->socket_id == -1)
        return jerry_create_undefined();;
    
    if(js_socket_info->writable == true)
    {
        struct write_data_info info;
        memset(&info,0,sizeof(struct write_data_info));
        info.js_data = args[0];
        if(args_cnt==2 && jerry_value_is_function(args[1]))
            info.js_callback = args[1];
        else
            info.js_callback = RT_NULL;
        write(js_socket_info->pipe_write_fd, &info, sizeof(struct write_data_info));
    }
    return jerry_create_undefined();
}

jerry_value_t create_js_socket()
{
    jerry_value_t js_socket = jerry_create_object();
    
    struct socket_info* js_socket_info = (struct socket_info*)malloc(sizeof(struct socket_info));
    memset(js_socket_info,0,sizeof(struct socket_info));
    int ret =pipe_init(js_socket_info);
    if(ret==-1)
    {
        rt_kprintf("net module create pipe failed\n");
        free(js_socket_info);
        jerry_release_value(js_socket);
        return jerry_create_undefined();
    }
    js_socket_info->js_server = RT_NULL;
    /*set method*/
    js_make_emitter(js_socket,jerry_create_undefined());
    
    REGISTER_METHOD_NAME(js_socket,"connect",socket_connect);
    REGISTER_METHOD_NAME(js_socket,"destory",socket_destory);
    REGISTER_METHOD_NAME(js_socket,"end",socket_end);
    REGISTER_METHOD_NAME(js_socket,"pause",socket_pause);
    REGISTER_METHOD_NAME(js_socket,"resume",socket_resume);
    REGISTER_METHOD_NAME(js_socket,"setTimeout",socket_setTimeout);
    REGISTER_METHOD_NAME(js_socket,"write",socket_write);
    
    /* set property*/
    if(js_socket_info)
    {
        jerry_value_t js_info = jerry_create_object();
        js_set_property(js_socket,"info",js_info);
        jerry_set_object_native_pointer(js_info,js_socket_info,&net_socket_info);  // set native_pointer
        jerry_release_value(js_info);

        jerry_value_t js_localAddress = jerry_create_undefined();
        js_set_property(js_socket,"localAddress",js_localAddress);
        jerry_release_value(js_localAddress);
        
        jerry_value_t js_localPort = jerry_create_undefined();
        js_set_property(js_socket,"localPort",js_localPort);
        jerry_release_value(js_localPort);
        
        jerry_value_t js_remoteAddress = jerry_create_undefined();
        js_set_property(js_socket,"remoteAddress",js_remoteAddress);
        jerry_release_value(js_remoteAddress);
        
        jerry_value_t js_remoteFamily = jerry_create_undefined();
        js_set_property(js_socket,"remoteFamily",js_remoteFamily);
        jerry_release_value(js_remoteFamily);
        
        jerry_value_t js_remotePort = jerry_create_undefined();
        js_set_property(js_socket,"remotePort",js_remotePort);
        jerry_release_value(js_remotePort);
    }
    return jerry_acquire_value(js_socket);
}

DECLARE_HANDLER(net_Socket)
{
    if(args_cnt >1)
        return jerry_create_undefined();

    jerry_value_t js_socket = create_js_socket();
    if(jerry_value_is_undefined(js_socket))
        return jerry_create_undefined();
    
    js_make_emitter(js_socket,jerry_create_undefined());
    /*get socket_info*/
    //GET_SOCKET_INFO(js_socket);
    struct socket_info* js_socket_info = RT_NULL; 
    get_net_info((void**)&js_socket_info,js_socket);
    
    // initialize the socket_info
    js_socket_info->socket_id = -1;
    js_socket_info->allowHalfOpen = false;
    js_socket_info->writable = true;
    js_socket_info->readable = true;
    js_socket_info->event_callback = js_add_callback(js_event_callback_func_net);
    js_socket_info->fun_callback = js_add_callback(js_callback_func_net);
    js_socket_info->close_callback = js_add_callback(js_scoket_close_callback_func_net);
    js_socket_info->socket_sem = rt_sem_create("socket_sem",1,RT_IPC_FLAG_FIFO);
    js_socket_info->timeout.tv_sec = 0;
    js_socket_info->timeout.tv_usec = 0;
    //get options value and set the paramaters
    if(args_cnt==1 && jerry_value_is_object(args[0]))
    {
        jerry_value_t js_fd =js_get_property(args[0],"fd");
        if(jerry_value_is_number(js_fd))
            js_socket_info->socket_id = jerry_get_number_value(js_fd);
        jerry_release_value(js_fd);
        
        jerry_value_t js_allowHalfOpen  = js_get_property(args[0],"allowHalfOpen");
        if(jerry_value_is_boolean(js_allowHalfOpen))
            js_socket_info->allowHalfOpen = jerry_get_boolean_value(js_allowHalfOpen);
        jerry_release_value(js_allowHalfOpen);
        
        jerry_value_t js_writable  = js_get_property(args[0],"writable");
        if(jerry_value_is_boolean(js_writable))
            js_socket_info->writable = jerry_get_boolean_value(js_writable);
        jerry_release_value(js_writable);
        
        jerry_value_t js_readable  = js_get_property(args[0],"readable");
        if(jerry_value_is_boolean(js_readable))
            js_socket_info->writable = jerry_get_boolean_value(js_readable);
        jerry_release_value(js_readable);
    }
    return jerry_acquire_value(js_socket);
}


/*  the method of server  */
static void server_listen_entry(void* p)
{
    jerry_value_t js_server = ((struct listen_thread_info*)p)->js_server;
    int backlog = ((struct listen_thread_info*)p)->backlog;
    
    free(p);

    /*get socket_info*/
    struct server_info* js_server_info = RT_NULL; 
    get_net_info((void**)&js_server_info,js_server);
    int socket_id = js_server_info->server_fd;
    rt_sem_t server_sem = js_server_info->server_sem;
    
    /*start listening and accepting*/
    if(socket > 0)
    {
        int ret = listen(socket_id,backlog); // start listen
        struct sockaddr_in client_addr;
        socklen_t addrlen=1;
        
        rt_kprintf("SERVER_MAX_CONNECTIONS : %d \n",backlog);
        while(1)
        {
            memset(&client_addr,0,sizeof(struct sockaddr_in));
            int client_fd = accept(socket_id,(struct sockaddr *)&client_addr,&addrlen); //accept ths client socket
            /*judge whether the new client are allow to connect*/
            rt_sem_take(server_sem,RT_WAITING_FOREVER);

            int client_count = get_socket_list_count(&js_server_info->socket_list);
            rt_sem_release(server_sem);
            if(client_fd && client_count < backlog)
            {
                /*set the option of new_socket func*/
                jerry_value_t js_new_socket_option = jerry_create_object();
                
                jerry_value_t js_socket_allowHalfOpen = jerry_create_boolean(js_server_info->allowHalfOpen);
                js_set_property(js_new_socket_option,"allowHalfOpen",js_socket_allowHalfOpen);
                jerry_release_value(js_socket_allowHalfOpen);
                
                jerry_value_t js_socket_fd = jerry_create_number(client_fd);
                js_set_property(js_new_socket_option,"fd",js_socket_fd);
                jerry_release_value(js_socket_fd);

                /*do new_socket func*/
                jerry_value_t js_new_socket = jerry_create_external_function(net_Socket_handler);
                jerry_value_t js_socket = jerry_call_function(js_new_socket,jerry_create_undefined(),&js_new_socket_option,1);
                jerry_release_value(js_new_socket_option);
                jerry_release_value(js_new_socket);

                if(jerry_value_is_undefined(js_socket))
                {
                    jerry_release_value(js_socket);
                    closesocket(client_fd);
                }else{
                    /*set the socket's server*/
                    //GET_SOCKET_INFO(js_socket);
                    struct socket_info* js_socket_info = RT_NULL; 
                    get_net_info((void**)&js_socket_info,js_socket);
                    js_socket_info->js_server = js_server;

                    /*add js_socket to socket_list*/
                    rt_sem_take(server_sem,RT_WAITING_FOREVER);
                    
                    socket_list_insert(&js_server_info->socket_list,js_socket);
                    client_count = get_socket_list_count(&js_server_info->socket_list);
                    socket_updateProperty_and_startRead(js_socket);
                    
                    rt_sem_release(server_sem);
                    /*emit 'connention' event*/
                    /*send the new socket to connection listener of js_server*/
                    struct event_callback_info* cb_info = (struct event_callback_info*)malloc(sizeof(struct event_callback_info));
                    memset(cb_info,0,sizeof(struct event_callback_info));

                    cb_info->event_name = strdup("connection");
                    cb_info->js_target = js_server;
                    cb_info->js_return = js_socket;
                    js_send_callback(js_server_info->event_callback,cb_info,sizeof(struct event_callback_info));
                }
            }
                
            else
                closesocket(client_fd);
            
        }
    }
}
DECLARE_HANDLER(server_close)
{
    /*get server info*/
    //GET_SERVER_INFO(this_value);
    struct server_info* js_server_info = RT_NULL; 
    get_net_info((void**)&js_server_info,this_value);
    if(js_server_info->server_fd == -1)
        return jerry_create_undefined();
    
    if(args_cnt >1)
        return jerry_create_undefined();
    
    if(args_cnt>0 && jerry_value_is_function(args[0]))
        js_add_event_listener(this_value,"close",args[0]);

    /*stop listening*/
    rt_thread_delete(js_server_info->listen_thread);
    closesocket(js_server_info->server_fd);
    js_server_info->server_fd = -1;
    
    if(get_socket_list_count(&(js_server_info->socket_list)) == 0)
        closeServer(this_value);
    
    return jerry_create_undefined();
}

DECLARE_HANDLER(server_listen)
{
    if(args_cnt == 0|| args_cnt >4)
        return jerry_create_undefined();
    
    /*get server info*/
    //GET_SERVER_INFO(this_value);
    struct server_info* js_server_info = RT_NULL; 
    get_net_info((void**)&js_server_info,this_value);
    /*end listen_thread*/
    //rt_thread_delete(js_server_info->listen_thread);
    
    /*the value of option*/
    int port = -1;
    char* host = RT_NULL;
    int backlog = SERVER_MAX_CONNECTIONS;
    int family = 4;
    
    jerry_value_t js_listen_cb = RT_NULL;
    #if LWIP_IPV6
    family = 6;
    #endif
    
    if(jerry_value_is_object(args[0]))
    {
        jerry_value_t js_port = js_get_property(args[0],"port");
        if(jerry_value_is_number(js_port))
            port = jerry_get_number_value(js_port);
        jerry_release_value(js_port);
        
        jerry_value_t js_host = js_get_property(args[0],"host");
        if(jerry_value_is_string(js_host))
            host = js_value_to_string(js_host);
        jerry_release_value(js_host);
        
        jerry_value_t js_backlog = js_get_property(args[0],"backlog");
        if(jerry_value_is_number(js_backlog))
        {
            int count = jerry_get_number_value(js_backlog);
            if(count < backlog)
                backlog = count;
        }
        jerry_release_value(js_backlog);
        
        if(args_cnt==2 && jerry_value_is_function(args[1]))
            js_listen_cb = args[1];
    }else if(jerry_value_is_number(args[0]))
    {
        switch(args_cnt)
        {
            case 4 :
                if(jerry_value_is_string(args[3]))
                    host = js_value_to_string(args[3]);
                else if(jerry_value_is_number(args[3]))
                    backlog = jerry_get_number_value(args[3]);
                else if(jerry_value_is_function(args[3]))
                    js_listen_cb =  args[3];
            case 3 :
                if(jerry_value_is_string(args[2]))
                    host = js_value_to_string(args[2]);
                else if(jerry_value_is_number(args[2]))
                    backlog = jerry_get_number_value(args[2]);
                else if(jerry_value_is_function(args[2]))
                    js_listen_cb =  args[2];
            case 2 :
                if(jerry_value_is_string(args[1]))
                    host = js_value_to_string(args[1]);
                else if(jerry_value_is_number(args[1]))
                    backlog = jerry_get_number_value(args[1]);
                else if(jerry_value_is_function(args[1]))
                    js_listen_cb =  args[1];
            case 1 :
                port = jerry_get_number_value(args[0]);
            default:
                break;
        }
    }

    if(port < 0 )
        goto __exit;
    
    /*create socket and bind*/
    if(family ==4)
        js_server_info->server_fd = socket(AF_INET,SOCK_STREAM,0);
    else if(family == 6)
        js_server_info->server_fd = socket(AF_INET6,SOCK_STREAM,0);

    if(js_server_info->server_fd > 0)
    {
        struct sockaddr_in server_addr;
        if(family ==4)
            server_addr.sin_family = AF_INET;
        else if(family == 6)
            server_addr.sin_family = AF_INET6;
        if(host)
            server_addr.sin_addr.s_addr = inet_addr(host);
        else
            server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        const int on=1;
        setsockopt(js_server_info->server_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)); 
        int ret = bind(js_server_info->server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));;
        if(ret == 0)
        {
            struct listen_thread_info* info = (struct listen_thread_info*)malloc(sizeof(struct listen_thread_info));
            memset(info,0,sizeof(struct listen_thread_info));
            info->js_server = this_value;
            info->backlog = backlog;
            
            //GET_SOCKET_INFO(info->js_target);
            struct socket_info* js_socket_info = RT_NULL; 
            get_net_info((void**)&js_socket_info,this_value);
            
            js_server_info->listen_thread = rt_thread_create("net_listen",server_listen_entry,info,1024,20,5);
            rt_err_t ret = rt_thread_startup(js_server_info->listen_thread);
            
            if(ret == RT_EOK)
            {
                /*emit 'listening' event*/
                struct event_callback_info* cb_info = (struct event_callback_info*)malloc(sizeof(struct event_callback_info));
                memset(cb_info,0,sizeof(struct event_callback_info));
                        
                cb_info->event_name = strdup("listening");
                cb_info->js_target = this_value;
                cb_info->js_return = RT_NULL;
                        
                js_send_callback(js_socket_info->event_callback,cb_info,sizeof(struct event_callback_info));
            }
        }
    }

    __exit:
        jerry_release_value(js_listen_cb);
        return jerry_create_undefined();
}

/* the method of net */

DECLARE_HANDLER(net_connect)
{
    if(args_cnt == 0 || args_cnt > 3)
        return jerry_create_undefined();
    
    /* create socket */
    jerry_value_t js_new_socket = js_get_property(this_value,"Socket");
    jerry_value_t js_socket = jerry_call_function(js_new_socket,this_value,NULL,0);
    jerry_release_value(js_new_socket);
    
    if(jerry_value_is_undefined(js_socket))
    {
        jerry_release_value(js_socket);
        return jerry_create_undefined();
    }
    /* do socket.connect */
    jerry_value_t js_socket_connect = js_get_property(js_socket,"connect");
    jerry_value_t js_ret = jerry_call_function(js_socket_connect,js_socket,args,args_cnt);
    jerry_release_value(js_socket_connect);
    
    if(jerry_value_is_undefined(js_ret))
    {
        jerry_release_value(js_ret);
        return jerry_create_undefined();
    }
    
    jerry_release_value(js_ret);
    return js_socket;
}

DECLARE_HANDLER(net_createServer)
{
    jerry_value_t js_server = jerry_create_object();

    js_make_emitter(js_server,jerry_create_undefined());
    
    struct server_info* js_server_info = (struct server_info*)malloc(sizeof(struct server_info));
    memset(js_server_info,0,sizeof(struct server_info));
    
    js_server_info->allowHalfOpen = false;
    js_server_info->server_fd = -1;
    js_server_info->event_callback = js_add_callback(js_event_callback_func_net);
    js_server_info->fun_callback = js_add_callback(js_callback_func_net);
    js_server_info->close_callback = js_add_callback(js_server_close_callback_func_net);
    js_server_info->socket_list = RT_NULL;
    js_server_info->server_sem = rt_sem_create("server_sem",1,RT_IPC_FLAG_FIFO);
    if(args_cnt >0 && jerry_value_is_object(args[0]))
    {
        jerry_value_t js_allowHalfOpen = js_get_property(args[0],"allowHalfOpen");
        
        if(jerry_value_is_boolean(js_allowHalfOpen))
            js_server_info->allowHalfOpen = jerry_value_to_boolean(js_allowHalfOpen);
        jerry_release_value(js_allowHalfOpen);
    }

    if(args_cnt > 0 &&jerry_value_is_function(args[0]))
        js_add_event_listener(js_server,"connection",args[0]);
    if(args_cnt > 1 && jerry_value_is_function(args[1]))
        js_add_event_listener(js_server,"connection",args[1]);
    
    /*add property of server*/
    
    jerry_value_t js_info = jerry_create_object();
    js_set_property(js_server,"info",js_info);
    jerry_set_object_native_pointer(js_info,js_server_info,&net_server_info);  // set native_pointer
    jerry_release_value(js_info);

    /*register method to server*/
    REGISTER_METHOD_NAME(js_server,"listen",server_listen);
    REGISTER_METHOD_NAME(js_server,"close",server_close);

    return jerry_acquire_value(js_server);
}

jerry_value_t jerry_net_init()
{
    
    jerry_value_t js_net = jerry_create_object();
    
    REGISTER_METHOD_NAME(js_net,"Socket",net_Socket);
    REGISTER_METHOD_NAME(js_net,"connect",net_connect);
    REGISTER_METHOD_NAME(js_net,"createConnection",net_connect);
    REGISTER_METHOD_NAME(js_net,"createServer",net_createServer);
    
    return jerry_acquire_value(js_net);
}

JS_MODULE(net,jerry_net_init)