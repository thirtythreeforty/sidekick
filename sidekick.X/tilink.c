/*
 * File:   tilink.c
 * Author: George Hilliard
 *
 * Created on November 11, 2013, 3:27 PM
 */

#include "pic24_all.h"

#include "tilink.h"

typedef enum {
    floating = 0,
    ring,
    tip
} pin;

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

volatile TIfifo_tag TIfifo = {.front = 0, .back = 0, .bits = 0};

void TIfifo_addBit(unsigned char newbit)
{
    TIfifo.shiftbyte = (TIfifo.shiftbyte >> 1) | (newbit << 7);
    if((TIfifo.bits += 1) == 8) {
        TIfifo.data[TIfifo.back++] = TIfifo.shiftbyte;
        if(TIfifo.back == sizeof(TIfifo.data))
            TIfifo.back = 0;
        if(TIfifo.back == TIfifo.front) // Detect overflow
            error_and_reset();
        TIfifo.bits = 0;
    }
}
unsigned char TIfifo_getBit(void)
{
    unsigned char b;
    if(TIfifo.bits-- == 0) {
        if(++TIfifo.front == sizeof(TIfifo.data))
            TIfifo.front = 0;
        TIfifo.bits = 8;
    }
    if(TIfifo.front == TIfifo.back)
        error_and_reset();
    b = TIfifo.shiftbyte & 1;
    TIfifo.shiftbyte >>= 1;
    return b;
}

unsigned char TIfifo_getByte(void) {
    unsigned char byte;
    while(TIfifo.front == TIfifo.back)
        ;//asm("pwrsav #1"); // This function should not be called from an interrupt.
    byte = TIfifo.data[TIfifo.front++];
    if(TIfifo.front == sizeof(TIfifo.data))
        TIfifo.front = 0;
    return byte;
}
void TIfifo_addByte(unsigned char byte) {
    while(TIfifo.front == (TIfifo.back + 1) % (sizeof(TIfifo.data) - 1))
        ;//asm("pwrsav #1"); // This function should not be called from an interrupt.
    TIfifo.data[TIfifo.back] = byte;
    if(++TIfifo.back == sizeof(TIfifo.data))
        TIfifo.back = 0;
}

void _ISRFAST _CNInterrupt(void) {
    static pin state = floating;

    if(TIPPIN == 0 && state == floating) {
        CONFIG_RING_AS_OUTPUT();
        RINGLATCH = 0;
        state = tip;
        TIfifo_addBit(0);
    }
    else if(RINGPIN == 0 && state == floating) {
        CONFIG_TIP_AS_OUTPUT();
        TIPLATCH = 0;
        state = ring;
        TIfifo_addBit(1);
    }
    else if(state == tip && TIPPIN) {
        state = floating;
        RINGLATCH = 1;
        CONFIG_RING_AS_INPUT();
    }
    else if(state == ring && RINGPIN) {
        state = floating;
        TIPLATCH = 1;
        CONFIG_TIP_AS_INPUT();
    }
    else {
        // No clue what happened, let's signal an error and get out of here.
        error_and_reset();
    };

    _CNIF = 0;
}

void configTIlink()
{
    // Configure tip
    CONFIG_TIP_AS_INPUT();
    ENABLE_RB5_PULLUP();
    ENABLE_RB5_OPENDRAIN();
    ENABLE_TIP_INTERRUPT();

    // Configure ring
    CONFIG_RING_AS_INPUT();
    ENABLE_RB6_PULLUP();
    ENABLE_RB6_OPENDRAIN();
    ENABLE_RING_INTERRUPT();

    TIPLATCH = 1;
    RINGLATCH = 1;

    // Configure interrupt
    _CNIF = 0;                       //clear interrupt flag
    _CNIP = 1;                       //choose a priority
    _CNIE = 1;                       //enable the interrupt
}

void error_and_reset()
{
    CONFIG_RING_AS_OUTPUT();
    CONFIG_TIP_AS_OUTPUT();
    // Signal an error
    RINGLATCH = 0;
    TIPLATCH = 0;
    DELAY_US(600);

    asm("RESET");
}
