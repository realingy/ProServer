/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/

#ifndef UTIL_FDE_H
#define UTIL_FDE_H

#include <errno.h>
#include <vector>

#if 0
#ifdef __linux__
	#define HAVE_EPOLL 1
#endif

#ifdef HAVE_EPOLL
	#include <sys/epoll.h>
#else
	#include <sys/select.h>
#endif
#endif

const static int FDEVENT_NONE = (0);
const static int FDEVENT_IN   = (1<<0);
const static int FDEVENT_PRI  = (1<<1);
const static int FDEVENT_OUT  = (1<<2);
const static int FDEVENT_HUP  = (1<<3);
const static int FDEVENT_ERR  = (1<<4);

struct Fdevent{
    int fd;
    int s_flags; // subscribed events
    int events;	 // ready events
    struct{
        int num;
        void *ptr;
    }data;

    bool readable() const{
        return events & FDEVENT_IN;
    }
    bool writable() const{
        return events & FDEVENT_OUT;
    }
};

class Fdevents
{
public:
    typedef std::vector<struct Fdevent *> events_t;

private:
/*
//#ifdef HAVE_EPOLL
#if 1 
    static const int MAX_FDS = 8 * 1024;
    int ep_fd;
    struct epoll_event ep_events[MAX_FDS];
#else
    int maxfd;
    fd_set readset;
    fd_set writeset;
#endif
*/
    events_t events;
    events_t ready_events;

public:
    Fdevents() { }
    ~Fdevents() { }

    struct Fdevent *get_fde(int fd);
    virtual bool isset(int fd, int flag) = 0;
    virtual int set(int fd, int flags, int data_num, void *data_ptr) = 0;
    virtual int del(int fd) = 0;
    virtual int clr(int fd, int flags) = 0;
    virtual const events_t* wait(int timeout_ms=-1) = 0;
};

#endif
