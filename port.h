#ifndef __PORT_H
#define __PORT_H

#include "types.h"

    class Port
    {
    protected:
        uint16_t portnumber;
        Port(uint16_t portnumber);
        ~Port();
    };

    class Port8bit : public Port
    {
    public:
        Port8bit(uint16_t portnumber);
        ~Port8bit();
        virtual void Write(uint8_t data);
        virtual uint8_t Read();
    };

    // Why a slow version of 8bit?
    // for synchronization
    class Port8bitSlow : public Port8bit
    {
    public:
        Port8bitSlow(uint16_t portnumber);
        ~Port8bitSlow();
        virtual void Write(uint8_t data);
    };


    class Port16bit : public Port
    {
    public:
        Port16bit(uint16_t portnumber);
        ~Port16bit();
        virtual void Write(uint16_t data);
        virtual uint16_t Read();
    };

    class Port32bit : public Port
    {
    public:
        Port32bit(uint16_t portnumber);
        ~Port32bit();
        virtual void Write(uint32_t data);
        virtual uint32_t Read();
    };

#endif