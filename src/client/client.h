/**
 * @file client.h
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年04月01日 17时06分53秒
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "message.h"

namespace Sim {
    class Link;
    std::string encode(const std::string s, bool force_ascii = false);
    std::string decode(const std::string s);

    class Client {
    public:
        static Client *connect(const char * ip, int port);
        static Client *connect(const std::string &ip, int port);
        int send(const Message &msg); //blocking send
        int recv(Message *msg); //blocking recv 
        ~Client();
    private:
        Client();
        Link *link;
    };
}; //namespace Sim

#endif //__CLIENT_H__
