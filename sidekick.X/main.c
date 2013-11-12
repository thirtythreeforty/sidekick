/* 
 * File:   main.c
 * Author: George Hilliard
 *
 * Created on November 10, 2013, 12:53 AM
 */

#include <stdio.h>
#include "pic24_all.h"

#include "tilink.h"
volatile extern TIfifo_tag TIfifo;

int main() {
    configBasic(HELLO_MSG);

    configTIlink();

    while(1) {
        doHeartbeat();
    }
}

