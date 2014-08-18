#include "picsetup.h"
#include "framework.h"
#include "sevensegment.h"
#include "rfm12.h"

#define MODE_RECEIVE 0x01
#define MODE_REGISTRATION 0x02
#define BUFFERSIZE 16

#define COMMAND_ON 0x72
#define COMMAND_OFF 0x75
#define COMMAND_REQUEST_STATUS 0x51


uchar receivebuffer[BUFFERSIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uchar receivebufferpos = 0;
uchar workingmode = MODE_RECEIVE;
uchar channelflags[5] = {0, 0, 0, 0, 0};

void handle_incoming_command() {
    //TODO: compare if this chip is addressed
    //case which command has been sent:
    //1. toggle output of certain port
    //2. schedule status callback in some delay, to send information of all ports
    uchar device = receivebuffer[0];
    uchar channel = receivebuffer[1];
    uchar command = receivebuffer[2];

    if (command == COMMAND_REQUEST_STATUS){
        uchar param[3] = {0, 0, 0};
        param[0] = device;
        param[1] = channel;

        if (channelflags[channel]) {
            param[2] = COMMAND_ON;
        } else {
            param[2] = COMMAND_OFF;
        }

        rfm12_init_sender();
        rfm12_send_data(param);
    } else if (command == COMMAND_ON) {
        channelflags[channel] = true;
    } else if (command == COMMAND_OFF) {
        channelflags[channel] = false;
    }

    rfm12_init_receiver();
    rfm12_wait_for_new_stream();
}

void handle_byte_received() {
    uchar data = rfm12_read_data();

    receivebuffer[receivebufferpos] = data;
    receivebufferpos++;
    if (data == COMMAND_END_OF_STREAM || receivebufferpos > BUFFERSIZE) {
        receivebufferpos = 0;
        handle_incoming_command();
    }
}

void interrupt ISR() {
   if (RABIF) {      //Check if it is PORTA/B-Change Interrupt and it is the RFM12 IRQ
        struct Rfm12Status status = rfm12_read_status();
        if (status.isFifoFull) {
            handle_byte_received();
        }
        RABIF=0;
   } else if (SWITCH) { //Check if it is PORTA/B-Change Interrupt and it is the Switch-IRQ
        fastBlink();
   }

}


void main(void) {

    configureports();

    rfm12_init_common();
    rfm12_init_receiver();
    delayms(50);

    while(1) {
         for (uchar i = 0; i < 5; i++) {
             if (channelflags[i]){
                displayChar(i | 0x10);
             } else {
                displayChar(i);
             }
             delayms(700);
         }
    }

}

