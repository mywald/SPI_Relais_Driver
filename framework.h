#ifndef FRAMEWORK_H
#define FRAMEWORK_H

/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <xc.h>            /* XC8 General Include File */
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include "picsetup.h"

/******************************************************************************/
/* Types                                                                      */
/******************************************************************************/

typedef unsigned char uchar;
typedef unsigned int uint;


/******************************************************************************/
/* Tools                                                                      */
/******************************************************************************/

void delayms(uint ms);

void fastBlink();

void displayByteOnLED(uchar answer);

#endif //FRAMEWORK_H