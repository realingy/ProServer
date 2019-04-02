/**
 * @file link.h
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年02月27日 09时28分36秒
 */

#ifndef __LINK_H__
#define __LINK_H__

#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define LEN 1024

    class Link
    {
    public:
        std::string output;
        char remote_ip[LEN]; //ip地址
        int remote_port; //端口
        double create_time; //
        double active_time; //
    
        ~Link();
        void close();
//        void nodelay(bool enable=true);
//        void noblock(bool enable=true);
//        void keepalive(bool enable=true);
    
        int fd() const {
            return sock;
        }

        bool error() const {
            return error_;
        }

        void mark_error() {
            error_ = true;
        }
   
    //static Link *connect(const char *ip, int port);
    //static Link *connect(const string &ip, int port);

        static Link *listen(const char *ip, int port);
        static Link *listen(const std::string &ip, int port);
    //Link *accept();
        int accept();
//        int read();

#if 0
        int write();
        int flush(); //冲刷
        int recv(Message *msg); //收信息
        int send(const Message &msg); //发信息
#endif

        int getSock() {
            return sock;
        }
    private:
        int sock;
        bool noblock_;
        bool error_;
//        Decoder decoder_;
        Link(bool is_server = false);
    };


#endif //__LINK_H__

