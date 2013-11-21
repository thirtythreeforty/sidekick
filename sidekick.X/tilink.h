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

typedef enum {
    receive,
    send
} TIlinkMode;

void configTIlink();
void error_and_reset() __attribute__((noreturn));
void setTIlinkMode(TIlinkMode);

void TIfifo_addByte(unsigned char);
unsigned char TIfifo_getByte();

#ifdef	__cplusplus
}
#endif

#endif	/* TILINK_H */
