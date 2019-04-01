/**
 * @file decoder.h
 * @brief  
 * @author  
 * @version 1.0
 * @date 2019年04月01日 17时21分59秒
 */

#ifndef __DECODER_H__
#define __DECODER_H__

namespace Sim {
    class Decoder {
    public:
        Decoder();
        int push(const char *buf, int len);
        int parse(Message *msg);

    private:
        std::string buffer;
        int buffer_offset;
    };
}; //namespace Sim

#endif //__DECODER_H__
