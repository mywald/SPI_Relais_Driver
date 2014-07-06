#include "picsetup.h"
#include "framework.h"
#include "sevensegment.h"
#include "rfm12.h"


uchar ringbuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uchar ringbufferpos = 0;
uchar maxbytes = 0;


void handle_byte_received() {
    uchar data = rfm12_read_data();

    if (ringbufferpos == 0){
        maxbytes=0;
    }

    maxbytes++;

    ringbuffer[ringbufferpos] = data;
    ringbufferpos++;
    if (ringbufferpos == 8){
        ringbufferpos = 0;
        rfm12_wait_for_new_stream();
    }


    //TODO: compare if this chip is addressed
    //case which command has been sent:
    //1. toggle output of certain port
    //2. schedule status callback in some delay, to send information of all ports
}

void interrupt ISR() {
    if(RABIF) {              //Check if it is PORTA/B-Change Interrupt and it is the RFM12 IRQ
        struct Rfm12Status status = rfm12_read_status();
        if (status.isFifoFull) {
            handle_byte_received();
        }
        RABIF=0;
    }/* else if (RABIF & SWITCH) { //Check if it is PORTA/B-Change Interrupt and it is the Switch-IRQ

        RABIF=0;
    }*/
}


void main(void) {

    configureports();

    rfm12_init_receiver();
    delayms(1000);

    while(1) {
//        SLEEP(); //Sleep and wait for Interrupt;


        /* TIMER 2 demo output
         * if (TMR2IF){
            LED = !LED;
            TMR2IF = 0;
        }
         */

         LED = 1;
         displayChar(maxbytes);
         delayms(700);
         LED = 0;

         for (uchar i = 0; i < 6; i++) {
            displayChar(ringbuffer[i]);
            delayms(700);
         }

    }

}

