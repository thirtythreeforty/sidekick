/* 
 * File:   main.c
 * Author: George Hilliard
 *
 * Created on November 10, 2013, 12:53 AM
 */

#include "pic24_all.h"

#include "tilink.h"
#include "tipacket.h"

int main() {
    configBasic(HELLO_MSG);

    configTIlink();

    while(1)
        if(!getTIPacket())
            sendTIPacketReply();
}
