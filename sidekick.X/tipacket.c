/*
 * File:   tipacket.c
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 7:38 PM
 */

#include "tilink.h"
#include "tipacket.h"

#include "pic24_all.h"

enum {
    idle = 0,
    needAck
} state;

typedef enum {
    ACK = 0x56,
    ERR = 0x5A
} PacketType;

volatile struct {
    unsigned char data[0x3500];
    unsigned int front;
    unsigned int goodFront;
    unsigned int back;
} datafifo __attribute__((far)) = {.front = 0, .back = 0, .goodFront = 0};

void packetfifo_PushByte(unsigned char byte)
{
    while(datafifo.front == (datafifo.back + 1) % (sizeof(datafifo.data) - 1))
        ;//asm("pwrsav #1");
    datafifo.data[datafifo.back] = byte;
    if(++datafifo.back == sizeof(datafifo.data))
        datafifo.back = 0;
}
inline void packetfifo_MarkGood(void)
{
    datafifo.goodFront = datafifo.front;
}
inline void packetfifo_MarkBad(void)
{
    datafifo.front = datafifo.goodFront;
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

        // Send reply
        if(dataChecksum == computedChecksum) {
            packetfifo_MarkGood();
            sendTIAck(ACK);
        }
        else {
            packetfifo_MarkBad();
            sendTIAck(ERR);
        }

        return 1; // CONT packets not implemented yet
    }
}

void sendTIAck(PacketType ack)
{
    setTIlinkMode(send);
    TIfifo_addByte(0x98); // Always act as TI-89 for now
    TIfifo_addByte(ack);
    TIfifo_addByte(0x00);
    TIfifo_addByte(0x00);
    setTIlinkMode(receive);
}

void sendTIPacketReply()
{
}