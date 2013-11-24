/*
 * File:   variable.c
 * Author: George Hilliard
 *
 * Created on November 12, 2013, 12:55 PM
 */

#include "pic24_all.h"
#include "common.h"
#include "tipacket.h"
#include "eeprom.h"
#include "variable.h"

typedef struct {
    unsigned char empty;
    unsigned char calcType;
    unsigned int numVariables;
    unsigned long int offsetToFree;
} eepromHeader;

typedef struct {
    unsigned long int dataSize;
    unsigned char typeID;
    unsigned char varNameSize;
    unsigned char varName[20];
} TIvarHeader;

unsigned char variableVerifyAndInit(unsigned char calcType)
{
    eepromHeader erHeader;
    unsigned char i;
    TIvarHeader varHeader;

    debug("VerifyAndInit called.\n");

    // Read the variable header, make sure it matches what is already stored
    while(eepromStart(read, 0x000000))
        ;//asm(" pwrsav #1");
    eepromReadArray(&erHeader, sizeof(erHeader));
    eepromStop();
    debug("Reading of header complete.\n");
    for(i = 0; i < 6; ++i)                      // Read constant-sized part
        ((unsigned char*)(&varHeader))[i] = packetfifo_PopByte();
    for(i = 0; i < varHeader.varNameSize; ++i)  // Read name, ugh
        varHeader.varName[i] = packetfifo_PopByte();
    // The docs mention something about an extra byte for FLASH transfers.
    // Ignore it I guess.
    packetfifo_PopByte();

    if(calcType != erHeader.calcType ||
       (varHeader.dataSize + sizeof(varHeader)) > (MAX_EEPROM_SIZE - erHeader.offsetToFree))
        // In either of these cases, we'll say we don't have enough memory.
        // In the second case, that's actually true.
        return 1;

    debug("erHeader.offsetToFree is %i.\n", erHeader.offsetToFree);
    // Good, now set up the EEPROM write process.
    while(eepromStart(write, erHeader.offsetToFree))
        ;//asm("pwrsav #1");
    eepromWriteArray(&varHeader, sizeof(varHeader));
    // Don't stop because we don't have a whole page.
    // Start the interrupt!
    _T2IE = 1;
    debug("Done.\n");
    return 0;
}

volatile unsigned char variable_writeNow;

void variableFlush(void)
{
    // Force the interrupt to write the remaining data in the buffer.
    // Bit of a hack.
    debug("Flushing variable...");
    variable_writeNow = 1;
    while(packetfifo_Size())
        ; //asm("pwrsav #1");
    variable_writeNow = 0;
    // Disable the async-write interrupt.
    _T2IE = 0;
    debug("Flushed.\n");
}

void  _ISRFAST _T2Interrupt(void)
{
    // Write if we can fill up a page, or if we've been instructed to write no matter what.
    if(((packetfifo_Size() + eepromState.bytesInPage) >= EEPROM_PAGE_SIZE || variable_writeNow) &&
       eepromStart(write, eepromState.currentAddress) != 1) {
        while(eepromState.bytesInPage < EEPROM_PAGE_SIZE && packetfifo_Size())
            eepromWriteByte(packetfifo_PopByte());
        eepromStop();
    }
    _T2IF = 0;
}

void configVariable(void)
{
    eepromReset();

    variable_writeNow = 0;

    T2CON = T2_OFF | T2_IDLE_CON | T2_GATE_OFF
            | T2_32BIT_MODE_OFF
            | T2_SOURCE_INT
            | T2_PS_1_256;
    PR2 = usToU16Ticks(250, getTimerPrescale(T2CONbits)) - 1;
    TMR2  = 0;
    _T2IF = 0;
    _T2IP = 1;
    _T2IE = 0; // Let variableVerifyAndInit() enable the interrupt.
    T2CONbits.TON = 1;
}

void variableCommit(void)
{
    eepromHeader header;
    variableFlush();
    debug("Committing variable...\n");

    // Commit the transaction to the first page
    
    // Update the offset from the last eeprom address.
    unsigned long int newOffset = eepromState.currentAddress;
    unsigned int remainder = newOffset % EEPROM_PAGE_SIZE;
    if (remainder != 0)
        newOffset += EEPROM_PAGE_SIZE - remainder;

    while(eepromStart(read, 0x000000))
        ;//asm("pwrsav #1");
    eepromReadArray(&header, sizeof(header));
    eepromStop();

    ++header.numVariables;
    header.empty = 0;
    header.offsetToFree = newOffset;

    while(eepromStart(write, 0x000000))
        ;//asm("pwrsav #1");
    eepromWriteArray(&header, sizeof(header));
    eepromStop();
    debug("Committed.\n");
}
void variableClear(void)
{
    // Delete all data; just erase the header!
    eepromHeader header = {.empty = 1, .calcType = 0x98,
                           .numVariables = 0, .offsetToFree = EEPROM_PAGE_SIZE};
    debug("Clearing variables...");
    while(eepromStart(write, 0x000000))
        ;//asm("pwrsav #1");
    eepromWriteArray(&header, sizeof(header));
    eepromStop();
    debug("Cleared.\n");
}