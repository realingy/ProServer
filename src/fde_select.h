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
#include <sys/select.h>
#include "fde.h"

class Slevents : public Fdevents
{
public:
    typedef std::vector<struct Fdevent *> events_t;

private:
    int maxfd;
    fd_set readset;
    fd_set writeset;

    events_t events;
    events_t ready_events;
	
public:
    Slevents();
    ~Slevents();

    bool isset(int fd, int flag);
    int set(int fd, int flags, int data_num, void *data_ptr);
    int del(int fd);
    int clr(int fd, int flags);
    const events_t* wait(int timeout_ms = -1);
};

#endif //__FDE_EPOLL_H__
