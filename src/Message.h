#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <map>
#include <string>

namespace Sim {
    class Message {
    public:
        Message();
        std::string encode() const;

    private:
        std::string type;
        std::map<int, std::string> msg;
        friend class Decoder();
    };
}; //namespace Sim

#endif //__MESSAGE_H__
