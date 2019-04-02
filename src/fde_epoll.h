#ifndef __FDE_EPOLL_H__
#define __FDE_EPOLL_H__

/*
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
*/
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "fde.h"

class Epevents : public Fdevents
{
public:
    typedef std::vector<struct Fdevent *> events_t;

private:
    static const int MAX_FDS = 8 * 1024;
    int ep_fd;
    struct epoll_event ep_events[MAX_FDS];

    events_t events;
    events_t ready_events;

//   struct Fdevent *get_fde(int fd);
	
public:
    Epevents();
    ~Epevents();

    bool isset(int fd, int flag);
    int set(int fd, int flags, int data_num, void *data_ptr);
    int del(int fd);
    int clr(int fd, int flags);
    const events_t* wait(int timeout_ms = -1);
};

#endif //__FDE_EPOLL_H__
