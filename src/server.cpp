/**
 * @file server.cpp
 * @date 2019年01月07日 09时58分41秒
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

#define PORT 6666
#define MAXLINE 1024
#define LISTENQ 5
#define INFTIM -1
#define EPOLLSIZE 1000
#define EVENTS 100

/*创建套接字，进行绑定和监听*/
int bind_and_listen() {
    int serverfd; /*监听socket: serverfd*/
    struct sockaddr_in my_addr; /*本机地址信息*/
    if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    printf("socket ok \n");

    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(PORT);
    my_addr.sin_addr.s_addr=INADDR_ANY;
    bzero(&(my_addr.sin_zero), 0);
    if(bind(serverfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return -2;
    }

    printf("bind ok \n");

    if (listen(serverfd, LISTENQ) == -1) {
        perror ("listen");
        return -3;
    }

    printf("listen ok!\n");
    
    return serverfd ;
}

void add_event(int epollfd, int fd, int state) {
    struct epoll_event event;
    event.events = state;
    event.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

//接收新的client连接,并将client文件描述符添加到监控描述符中
void handle_accept(int epollfd, int listenfd) {
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen;
    clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
    if(clientfd == -1) {
        perror("accept error:");
    } else {
        printf("accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        add_event(epollfd, clientfd, EPOLLIN);
    }
}

void delete_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void modify_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void do_read(int epollfd, int fd, char *buf) {
    int nread;
    nread = read(fd, buf, MAXLINE);
    if(-1 == nread) {
        perror("read error!");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    } else if(!nread) {
        fprintf(stderr, "client error.\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    } else {
        printf("read message is: %s", buf);
        modify_event(epollfd, fd, EPOLLOUT);
    }
}

void do_write(int epollfd, int fd, char *buf) {
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if(-1 == nwrite) {
        perror("write error!");
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    } else {
        modify_event(epollfd, fd, EPOLLIN);
    }
    memset(buf, 0, MAXLINE);
}

//处理epoll_wait等到的事件，并处理事件:1.添加新的client连接 2.读取client发来的包 3.发包
void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf) {
    int fd;
    for(int i = 0; i < num; i++) {
        fd = events[i].data.fd;
        if((fd == listenfd) && (events[i].events & EPOLLIN)) 
            handle_accept(epollfd, listenfd); //添加新的client连接
        else if(events[i].events & EPOLLIN)
            do_read(epollfd, fd, buf); //读取旧client的包
        else if(events[i].events & EPOLLOUT)
            do_write(epollfd, fd, buf); //发包
    }
}

void do_epoll(int listenfd) {
    int epollfd; //
    int ret;
    epollfd = epoll_create(EPOLLSIZE); //创建epoll文件描述符
    char buf[MAXLINE];
    memset(buf, 0, sizeof(buf));
    struct epoll_event events[EVENTS]; //epoll事件数组
    add_event(epollfd, listenfd, EPOLLIN);
    while(1) {
        ret = epoll_wait(epollfd, events, EVENTS, -1);
        handle_events(epollfd, events, ret, listenfd, buf);
    }
    close(epollfd);
}

int main() {
    int listenfd = bind_and_listen();
    if(listenfd < 0)
        return 0;
    do_epoll(listenfd);
    return 0;
}

