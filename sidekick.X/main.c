/* 
 * File:   main.c
 * Author: George Hilliard
 *
 * Created on November 10, 2013, 12:53 AM
 */

#include "pic24_all.h"

#include "tilink.h"
#include "tipacket.h"
#include "variable.h"

int main() {
    configBasic(HELLO_MSG);

    configTIlink();
    configVariable();

    while(1)
        getTIPacket();
}
