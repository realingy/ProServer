#include "util/log.h"
#include "handler.h"
#include "util/thread.h"

int Handler::m_init(){
    resps = new SelectableQueue<Response *>();
    return init();
}

int Handler::m_free(){
    delete resps;
    return free();
}

int Handler::fd(){
    return resps->fd();
}

HandlerState Handler::accept(const Session &sess){
    return HANDLE_OK;
}

HandlerState Handler::close(const Session &sess){
    return HANDLE_OK;
}

HandlerState Handler::proc(const Request &req, Response *resp){
    return HANDLE_OK;
}

void Handler::async_send(Response *resp){
    resps->push(resp);
}

Response* Handler::handle(){
    while(resps->size() > 0){
        Response *resp;
        if(resps->pop(&resp) == 1 && resp != NULL){
            return resp;
        }
    }

    return NULL;
}

