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

unsigned char getTIPacket();
void sendTIPacketReply();
void packetfifo_PushByte(unsigned char byte);
void packetfifo_PopByte(unsigned char byte);
void sendTIAck(PacketType);

#ifdef	__cplusplus
}
#endif

#endif	/* TIPACKET_H */
