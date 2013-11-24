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

#define DEBUG

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

