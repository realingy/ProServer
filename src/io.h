#ifndef __UTIL_IO_H__
#define __UTIL_IO_H__

#include <errno.h>
#include <vector.h>

#define EPOLL_PROC 1

#ifdef EPOLL_PROC
    #include <sys/epoll.h>
#else
    #include <sys/select.h>
#endif

namespace proserver {

    const static int EVENT_NONE = 0;
    const static int EVENT_IN  = (1<<0);
    const static int EVENT_PRI = (1<<1);
    const static int EVENT_OUT = (1<<2);
    const static int EVENT_HUP = (1<<3);
    const static int EVENT_ERR = (1<<4);

    struct event{
        int fd_;
        int flag_; //订阅事件
        int event_; //就绪事件
        struct {
            int num_;
            void *ptr_;
        }msg;

        bool writable() const {
            return event_ & FDEVENT_IN;
        }

    };


};


#endif //__UTIL_IO_H__

