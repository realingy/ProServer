/**
 * @file selectserver.h
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月27日 09时15分28秒
 */

#ifndef __SELECT_SERVER_H__
#define __SELECT_SERVER_H__

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

#define DEFAULT_PORT 6666
#define LISNUM 10

class SelectServer {

int main() {

private:
    int m_serverfd;

public:
    int bind_and_listen() {

        if((m_serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            return -1;
        }

        printf("socket ok \n");

        unsigned int sin_size, myport=6666 , lisnum=10;
        struct sockaddr_in my_addr; /*本机地址信息*/
        my_addr.sin_family=AF_INET;
        my_addr.sin_port=htons(DEFAULT_PORT);
        my_addr.sin_addr.s_addr=INADDR_ANY;
        bzero(&(my_addr.sin_zero), 0);

        if(bind(m_serverfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
            perror("bind error!");
            return -2;
        }
    
        if(listen(m_serverfd, lisnum) == -1) {
            perror("listen error!");
            return -3;
        }

    }



    int acceptfd; /*监听socket:serverfd，数据传输socket:acceptfd*/
    struct sockaddr_in their_addr; /*客户地址信息*/



    fd_set client_fdset; /*监控文件描述符集合*/
    int maxsock; /*最大文件描述符*/
    struct timeval timeout; /*超时结构体*/
    int client_sockfd[5]; /*存放活动的sockfd*/
    bzero((void*)client_sockfd, sizeof(client_sockfd));
    int conn_amount = 0; /*记录描述符数量*/
    maxsock = serverfd;
    char buffer[1024];
    int ret = 0;

    while(1) {
        FD_ZERO(&client_fdset); /*清空描述符集*/
        FD_SET(serverfd, &client_fdset); /*添加服务器描述符*/
        timeout.tv_sec = 30; /*30秒*/
        timeout.tv_usec = 0;

        for (int i = 0; i < 5; i++) {
            if(client_sockfd[i] != 0) {
                FD_SET(client_sockfd[i], &client_fdset); /*添加活动的套接字*/
            }
        }

        ret = select(maxsock+1, &client_fdset, NULL, NULL, &timeout);
        if(ret < 0) {
            perror("select error!");
            break;
        } else if(0 == ret) {
            printf("time out!\n");
            continue;
        }

        /*轮询各个文件描述符*/
        for(int i = 0; i < conn_amount; ++i) {
            /* FD_ISSET 检查 client_sockfd 是否可读写，>0可读写*/
            if(FD_ISSET(client_sockfd[i], &client_fdset)) {
                printf("start recv from client[%d]:\n", i);
                ret = recv(client_sockfd[i], buffer, 1024, 0);
                if(ret <= 0) {
                    printf ("client[%d] close\n", i);
                    close(client_sockfd[i]);
                    FD_CLR(client_sockfd[i], &client_fdset);
                    client_sockfd[i] = 0;
                } else {
                    printf("recv from client[%d]:%s\n", i, buffer);
                }
            }
        }

        /*检查是否有新的连接，接受连接并添加到client_sockfd中*/
        if(FD_ISSET(serverfd, &client_fdset)) {
            /*接受连接*/
            struct sockaddr_in client_addr;
            size_t size= sizeof(struct sockaddr_in);
            int sock_client = accept(serverfd, (struct sockaddr*)(&client_addr), (unsigned int*) (&size));
            if(sock_client < 0) {
                perror("accept error!\n");
                continue;
            }

        /*把连接添加到文件描述符集*/
        if(conn_amount < 5) {
            client_sockfd[conn_amount++] = sock_client;
            bzero(buffer, 1024);
            strcpy(buffer, "this is server! welcome!\n");
            send(sock_client, buffer, 1024, 0); 
            printf ("new connection client[%d] %s:%d\n", conn_amount, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            bzero(buffer, sizeof(buffer));
            ret = recv(sock_client, buffer, 1024, 0);
            if(ret < 0) {
                perror("recv error!\n");
                close(serverfd);
                return -1;
            }

            printf("recv : %s\n", buffer);
            if(sock_client > maxsock) {
                maxsock = sock_client;
            } else {
                printf("max connections! ! !quit ! !\n");
                break;
            }
        }
        }
    }

    for(int i = 0; i < 5; ++i) {
        if(client_sockfd[i] != 0) {
            close(client_sockfd[i]);
        }
    }

    close(serverfd);
    return 0;
}


};

#endif //__SELECT_SERVER_H__
