/* 
 * File:   main.c
 * Author: georgev
 *
 * Created on November 10, 2013, 12:53 AM
 */

#include <stdio.h>
#include <stdlib.h>

#include "pic24_all.h"

typedef enum {
    floating = 0,
    ring,
    tip
} pin;

#define TIPPIN _RB5
#define TIPLATCH _LATB5
#define CONFIG_TIP_AS_OUTPUT() CONFIG_RB5_AS_DIG_OUTPUT()
#define CONFIG_TIP_AS_INPUT()  CONFIG_RB5_AS_DIG_INPUT()

#define RINGPIN _RB6
#define RINGLATCH _LATB6
#define CONFIG_RING_AS_OUTPUT() CONFIG_RB6_AS_DIG_OUTPUT()
#define CONFIG_RING_AS_INPUT()  CONFIG_RB6_AS_DIG_INPUT()

struct {
    unsigned char *front;
    unsigned char *back;
    unsigned char data[256];
    unsigned char bits;
    unsigned char shiftbyte;
} TIfifo;

void TIfifo_addBit(unsigned char newbit)
{
    // No overflow detection is performed!
    TIfifo.shiftbyte = (TIfifo.shiftbyte << 1) | newbit;
    if((TIfifo.bits += 1) == 8) {
        *TIfifo.back = TIfifo.shiftbyte;
        if(++TIfifo.back == TIfifo.data + sizeof(TIfifo.data))
            TIfifo.back = &TIfifo.data;
        TIfifo.bits = 0;
    }
}

void _ISRFAST _CNInterrupt(void) {
    static pin state = floating;

    if(TIPPIN == 0 && state == floating) {
        CONFIG_RING_AS_OUTPUT();
        RINGLATCH = 0;
        state = tip;
        TIfifo_addBit(0);
#ifndef NDEBUG
        putchar('0');
#endif
    }
    else if(RINGPIN == 0 && state == floating) {
        CONFIG_TIP_AS_OUTPUT();
        TIPLATCH = 0;
        state = ring;
        TIfifo_addBit(1);
#ifndef NDEBUG
        putchar('1');
#endif
    }
    else if(state == tip && TIPPIN) {
        state = floating;
        RINGLATCH = 1;
        CONFIG_RING_AS_INPUT();
    }
    else if(state == ring && RINGPIN) {
        state = floating;
        TIPPIN = 1;
        CONFIG_TIP_AS_INPUT();
    }
    else {
        putchar('X');
        asm("RESET");
    };

    _CNIF = 0;
}

void configTIlink()
{
    // Configure tip
    CONFIG_RB5_AS_DIG_INPUT();
    DISABLE_RB5_PULLUP();
    ENABLE_RB5_OPENDRAIN();
    ENABLE_RB5_CN_INTERRUPT();

    // Configure ring
    CONFIG_RB6_AS_DIG_INPUT();
    DISABLE_RB6_PULLUP();
    ENABLE_RB6_OPENDRAIN();
    ENABLE_RB6_CN_INTERRUPT();

    _CNIF = 0;                       //clear interrupt flag
    _CNIP = 1;                       //choose a priority
    _CNIE = 1;                       //enable the interrupt
}

int main(int argc, char** argv) {
    configBasic(HELLO_MSG);

    configTIlink();

    while(1) {
        doHeartbeat();
    }
}

