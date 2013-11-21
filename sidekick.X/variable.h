/* 
 * File:   variable.h
 * Author: George Hilliard
 *
 * Created on November 15, 2013, 12:55 AM
 */

#ifndef VARIABLE_H
#define	VARIABLE_H

#ifdef	__cplusplus
extern "C" {
#endif

void variableClear(void);
void variableCommit(void);
void variableFlush(void);
unsigned char variableVerifyAndInit(unsigned char calcType);
void configVariable(void);

#ifdef	__cplusplus
}
#endif

#endif	/* VARIABLE_H */

