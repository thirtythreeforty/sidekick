/* 
 * File:   common.h
 * Author: George Hilliard
 *
 * Created on November 23, 2013, 7:16 PM
 */

#ifndef COMMON_H
#define	COMMON_H

#ifdef	__cplusplus
extern "C" {
#endif

// FEATURE #DEFINES

// Define DEBUG to enable UART logging (degrades performance!).
//#define DEBUG

// Define USE_DISPLAY to enable feedback on Newhaven I2C display.
#define USE_DISPLAY

// Define the appropriate macros to match the EEPROM chip(s) used.
#define NUM_EEPROMS 3
//#define USE_515
#define USE_1025
//#define USE_LC
#define USE_FC

#ifdef DEBUG
#include <stdio.h>
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_H */

