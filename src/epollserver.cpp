/**
 * @file epollserver.cpp
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月26日 15时00分18秒
 */

#include "epollserver.h"

Server::Server()
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(PORT);
    m_addr.sin_addr.s_addr = INADDR_ANY;

    m_epollfd = epoll_create(EPOLLSIZE); //创建epoll文件描述符
    memset(m_buf, 0, sizeof(m_buf));

    bind_and_listen(); //绑定加监听
}

Server::~Server()
{
    close(m_epollfd);
}

int Server::bind_and_listen()
{
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
