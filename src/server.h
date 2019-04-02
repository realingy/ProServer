/**
 * @file epollserver.h
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月26日 15时00分14秒
 */

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
#include <poll.h>
#include <sys/epoll.h>
#include <map>
#include <vector>

#include "link.h"
#include "handler.h"

#define MAXLINE 1024
#define LISTENQ 5
#define INFTIM -1
#define EPOLLSIZE 1000
#define EVENTS 100

class Fdevents;

class Server
{
public:
    Server();
    ~Server();

    void loop();
    int single_loop();

private:
    int bind_and_listen(const char *ip, int port);
    void add_event(int fd, int state);
    //接收新的client连接,并将client文件描述符添加到监控描述符中
    void handle_accept();
    void delete_event(int fd, int state);
    void modify_event(int fd, int state);
    void do_read(int fd);
    void do_write(int fd);
    //处理epoll_wait等到的事件，并处理事件
    //:1.添加新的client连接 2.读取client发来的包 3.发包
    void handle_events(int num);
    void do_epoll();

    //添加事件句柄
//    void add_handler(Handler *handler);

private:
    Fdevents *fdes;
    int m_listenfd; //监听socket
    int m_epollfd;  //epoll文件描述符
    char m_buf[MAXLINE];
    struct sockaddr_in m_addr; //本地地址信息
    struct epoll_event m_events[EVENTS]; //epoll事件组
    Link *serv_link_;
    int link_count_;

    std::map<int64_t, Session *> sessions;
    std::vector<Handler *> handlers;

#if 0
    Session *accept_session();
    Session *get_session(int64_t sess_id);
    int close_session(Session *sess);
    int read_session(Session *sess);
    int write_session(Session *sess);
#endif
};

