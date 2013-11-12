/* 
 * File:   tilink.h
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 3:28 PM
 */

#ifndef TILINK_H
#define	TILINK_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char data[0x100];
    unsigned int front;
    unsigned int back;
    unsigned char bits;
    unsigned char shiftbyte;
} TIfifo_tag;

void configTIlink();
void error_and_reset();

void TIfifo_addByte(unsigned char);
unsigned char TIfifo_getByte();

#ifdef	__cplusplus
}
#endif

#endif	/* TILINK_H */
