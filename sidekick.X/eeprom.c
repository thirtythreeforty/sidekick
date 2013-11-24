/*
 * File:   eeprom.c
 * Author: George Hilliard
 *
 * Created on November 12, 2013, 12:55 PM
 */

#include "pic24_all.h"
#include "common.h"
#include "eeprom.h"

volatile eepromState_t eepromState;

typedef struct {
    unsigned char ctrl;
    unsigned char addresshigh; // big-endian
    unsigned char addresslow;
} eepromPhyAddr;

unsigned char eepromStart(eepromOpType op, unsigned long int desiredAddress)
{
    if(eepromState.state == erIdle) {
        eepromState.currentAddress = desiredAddress;
        startI2C1();
        unsigned char ctrlbyte = 0b10100000 | ((eepromState.currentAddress & 0x10000) >> 13) | (eepromState.currentAddress >> 17);
        if(putNoAckCheckI2C1(ctrlbyte)) {
            // No good, the device is still in write cycle.
            return 1;
        }
        putI2C1(eepromState.currentAddress >> 8);
        putI2C1(eepromState.currentAddress & 0xFF);
        if(op == read) {
            // Must restart the device in read mode
            rstartI2C1();
            putI2C1(ctrlbyte | 1);
        }
        else {
            eepromState.bytesInPage = eepromState.currentAddress % EEPROM_PAGE_SIZE;
            debug("Bytes in new page: 0x%02x (@ 0x%04x).\n",eepromState.bytesInPage, (unsigned int)eepromState.currentAddress);
        }
        eepromState.state = (op == write) ? erWrite : erRead;
        return 0;
    }
    else
        // We're busy.
        return 2;
}

void eepromStop(void)
{
    if(eepromState.state == erRead)
        // Must have dummy read with no ack
        getI2C1(1);
    stopI2C1();
    eepromState.state = erIdle;
    // The following is required for clients that check if we have free space.
    if(eepromState.bytesInPage == EEPROM_PAGE_SIZE)
        eepromState.bytesInPage = 0;
}

void eepromWriteByte(unsigned char byte)
{
    putI2C1(byte);
    ++eepromState.bytesInPage;
    ++eepromState.currentAddress;
}

void eepromReadArray(void* dest, unsigned int size)
{
    unsigned int i;
    eepromState.currentAddress += size;
    for(i = 0; i < size; ++i)
        *(unsigned char*)dest++ = getI2C1(0);
}

void eepromWriteArray(void* src, unsigned char size)
{
    unsigned char i;
    eepromState.bytesInPage += size;
    eepromState.currentAddress += size;
    for(i = 0; i < size; ++i)
        putI2C1(*(unsigned char*)src++);
}

void eepromReset(void)
{
    configI2C1(1000); // TODO is this ok with 3 chips?
    
    // As recommended in Microchip's docs: the
    // correct way to ensure the memory is in
    // a neutral state.
    startI2C1();
    putNoAckCheckI2C1(0xFF);
    rstartI2C1();
    stopI2C1();
}