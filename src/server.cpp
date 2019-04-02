/**
 * @file epollserver.cpp
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月26日 15时00分18秒
 */

#include "server.h"

#if 0
namespace Test
{
    const static int DEFAULT_TYPE = 0;
    const static int HANDLER_TYPE = 1;

    //服务器的构造函数
    Server::Server() {
        signal(SIGPIPE, SIG_IGN);
        link_count = 0; //连接数为0
        serv_link = NULL; //连接
        fdes = new Fdevents();
    }

    Server::~Server() {
        for (int i = 0; i < handlers.size(); i++)
        {
            Handler *handler = handlers[i];
            handler->free();
            delete handler;
        }
        handlers.clear();
        delete serv_link;
        delete fdes;
    }

    Server *Server::listen(const std::string &ip, int port) {
        Link *serv_link = Link::listen(ip, port);
        if(!serv_link)
            return NULL;

        Server *ret = new Server();
        ret->serv_link = serv_link;
        ret->fdes->set(serv_link->fd(), FDEVENT_IN, DEFAULT_TYPE, serv_link);
        return &ret;
    }

    void Server::add_handler(Handler *handler) {

    }

    int Server::close_session(Session *sess) {

    }

    int Server::read_session(Session *sess) {

    }

    int Server::write_session(Session *sess) {

    }

};
#endif

#if 1
Server::Server()
{
    m_epollfd = epoll_create(EPOLLSIZE); //创建epoll文件描述符
    memset(m_buf, 0, sizeof(m_buf));

    const char *ip = "127.0.0.1";
    int port = 6666;
    bind_and_listen(ip, port); //绑定加监听
    do_epoll(); //
}

Server::~Server()
{
    if(m_epollfd) {
        ::close(m_epollfd);
    }
}

int Server::bind_and_listen(const char *ip, int port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons((short)port);
//    m_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, ip, &m_addr.sin_addr);

    if((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    printf("socket ok \n");

    bzero(&(m_addr.sin_zero), 0);
    if(bind(m_listenfd, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return -2;
    }

    printf("bind ok \n");

    if (listen(m_listenfd, LISTENQ) == -1) {
        perror ("listen");
        return -3;
    }

    printf("listen ok!\n");
    
    return 0;
}

void Server::add_event(int fd, int state)
{
    struct epoll_event event;
    event.events = state;
    event.data.fd = fd;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event);
}

//接收新的client连接,并将client文件描述符添加到监控描述符中
void Server::handle_accept()
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen;
    clientfd = accept(m_listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
    if(clientfd == -1) {
        perror("accept error:");
    } else {
        printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        add_event(clientfd, EPOLLIN);
    }
}

void Server::delete_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void Server::modify_event(int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void Server::do_read(int fd)
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

void Server::do_write(int fd)
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

//处理epoll_wait等到的事件，并处理事件:1.添加新的client连接 2.读取client发来的包 3.发包
void Server::handle_events(int num)
{
    int fd;
    for(int i = 0; i < num; i++) {
        fd = m_events[i].data.fd;
        if((fd == m_listenfd) && (m_events[i].events & EPOLLIN)) 
            handle_accept(); //添加新的client连接
        else if(m_events[i].events & EPOLLIN)
            do_read(fd); //读取旧client的包
        else if(m_events[i].events & EPOLLOUT)
            do_write(fd); //发包
    }
}

void Server::do_epoll()
{
    add_event(m_listenfd, EPOLLIN); //先将listenfd添加到监听事件组中
    int ret;
    while(1) {
        ret = epoll_wait(m_epollfd, m_events, EVENTS, -1);
        handle_events(ret);
    }
}

#if 0
//删除事件
int del(int fd)
{
    struct epoll_event epe;
    int ret = epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &epe);

    if(-1 == ret) {

    }

//////////////////////////////////////////////////////////
    if(!(fde->s_flags) & flags) {
        return 0;
    }

    fde->s_flags &= ~flags;

    int ctl_op = fde->s_flags ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
//////////////////////////////////////////////////////////
}

//清空事件
int clr(int fd, int flags)
{
    struct Fdevent *fde = get_fde(fd);
    if(!(fde->s_flags) & flags) {
        return 0;
    }

    fde->s_flags &= ~flags;

    int ctl_op = fde->s_flags ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    struct epoll_event epe;
    epe.data.ptr = fde;
    epe.events = 0;
    if(fde->s_flags & FDEVENT_IN) 
        epe.events |= EPOLLIN;
    if(fde->s_flags & FDEVENT_OUT) 
        epe.events |= EPOLLOUT;

    int ret = epoll_ctl(m_epollfd, ctl_op, fd, &epe);
    if(ret == -1) {
        return -1;
    }
    return 0;
}

//等待客户端事件,将已就绪事件清空,加载新的就绪事件
const int wait(int timeout) {
    struct Fdevent *fde;
    struct epoll_event *epe;
    ready_events.clear();

    int ret = epoll_wait(m_epollfd, m_events, EVENTS, timeout);
    if(-1 == ret) {
        return NULL;
    }

    for(int i = 0; i < ret; i++) {
        epe = &m_events[i];
        fde = (struct Fdevent *)epe->data.ptr;

        fde->events = FDEVENT_NONE;
        if(epe->events & EPOLLIN)
            fde->events |= FDEVENT_IN;
        else if(epe->events & EPOLLPRI)
            fde->events |= FDEVENT_IN;
        else if(epe->events & EPOLLOUT)
            fde->events |= FDEVENT_OUT;
        else if(epe->events & EPOLLHUP)
            fde->events |= FDEVENT_ERR;
        else if(epe->events & EPOLLERR)
            fde->events |= FDEVENT_ERR;
        ready_events.push_back(fde);
    }
    return &ready_events;
}
#endif
#endif


