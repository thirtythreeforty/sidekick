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
    ring,
    tip
} pin;

void _ISRFAST _CNInterrupt(void) {
    static pin state = 0;

    _CNIF = 0;
}

void configTIlink()
{
    // Configure tip
    CONFIG_RB5_AS_DIG_INPUT();
    ENABLE_RB5_PULLUP();
    ENABLE_RB5_OPENDRAIN();
    ENABLE_RB5_CN_INTERRUPT();

    // Configure ring
    CONFIG_RB6_AS_DIG_INPUT();
    ENABLE_RB6_PULLUP();
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

