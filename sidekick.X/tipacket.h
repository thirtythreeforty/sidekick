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
    
typedef enum {
    ACK = 0x56,
    ERR = 0x5A
} PacketType;

void getTIPacket();
void sendTIPacketReply(unsigned char unit, unsigned char command);
void packetfifo_PushByte(unsigned char byte);
unsigned char packetfifo_PopByte();
unsigned int packetfifo_Size(void);
void sendTIAck(unsigned char unit, PacketType ack);

#ifdef	__cplusplus
}
#endif

#endif	/* TIPACKET_H */
