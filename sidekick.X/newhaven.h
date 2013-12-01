/* 
 * File:   newhaven.h
 * Author: George Hilliard
 *
 * Created on November 30, 2013, 9:55 PM
 */

#ifndef NEWHAVEN_H
#define	NEWHAVEN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "pic24_all.h"

#define NEWHAVEN_DELAY 70
#define NEWHAVEN_SPEED 100

void Newhaven_Print(const char* str);
void Newhaven_SendCode(unsigned char byte, unsigned char arg);

#define Newhaven_On(x)                Newhaven_Print("\xFE\x41")
#define Newhaven_Off(x)               Newhaven_Print("\xFE\x42")
#define Newhaven_SetCursor(x)         Newhaven_SendCode(0x45, (x))
#define Newhaven_Line1                0x00
#define Newhaven_Line2                0x40
#define Newhaven_CursorHome(x)        Newhaven_Print("\xFE\x46")
#define Newhaven_UnderlineCursorOn()  Newhaven_Print("\xFE\x47")
#define Newhaven_UnderlineCursorOff() Newhaven_Print("\xFE\x48")
#define Newhaven_CursorLeft(x)        Newhaven_Print("\xFE\x49")
#define Newhaven_CursorRight(x)       Newhaven_Print("\xFE\x4A")
#define Newhaven_BlinkingCursorOn(x)  Newhaven_Print("\xFE\x4B")
#define Newhaven_BlinkingCursorOff(x) Newhaven_Print("\xFE\x4C")
#define Newhaven_Backspace(x)         Newhaven_Print("\xFE\x4E")
#define Newhaven_Clear(x)             Newhaven_Print("\xFE\x51")
#define Newhaven_SetContrast(x)       Newhaven_SendCode(0x52, (x))
#define Newhaven_SetBrightness(x)     Newhaven_SendCode(0x53, (x))
#define Newhaven_ScrollLeft(x)        Newhaven_Print("\xFE\x55")
#define Newhaven_ScrollRight(x)       Newhaven_Print("\xFE\x56")


#ifdef	__cplusplus
}
#endif

#endif	/* NEWHAVEN_H */

