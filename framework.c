#include "framework.h"


void delayms(uint ms){
    uint waitedms = 0;
    while (waitedms < ms){
        for(uint cycles = 0; cycles< 65; cycles++){
            NOP();
        }
        waitedms++;
    }
}



void fastBlink(){
   LED = 1;
   delayms(50);
   LED = 0;
   delayms(50);
   LED = 1;
   delayms(50);
   LED = 0;
   delayms(50);
   LED = 1;
   delayms(50);
   LED = 0;
}

void displayByteOnLED(uchar answer){
    uchar n = 8;
    while (n--){
        LED = 0;
        delayms(400);
        if (answer & 0b10000000) {
            LED = 1;
        }
        delayms(100);
        answer = answer << 1;
    }
    delayms(1000);
    LED = 0;
}
