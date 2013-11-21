/* 
 * File:   tipacket.h
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 7:38 PM
 */

#ifndef TIPACKET_H
#define	TIPACKET_H

#ifdef	__cplusplus
extern "C" {
#endif

// 0x08 is PC, 0x98 is TI-89
#define HOSTTYPE 0x08
    
typedef enum {
    ACK = 0x56,
    ERR = 0x5A
} PacketType;

void getTIPacket();
void sendTIPacketReply(unsigned char unit, unsigned char command);
void packetfifo_PushByte(unsigned char byte);
unsigned char packetfifo_PopByte();
unsigned int packetfifo_Size(void);
void sendTIAck(PacketType ack);

#ifdef	__cplusplus
}
#endif

#endif	/* TIPACKET_H */
