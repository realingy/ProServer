/**
 * @file fde.h
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年04月01日 17时29分42秒
 */

#ifndef __FDE_H__
#define __FDE_H__

#include <sys/epoll.h>

namespace Sim
{
    //
    typedef struct Fd_event {

    }Fdevent;

    class Fdevents {
    public:

    private:
        int maxfd;
        fd_set readset;
        fd_set writeset;
        events_t events;
        events_t ready_events;

        Fdevent *get_fde(int fd);
    };

}; //namspace Sim

#endif //__FDE_H__
