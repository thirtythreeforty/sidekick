/*
 * File:   tilink.c
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 3:27 PM
 */

#include "pic24_all.h"
#include "common.h"
#include "tilink.h"

typedef enum {
    floating = 0,
    ring,
    tip
} pin;

volatile struct {
    unsigned char data[0x200];
    unsigned int front;
    unsigned int back;
    unsigned char bits;
    unsigned char shiftbyte;
    TIlinkMode mode;
    pin state;
} TIfifo = {.front = 0, .back = 0, .bits = 0, .mode = receive, .state = floating};

#define TIPPIN _RB5
#define TIPLATCH _LATB5
#define CONFIG_TIP_AS_OUTPUT() CONFIG_RB5_AS_DIG_OUTPUT()
#define CONFIG_TIP_AS_INPUT()  CONFIG_RB5_AS_DIG_INPUT()
#define ENABLE_TIP_INTERRUPT() ENABLE_RB5_CN_INTERRUPT()

#define RINGPIN _RB6
#define RINGLATCH _LATB6
#define CONFIG_RING_AS_OUTPUT() CONFIG_RB6_AS_DIG_OUTPUT()
#define CONFIG_RING_AS_INPUT()  CONFIG_RB6_AS_DIG_INPUT()
#define ENABLE_RING_INTERRUPT() ENABLE_RB6_CN_INTERRUPT()

void TIfifo_addBit(unsigned char newbit)
{
    TIfifo.shiftbyte = (TIfifo.shiftbyte >> 1) | (newbit << 7);
    if((TIfifo.bits += 1) == 8) {
        TIfifo.data[TIfifo.back] = TIfifo.shiftbyte;
        if(++TIfifo.back == sizeof(TIfifo.data))
            TIfifo.back = 0;
        if(TIfifo.back == TIfifo.front) // Detect overflow
            error_and_reset();
        TIfifo.bits = 0;
    }
}
unsigned char TIfifo_getBit(void)
{
    unsigned char b;
    if(TIfifo.bits == 0) {
        TIfifo.shiftbyte = TIfifo.data[TIfifo.front];
        if(++TIfifo.front == sizeof(TIfifo.data))
            TIfifo.front = 0;
        TIfifo.bits = 8;
    }
    --TIfifo.bits;
    b = TIfifo.shiftbyte & 1;
    TIfifo.shiftbyte >>= 1;
    return b;
}

unsigned char TIfifo_getByte(void) {
    unsigned char byte;
    while(TIfifo.front == TIfifo.back)
        ;//asm("pwrsav #1"); // This function should not be called from an interrupt.
    byte = TIfifo.data[TIfifo.front];
    if(++TIfifo.front == sizeof(TIfifo.data))
        TIfifo.front = 0;
    return byte;
}
void TIfifo_addByte(unsigned char byte) {
    // TIlink must be in send mode or this function will not behave!
    while(TIfifo.front == (TIfifo.back + 1) % (sizeof(TIfifo.data) - 1))
        ;//asm("pwrsav #1"); // This function should not be called from an interrupt.
    TIfifo.data[TIfifo.back] = byte;
    if(++TIfifo.back == sizeof(TIfifo.data))
        TIfifo.back = 0;
    if(!_CNIE) {
        _CNIE = 1;
        _CNIF = 1;
    }
}

void setTIlinkMode(TIlinkMode mode)
{
    if(TIfifo.mode == send)
        while(_CNIE)
            // Wait for all bytes to be sent
            ;//asm("pwrsav #1");
    _CNIE = 0;
    TIfifo.mode = mode;
    TIfifo.front = TIfifo.back = 0;
    TIfifo.bits = 0;
    TIfifo.state = floating;

    TIPLATCH = 1;
    RINGLATCH = 1;
    CONFIG_TIP_AS_INPUT();
    CONFIG_RING_AS_INPUT();

    if(mode == receive)
        _CNIE = 1;
}

void _ISRFAST _CNInterrupt(void) {
    switch(TIfifo.mode) {
    case send:
        if(TIfifo.state == floating) {
            if(TIfifo.front == TIfifo.back && TIfifo.bits == 0)
                // Received our ACK but out of bits to send.
                _CNIE = 0;
            else {
                if(RINGPIN != 1 || TIPPIN != 1)
                    error_and_reset();
                // Send next bit
                if(TIfifo_getBit()) {
                    CONFIG_RING_AS_OUTPUT();
                    RINGLATCH = 0;
                    TIfifo.state = ring;
                }
                else {
                    CONFIG_TIP_AS_OUTPUT();
                    TIPLATCH = 0;
                    TIfifo.state = tip;
                }
            }
        }
        else if(TIfifo.state == tip && RINGPIN == 0) {
            TIPLATCH = 1;
            CONFIG_TIP_AS_INPUT();
            TIfifo.state = floating;
        }
        else if(TIfifo.state == ring && TIPPIN == 0) {
            RINGLATCH = 1;
            CONFIG_RING_AS_INPUT();
            TIfifo.state = floating;
        }
        else
            // No clue what happened, let's signal an error and get out of here.
            error_and_reset();
        break;
    case receive:
        if(TIPPIN == 0 && TIfifo.state == floating) {
            TIfifo_addBit(0);
            CONFIG_RING_AS_OUTPUT();
            RINGLATCH = 0;
            TIfifo.state = tip;
        }
        else if(RINGPIN == 0 && TIfifo.state == floating) {
            TIfifo_addBit(1);
            CONFIG_TIP_AS_OUTPUT();
            TIPLATCH = 0;
            TIfifo.state = ring;
        }
        else if(TIfifo.state == tip && TIPPIN) {
            TIfifo.state = floating;
            RINGLATCH = 1;
            CONFIG_RING_AS_INPUT();
        }
        else if(TIfifo.state == ring && RINGPIN) {
            TIfifo.state = floating;
            TIPLATCH = 1;
            CONFIG_TIP_AS_INPUT();
        }
        else
            // No clue what happened, let's signal an error and get out of here.
            error_and_reset();
        break;
    }

    _CNIF = 0;
}

void configTIlink()
{
    CONFIG_TIP_AS_INPUT();
    CONFIG_RING_AS_INPUT();

    // Configure tip
    ENABLE_RB5_PULLUP();
    ENABLE_RB5_OPENDRAIN();
    ENABLE_TIP_INTERRUPT();

    // Configure ring
    ENABLE_RB6_PULLUP();
    ENABLE_RB6_OPENDRAIN();
    ENABLE_RING_INTERRUPT();
    DELAY_US(1); // wait for pullups

    // Configure interrupt
    _CNIF = 0;                       //clear interrupt flag
    _CNIP = 2;                       //choose a priority
    setTIlinkMode(receive);
}

void error_and_reset()
{
    CONFIG_RING_AS_OUTPUT();
    CONFIG_TIP_AS_OUTPUT();
    // Signal an error
    RINGLATCH = 0;
    TIPLATCH = 0;
    DELAY_US(600);
    debug("Error!");
    WAIT_UNTIL_TRANSMIT_COMPLETE_UART1();

    asm("RESET");
}
