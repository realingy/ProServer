/**
 * @file client.cpp
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
#include <sys/epoll.h>

#define MAXLINE 1024
#define IPADDRESS "127.0.0.1"
#define FDSIZE 1024
#define EPOLLEVENTS 20
#define SERVER_PORT 6666

int count = 0;

//连接服务器
int connect() {
    int sockfd;
    struct sockaddr_in serveraddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, IPADDRESS, &serveraddr.sin_addr);
    connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    return sockfd;
}

void add_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = state;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void delete_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = state;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void modify_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = state;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void do_read(int epollfd, int fd, int sockfd, char *buf) {
    int nread;
    nread = read(fd, buf, MAXLINE);
    if(-1 == nread) {
        perror("read error!");
        close(fd);
    } else if(0 == nread) {
        fprintf(stderr, "client error.\n");
        close(fd);
    } else {
        if(fd == STDIN_FILENO) {
            add_event(epollfd, sockfd, EPOLLOUT); //???
        } else {
            delete_event(epollfd, sockfd, EPOLLIN); //???
            add_event(epollfd, STDOUT_FILENO, EPOLLOUT); //???
        }
    }
}

void do_write(int epollfd, int fd, int sockfd, char *buf) {
    int nwrite;
    char temp[100];
    buf[strlen(buf)-1] = '\0';
    snprintf(temp, sizeof(temp), "%s_%02d\n", buf, count++);
    nwrite = write(fd, temp, strlen(temp));
    if(-1 == nwrite) {
        perror("write error!");
        close(fd);
    } else {
        if(STDOUT_FILENO == fd) {
            delete_event(epollfd, fd, EPOLLOUT);
        } else {
            modify_event(epollfd, fd, EPOLLIN);
        }
    } 
    memset(buf, 0, MAXLINE);
}

void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf) {
    int fd;
    for(int i = 0; i < num; i++) {
        fd = events[i].data.fd;
        if(events[i].events & EPOLLIN) {
            do_read(epollfd, fd, sockfd, buf);
        } else if(events[i].events & EPOLLOUT) {
            do_write(epollfd, fd, sockfd, buf);
        }
    }
}
    
void handle_connection(int sockfd) {
    int epollfd;
    epollfd = epoll_create(FDSIZE); //epoll文件描述符
    struct epoll_event events[EPOLLEVENTS]; //事件数组
    char buf[MAXLINE]; //数据缓存
    memset(buf, 0, MAXLINE);
    add_event(epollfd, STDIN_FILENO, EPOLLIN); //添加标准输入为监控事件
    int ret;
    while(1) {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, sockfd, buf);
    }
    close(epollfd);
}

int main() {
    int sockfd;
    sockfd = connect();

    handle_connection(sockfd);

    close(sockfd);
    return 0;
}

