/*
 * File:   tipacket.c
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 7:38 PM
 */

#include "pic24_all.h"
#include "common.h"
#include "variable.h"
#include "tilink.h"
#include "tipacket.h"

enum {
    idle = 0,
    needAck
} state;

volatile struct {
    unsigned char data[0x3500];
    unsigned int front;
    unsigned int back;
    unsigned int goodBack;
} datafifo __attribute__((far)) = {.front = 0, .back = 0, .goodBack = 0};

void packetfifo_PushByte(unsigned char byte)
{
    while(datafifo.front == (datafifo.back + 1) % (sizeof(datafifo.data) - 1))
        ;
    datafifo.data[datafifo.back] = byte;
    if(++datafifo.back == sizeof(datafifo.data))
        datafifo.back = 0;
}
unsigned char packetfifo_PopByte(void)
{
    unsigned char byte;
    while(datafifo.goodBack == datafifo.front)
        ;
    byte = datafifo.data[datafifo.front];
    if(++datafifo.front == sizeof(datafifo.data))
        datafifo.front = 0;
    return byte;
}
inline unsigned int packetfifo_Size()
{
    return (datafifo.goodBack - datafifo.front) % sizeof(datafifo.data);
}
inline void packetfifo_MarkGood(void)
{
    datafifo.goodBack = datafifo.back;
}
inline void packetfifo_MarkBad(void)
{
    datafifo.back = datafifo.goodBack;
}

void getTIPacket(void)
{
    unsigned int i, packetSize, dataChecksum, computedChecksum = 0;
    unsigned char unit, command;
    debug("packetfifo size is %i\n", packetfifo_Size());
    // Packet header
    unit = TIfifo_getByte();                // Sending unit (0x98: TI-89, 0x89: TI-89 Titanium)
    command = TIfifo_getByte();
    debug("command = 0x%02x; ", command);
    packetSize = TIfifo_getByte();          // Data size, low byte
    packetSize |= TIfifo_getByte() << 8;    // Data size, high byte
    debug("packet size = 0x%04x; ", packetSize);
    for(i = 0; i < packetSize; ++i) {       // Data
        unsigned char in = TIfifo_getByte();
        packetfifo_PushByte(in);
        computedChecksum += in;
    }
    if(packetSize) {
        dataChecksum = TIfifo_getByte();        // Checksum, low byte
        dataChecksum |= TIfifo_getByte() << 8;  // Checksum, high byte
    }

    // Send ack
    if(dataChecksum == computedChecksum || !packetSize) {
        debug("Packet good.\n");
        packetfifo_MarkGood();

        debug("packetfifo size is %i\n", packetfifo_Size());

        sendTIAck(unit, ACK);
        sendTIPacketReply(unit, command);
    }
    else {
        debug("Packet bad.\n");
        packetfifo_MarkBad();
        sendTIAck(unit, ERR);
    }
}

void sendTIPacket(unsigned char unit, unsigned char command, const unsigned char *data, unsigned int size)
{
    unsigned char ok = 0;
    do {
        debug("Sending TI packet");
        unsigned int i, checksum = 0;

        // Send our data
        setTIlinkMode(send);
        TIfifo_addByte(unit);
        TIfifo_addByte(command);
        TIfifo_addByte(size & 0xFF);
        TIfifo_addByte(size >> 8);
        debug("....");
        for(i = 0; i < size; ++i) {
            TIfifo_addByte(data[i]);
            checksum += data[i];
            debug(".");
        }
        if(size > 0) {
            TIfifo_addByte(checksum & 0xFF);
            TIfifo_addByte(checksum >> 8);
            debug("..");
        }

        // Get their ACK
        setTIlinkMode(receive);
        ok = getTIAck();
    } while(!ok);
    debug("Sent&ACKd.\n");
}

void sendTIAck(unsigned char unit, PacketType ack)
{
    debug("Sending ack %02x.\n", ack);
    setTIlinkMode(send);
    TIfifo_addByte(unit);
    TIfifo_addByte(ack);
    TIfifo_addByte(0x00);
    TIfifo_addByte(0x00);
    setTIlinkMode(receive);
}

unsigned char getTIAck(void)
{
    unsigned char ok;
    TIfifo_getByte(); // Unit type, don't care
    ok = (TIfifo_getByte() == ACK);
    if(TIfifo_getByte() || TIfifo_getByte())
        // Size should be zero.
        error_and_reset();
    return ok;
}
void sendTIPacketReply(unsigned char unit, unsigned char command)
{
    static unsigned char receiving = 0;

    debug("Replying (command byte was %02x).\n", command);

    // Decide what to do, based on state and command byte.
    if(receiving == 1 && (command == 0x92 || command == 0x06)) { // EOT, VAR
        debug("EOT or VAR received. Committing variable.\n");
        variableCommit();
        receiving = 0;
        // This will fall through to the next if-block if command == 0x06.
    }
    if(command == 0x06) { // VAR
        debug("Sending reply ");
        if(variableVerifyAndInit(unit)) {
            const unsigned char MEMpacket[] = {0x05, 0x00, 0x01, 0x00, 0x00};
            debug("MEM\n");
            sendTIPacket(unit, 0x36, MEMpacket, sizeof(MEMpacket));
        }
        else {
            debug("CTS\n");
            sendTIPacket(unit, 0x09, (void*)0, 0);
            receiving = 1;
        }
    }
    debug("Done replying.\n");
}