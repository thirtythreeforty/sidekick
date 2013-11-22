/*
 * File:   tipacket.c
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 7:38 PM
 */

#include "variable.h"


#include "tilink.h"
#include "tipacket.h"

#include "pic24_all.h"

#include <stdio.h>

enum {
    idle = 0,
    needAck
} state;

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
unsigned char packetfifo_PopByte(void)
{
    unsigned char byte;
    while(datafifo.goodFront == datafifo.back)
        ;//asm("pwrsav #1");
    byte = datafifo.data[datafifo.goodFront];
    if(++datafifo.goodFront == sizeof(datafifo.data))
        datafifo.goodFront = 0;
    return byte;
}
inline unsigned int packetfifo_Size()
{
    return (datafifo.back - datafifo.goodFront) % sizeof(datafifo.data);
}
inline void packetfifo_MarkGood(void)
{
    datafifo.goodFront = datafifo.front;
}
inline void packetfifo_MarkBad(void)
{
    datafifo.front = datafifo.goodFront;
}

void getTIPacket(void)
{
    unsigned int i, packetSize, dataChecksum, computedChecksum = 0;
    unsigned char unit, command;
    // Packet header
    unit = TIfifo_getByte();
    if(unit != 0x98)                        // Sending unit (0x98 : TI-89)
        // Paranoia, only accept TI-89 packets for now
        error_and_reset();
    command = TIfifo_getByte();
    packetSize = TIfifo_getByte();          // Data size, low byte
    packetSize |= TIfifo_getByte() << 8;    // Data size, high byte
    for(i = 0; i < packetSize; ++i) {       // Data
        unsigned char in = TIfifo_getByte();
        packetfifo_PushByte(in);
        computedChecksum += in;
    }
    dataChecksum = TIfifo_getByte();        // Checksum, low byte
    dataChecksum |= TIfifo_getByte() << 8;  // Checksum, high byte

    // Send ack
    if(dataChecksum == computedChecksum) {
        puts("Packet good.\n");
        packetfifo_MarkGood();
        sendTIAck(unit, ACK);
        sendTIPacketReply(unit, command); // Hardcoded for now.
    }
    else {
        puts("Packet bad.\n");
        packetfifo_MarkBad();
        sendTIAck(unit, ERR);
    }
}

void sendTIPacket(unsigned char unit, unsigned char command, const unsigned char *data, unsigned int size)
{
    unsigned char ok = 0;
    do {
        unsigned int i, checksum = 0;

        // Send our data
        setTIlinkMode(send);
        TIfifo_addByte(unit);
        TIfifo_addByte(command);
        for(i = 0; i < size; ++i) {
            TIfifo_addByte(data[i]);
            checksum += data[i];
        }
        if(size > 0) {
            TIfifo_addByte(checksum & 0xFF);
            TIfifo_addByte(checksum >> 8);
        }

        // Get their ACK
        setTIlinkMode(receive);
        TIfifo_getByte(); // Unit type, don't care
        if(TIfifo_getByte() == ACK)
            // Golden.
            ok = 1;
        TIfifo_getByte(); // Size, should be zero.
        TIfifo_getByte(); // But don't really care.
    } while(!ok);
}

void sendTIAck(unsigned char unit, PacketType ack)
{
    setTIlinkMode(send);
    TIfifo_addByte(unit);
    TIfifo_addByte(ack);
    TIfifo_addByte(0x00);
    TIfifo_addByte(0x00);
    setTIlinkMode(receive);
}

void sendTIPacketReply(unsigned char unit, unsigned char command)
{
    static unsigned char receiving = 0;

    // Decide what to do, based on state and command byte.
    if(receiving == 1 && (command == 0x92 || command == 0x06)) { // EOT, VAR
        variableCommit();
        receiving = 0;
        // This will fall through to the next if-block if command == 0x06.
    }
    if(command == 0x06) { // VAR
        if(variableVerifyAndInit(unit)) {
            const unsigned char MEMpacket[] = {0x05, 0x00, 0x01, 0x00, 0x00};
            sendTIPacket(unit, 0x36, MEMpacket, sizeof(MEMpacket));
        }
        else {
            sendTIPacket(unit, 0x09, (void*)0, 0);
            receiving = 1;
        }
    }
}