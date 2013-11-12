/*
 * File:   tipacket.c
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 7:38 PM
 */

#include "tilink.h"
volatile extern TIfifo_tag TIfifo;


enum {
    idle = 0,
    needAck
} state;

volatile struct {
    unsigned char data[0x3500];
    unsigned int front;
    unsigned int back;
} datafifo __attribute__((far)) = {.front = 0, .back = 0};

void packetfifo_PushByte(unsigned char byte)
{
    while(datafifo.front == (datafifo.back + 1) % (sizeof(datafifo.data) - 1))
        ;//asm("pwrsav #1");
    datafifo.data[datafifo.back++] = byte;
    if(datafifo.back == sizeof(datafifo.data))
        datafifo.back = 0;
}

unsigned char getTIPacket()
{
    unsigned int i, packetSize, dataChecksum, computedChecksum = 0;
    while(1) {
        // Packet header
        if(TIfifo_getByte() != 0x98)            // Sending unit (0x98 : TI-89)
            // Paranoia, only accept TI-89 packets for now
            error_and_reset();
        if(TIfifo_getByte() != 0x06)            // Command (0x06 : VAR)
            error_and_reset();
        packetSize = TIfifo_getByte();          // Data size, low byte
        packetSize |= TIfifo_getByte() << 8;    // Data size, high byte
        for(i = 0; i < packetSize; ++i) {       // Data
            unsigned char in = TIfifo_getByte();
            packetfifo_PushByte(in);
            computedChecksum += in;
        }
        dataChecksum = TIfifo_getByte();        // Checksum, low byte
        dataChecksum |= TIfifo_getByte() << 8;  // Checksum, high byte
        if(dataChecksum == computedChecksum)
            ;
        else
            ;
        return 1; // CONT packets not implemented yet
    }
}

void sendTIPacketReply()
{
}