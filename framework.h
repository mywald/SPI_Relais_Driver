#ifndef FRAMEWORK_H
#define FRAMEWORK_H

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