/*
 * File:   newhaven.c
 * Author: George Hilliard
 *
 * Created on November 30, 2013, 9:55 PM
 */

#include "newhaven.h"

void Newhaven_Print(const char* str)
{
    unsigned char c;
    unsigned int i = 0;

    unsigned int oldI2Cspeed = I2C1BRG;
    configI2C1(NEWHAVEN_SPEED);

    startI2C1();
    putI2C1(0x50);
    while((c = str[i++]) != 0) {
        DELAY_US(NEWHAVEN_DELAY);
        putI2C1(c);
    }
    DELAY_US(NEWHAVEN_DELAY);
    stopI2C1();

    I2C1BRG = oldI2Cspeed;
}

inline void Newhaven_SendCode(unsigned char byte, unsigned char arg)
{
    const char str[] = {0xFE, byte, arg, 0};
    Newhaven_Print(str);
}
