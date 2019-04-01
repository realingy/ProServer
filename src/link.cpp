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
    m_bNoblock = false; //非阻塞
    m_error = false; //错误标志
    remote_ip = '\0';
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
    m_bNoblock = enable;
    if(enable) {
        ::fcntl(sock, F_SETFL, O_NONBLOCK | O_RDWR);
    } else {
        ::fcntl(sock, F_SETFL, O_RDWR);
    }
}

Link *Link::connect(const char *ip, int port)
{
    //return connect(ip.c_str(), port);
}

Link *Link::connect(const string &ip, int port)
{
    return connect(ip.c_str(), port);
}

Link *Link::listen(const char *ip, int port)
{

}

Link *Link::listen(const string &ip, int port)
{
    return listen(ip.c_str(), port);
}

Link *Link::accept()
{

}

int Link::read()
{

}

int Link::write()
{

}

int Link::flush()
{
//冲刷

}

int Link::recv(Message *msg)
{
//收信息
    
}

int send(const Message &msg)
{
//发信息

}

