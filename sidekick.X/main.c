/* 
 * File:   main.c
 * Author: George Hilliard
 *
 * Created on November 10, 2013, 12:53 AM
 */

#include "pic24_all.h"
#include "common.h"
#include "tilink.h"
#include "tipacket.h"
#include "variable.h"

int main() {
#ifdef DEBUG
    configBasic(HELLO_MSG);
#endif

    configTIlink();
    configVariable();

#ifdef DEBUG
    // for debugging, clear on every poweron
    variableClear();
#endif

    while(1)
        getTIPacket();
}
