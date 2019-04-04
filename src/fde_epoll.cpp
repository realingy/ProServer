#ifndef UTIL_FDE_EPOLL_H
#define UTIL_FDE_EPOLL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <errno.h>
#include "fde.h"


Epevents::Epevents()
{
    ep_fd = epoll_create(1024);
}

Epevents::~Epevents()
{
    for(int i=0; i<(int)events.size(); i++){
        delete events[i];
    }
    if(ep_fd){
        ::close(ep_fd);
    }
    events.clear();
    ready_events.clear();
}

bool Epevents::isset(int fd, int flag)
{
    struct Fdevent *fde = get_fde(fd);
    return (bool)(fde->s_flags & flag);
}

int Epevents::set(int fd, int flags, int data_num, void *data_ptr)
{
    struct Fdevent *fde = get_fde(fd);
    if(fde->s_flags & flags){
        return 0;
    }
    int ctl_op = fde->s_flags? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

    fde->s_flags |= flags;
    fde->data.num = data_num;
    fde->data.ptr = data_ptr;

    struct epoll_event epe;
    epe.data.ptr = fde;
    epe.events = 0;
    if(fde->s_flags & FDEVENT_IN)  epe.events |= EPOLLIN;
    if(fde->s_flags & FDEVENT_OUT) epe.events |= EPOLLOUT;

    int ret = epoll_ctl(ep_fd, ctl_op, fd, &epe);
    if(ret == -1){
        return -1;
    }
    return 0;
}

bool Epevents::add(int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &event);
    return true;
}

void Epevents::loop(Link *servlink)
{
    //先将listenfd添加到监听事件组中
    int m_listenfd = servlink->fd();
    add(m_listenfd);
    int ret;
    while(1) {
        ret = epoll_wait(ep_fd, ep_events, MAX_FDS, -1);
        handle_events(ret);
    }
}

//处理epoll_wait等到的事件，并处理事件:
// 1.添加新的client连接
// 2.读取client发来的包
// 3.发包
void Epevents::handle_events(int num)
{
    int fd;
    for(int i = 0; i < num; i++) {
        fd = ep_events[i].data.fd;
        if((fd == m_listenfd) && (ep_events[i].events & EPOLLIN)) 
            handle_accept(); //添加新的client连接
        else if(ep_events[i].events & EPOLLIN)
            do_read(fd); //读取旧client的包
        else if(ep_events[i].events & EPOLLOUT)
            do_write(fd); //发包
    }
}

void Epevents::handle_accept()
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen;
    clientfd = accept(m_listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
    if(clientfd == -1) {
        perror("accept error:");
    } else {
        printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        add(clientfd);
    }
#if 0
    int clientfd = serv_link_->accept();
    add_event(clientfd, EPOLLIN);
#endif
}

void Epevents::do_read(int fd)
{
    int nread;
    nread = read(fd, m_buf, MAXLINE);
    if(-1 == nread) {
        perror("read error!");
        close(fd);
        delete_event(fd, EPOLLIN);
    } else if(!nread) {
        fprintf(stderr, "client error.\n");
        close(fd);
        delete_event(fd, EPOLLIN);
    } else {
        printf("read message is: %s", m_buf);
        modify_event(fd, EPOLLOUT);
    }
}

void Epevents::do_write(int fd)
{
    int nwrite;
    nwrite = write(fd, &m_buf, strlen(m_buf));
    if(-1 == nwrite) {
        perror("write error!");
        close(fd);
        delete_event(fd, EPOLLOUT);
    } else {
        modify_event(fd, EPOLLIN);
    }
    memset(m_buf, 0, MAXLINE);
}

void Epevents::delete_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &ev);
}

void Epevents::modify_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &ev);
}






int Epevents::del(int fd)
{
    struct epoll_event epe;
    int ret = epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &epe);
    if(ret == -1){
        return -1;
    }

    struct Fdevent *fde = get_fde(fd);
    fde->s_flags = FDEVENT_NONE;
    return 0;
}

int Epevents::clr(int fd, int flags)
{
    struct Fdevent *fde = get_fde(fd);
    if(!(fde->s_flags & flags)){
        return 0;
    }

    fde->s_flags &= ~flags;
    int ctl_op = fde->s_flags? EPOLL_CTL_MOD: EPOLL_CTL_DEL;

    struct epoll_event epe;
    epe.data.ptr = fde;
    epe.events = 0;
    if(fde->s_flags & FDEVENT_IN)  epe.events |= EPOLLIN;
    if(fde->s_flags & FDEVENT_OUT) epe.events |= EPOLLOUT;

    int ret = epoll_ctl(ep_fd, ctl_op, fd, &epe);
    if(ret == -1){
        return -1;
    }
    return 0;
}

const Fdevents::events_t* Epevents::wait(int timeout_ms)
{
    struct Fdevent *fde;
    struct epoll_event *epe;
    ready_events.clear();

    int nfds = epoll_wait(ep_fd, ep_events, MAX_FDS, timeout_ms);
    if(nfds == -1){
        if(errno == EINTR){
            return &ready_events;
        }
        return NULL;
    }

    for(int i = 0; i < nfds; i++){
        epe = &ep_events[i];
        fde = (struct Fdevent *)epe->data.ptr;
	
        fde->events = FDEVENT_NONE;
        if(epe->events & EPOLLIN)  fde->events |= FDEVENT_IN;
        if(epe->events & EPOLLPRI) fde->events |= FDEVENT_IN;
        if(epe->events & EPOLLOUT) fde->events |= FDEVENT_OUT;
        if(epe->events & EPOLLHUP) fde->events |= FDEVENT_ERR;
        if(epe->events & EPOLLERR) fde->events |= FDEVENT_ERR;
        ready_events.push_back(fde);
    }
    return &ready_events;
}

#endif //

