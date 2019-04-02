/**
 * @file link.cpp
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月27日 10时09分40秒
 */

#include <stdio.h>
#include <error.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>

#include "link.h"

Link::Link(bool isserver)
{
    sock = -1;
    noblock_ = false; //非阻塞
    error_ = false; //错误标志
    remote_ip[0] = '\0';
    remote_port = -1;
}

Link::~Link()
{
    close();
}

Link::close()
{
    if(sock >= 0)
    {
        ::close(sock);
        sock = -1;
    }
}

void Link::nodelay(bool enable)
{
    int opt = enable ? 1:0;
    ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
}

void Link::keepalive(bool enable)
{
    int opt = enable ? 1:0;
    ::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
}

void Link::noblock(bool enable)
{
    noblock_ = enable;
    if(enable) {
        ::fcntl(sock, F_SETFL, O_NONBLOCK | O_RDWR);
    } else {
        ::fcntl(sock, F_SETFL, O_RDWR);
    }
}

Link *Link::connect(const char *ip, int port)
{
    int sock = -1;

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    //创建套接字
    if((sock = ::socket(AF_INET, SOCK_STREAM, 0))== -1) {
        goto sock_err;
    }

    //连接套接字
    if(::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        goto sock_err;
    }

    Link link;
    link.sock = sock;
    link.keepalive(true);
    return &link;

sock_err:
    if(sock >= 0) {
        ::close(sock);
    }
    return NULL;
}

Link *Link::connect(const string &ip, int port)
{
    return connect(ip.c_str(), port);
}

Link *Link::listen(const char *ip, int port)
{
    int sock = -1;
    int opt = 1;

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    //创建套接字
    if((sock = ::socket(AF_INET, SOCK_STREAM, 0))== -1) {
        goto listen_err;
    }
    if(::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt)) == -1) {
        goto listen_err;
    }
    if(::bind(sock , (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        goto listen_err;
    }
    if(::listen(sock, 1024) == -1) {
        goto listen_err;
    }

    Link *link = new Link(true);
    link->sock = sock;
    snprintf(link->remote_ip, sizeof(link->remote_ip), "%s", ip);
    link->remote_port = port;

listen_err:
    if(sock >= 0)
        ::close(sock);
    return NULL;
}

Link *Link::listen(const string &ip, int port)
{
    return listen(ip.c_str(), port);
}

Link *Link::accept()
{
    int client_sock; //客户端套接字
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    while((client_sock = ::accept(sock, (struct sockaddr *)&addr, &addrlen)) == -1) {
        if(errno != EINTR)
            return NULL;
    }

    struct linger opt = {1, 0};
    int ret = ::setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (void *)&opt, sizeof(opt));
    if(ret) {
        //set linger failed!
    }

    Link *link = new Link();
    link->sock = client_sock;
    link->keepalive(true);
    inet_ntop(AF_INET, &addr.sin_addr, link->remote_ip, sizeof(link->remote_ip));
    link->remote_port = ntohs(addr.sin_port);
    return link;
}

//读套接字中的数据
int Link::read()
{
    int ret = 0;
    char buf[16 * 1024];
    while(1) {
        int len = ::read(sock, buf, sizeof(buf));
        if(len == -1) {
            if(errno == EINTR)
                continue;
            else if(errno == EWOULDBLOCK)
                break;
            else
                return -1;
        }
        else if(!len) {
            return 0;
        }
        else {
            ret += len;
            decoder_.push(buf, len);
        }
        if(!noblock)
            break; //非阻塞
    }
    return ret;
}

//向套接字中写数据
int Link::write()
{
#if 0
    int ret = 0;
    int want;
    while((want = output.size()) > 0) {
        int len = ::write(sock, output.data(), want);
        if(len == -1) {
            if(errno == EINTR)
                continue;
            else if(errno == EWOULDBLOCK)
                break;
            else
                return -1;
        }
        else if(0 == len) {
            break;
        }
        else {
            ret += len;
            output = std::string()
            decoder_.push(buf, len);
        }
        if(!noblock)
            break; //非阻塞
    }
    return ret;
#endif
}

//将output的内容全部写入套接字
int Link::flush()
{
//冲刷

}

//调用Decoder的parse接口处理消息
int Link::recv(Message *msg)
{
//收信息
    
}

//将消息添加到output中(添加之前需要先对消息进行编码)
int send(const Message &msg)
{
//发信息

}

