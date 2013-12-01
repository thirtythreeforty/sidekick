/* 
 * File:   eeprom.h
 * Author: George Hilliard
 *
 * Created on November 12, 2013, 12:55 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

#include "common.h"


#ifdef	__cplusplus
extern "C" {
#endif

// 128 bytes per EEPROM page
#ifdef USE_515
#define EEPROM_PAGE_SIZE 64
#endif
#ifdef USE_1025
#define EEPROM_PAGE_SIZE 128
#endif

#ifdef USE_LC
#define EEPROM_SPEED 400
#endif
#ifdef USE_FC
#define EEPROM_SPEED 1000
#endif

#define MAX_EEPROM_SIZE (EEPROM_PAGE_SIZE * NUM_EEPROMS * 1024)

typedef volatile struct {
    unsigned char bytesInPage;          // Number of bytes written to the current page so far
    unsigned long int currentAddress;
    enum {
        erIdle = 0,
        erWrite,
        erRead
    } state;
} eepromState_t;
extern volatile eepromState_t eepromState;

typedef enum {
    read = 1,
    write = 0
} eepromOpType;

void eepromReset(void);
unsigned char eepromStart(eepromOpType op, unsigned long int desiredAddress);
void eepromStop(void);
// These write commands may not cross page boundaries!
void eepromWriteArray(void* src, unsigned char size);
void eepromWriteByte(unsigned char byte);

// These read commands may not cross device boundaries! (TODO maybe?)
void eepromReadArray(void* dest, unsigned int size);
#define eepromReadByte() getI2C1(0)


#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

