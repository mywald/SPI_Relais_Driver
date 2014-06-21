/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <xc.h>            /* XC8 General Include File */
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */


/******************************************************************************/
/* CHIP Setup
/******************************************************************************/

#define _XTAL_FREQ 4000000
#pragma config FOSC = EXTRCCLK  // Oscillator Selection bits (RC oscillator: CLKOUT function on RA4/OSC2/CLKOUT pin, RC on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)


/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

typedef unsigned char uchar;
typedef unsigned int uint;

#define nSS RC2
#define LED RC1
#define ssDisplay RC0
#define RFMB_IRQ RA2
#define SWITCH RB7

uchar characters[16] = { 0b11001111, 0b00001100, 0b10101011, 0b10101110, 0b01101100, 0b11100110,
                         0b11100111, 0b11001100, 0b11101111, 0b11101110, 0b11101101, 0b01100111,
                         0b00100011, 0b00101111, 0b11100011, 0b11100001 };

uchar ringbuffer[6] = { 0, 0, 0, 0, 0, 0 };
uchar ringbufferpos = 0;
uchar maxbytes = 0;

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

void delayms(uint ms){
    uint waitedms = 0;
    while (waitedms < ms){
        for(uint cycles = 0; cycles< 65; cycles++){
            NOP();
        }
        waitedms++;
    }
}


void indicateByteSent(){
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

void displayChar(uchar data){
   uchar hnib = ((data & 0xF0) >> 4);
   uchar lnib = data & 0x0F;

   ssDisplay = 1;
   SSPBUF = characters[lnib];
   while (!SSPSTATbits.BF);
   SSPBUF = characters[hnib];
   while (!SSPSTATbits.BF);
   ssDisplay = 0;
}


uint spi_w(uint data){
    uchar ret1;
    uchar ret2;
    uchar byte1 = ((data & 0xFF00) >> 8);
    uchar byte2 = data & 0x00FF;

    nSS=0;                    //activate slave
    delayms(1);

    SSPBUF=byte1;             //send byte1
    while (!SSPSTATbits.BF);  //wait for transmission complete
    ret1=SSPBUF;              //read answer from buffer

    SSPBUF=byte2;             //send byte2
    while (!SSPSTATbits.BF);  //wait for transmission complete
    ret2=SSPBUF;

    delayms(1);
    nSS=1;                    //deactivate slave

    return (ret1 << 8) | ret2;
}

/*
void display(uchar answer){
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
*/

void interrupt ISR() {
    if(RABIF ) {              //Check if it is PORTA/B-Change Interrupt and it is the RFM12 IRQ
        uint status = spi_w(0x0000);  //Read Status register

        if (status & 0x8000) { //If FIFO full
            uint data1 = spi_w(0xB000) & 0x00FF; //Read data

            if (ringbufferpos == 0){
                maxbytes=0;
            }

            maxbytes++;

            ringbuffer[ringbufferpos] = data1;
            ringbufferpos++;
            if (ringbufferpos == 6){
                ringbufferpos = 0;
                spi_w(0xCA81);       //Disable ...
                spi_w(0xCA83);       //... and enable FIFO to start new synch detection
            }

       
            //TODO: compare if this chip is addressed
            //case which command has been sent:
            //1. toggle output of certain port
            //2. schedule status callback in some delay, to send information of all ports

        }
        RABIF=0;
    }/* else if (RABIF & SWITCH) { //Check if it is PORTA/B-Change Interrupt and it is the Switch-IRQ

        RABIF=0;
    }*/
}


void main(void)
{
    //Init IO
    ANSELH = 0X0;         //Disable analog Ports
    ANSEL = 0X0;          //Disable analog Ports

    TRISBbits.TRISB6 = 0; //RB6=Clock as Output
    TRISBbits.TRISB4 = 1; //RB4=Serial Data In as Input
    TRISCbits.TRISC7 = 0; //RC7=Serial Data Out as Output
    TRISCbits.TRISC2 = 0; //RC2=Slave Select for RFM12 as Output
    TRISCbits.TRISC1 = 0; //RC1=LED as Output
    TRISCbits.TRISC0 = 0; //RC0=Slave Select for Display as Output

    //Setup Timer2 for use as SPI-CLK
    //T2CON = 0b00001100;  //Enable Timer2 with Postscaler=1:2 and Prescaler=1 = ca. 2kHz

    //Init SPI
    SSPEN = 0;           //Turn off SPI to make it configurable
    SSPSTAT= 0b01000000; //SMP=0, CKE=1,
    //SSPCON = 0b00000011; //Enable SPI = false, CKP = 0, Configure as Master with CLK = Timer 2 output
    SSPCON = 0b00000010; //Enable SPI = false, CKP = 0, Configure as Master with CLK = FOSC/64
    SSPEN = 1;           //Turn on SPI
    SSPSTATbits.BF = 1;

    //Configure RA2 as Interrupt for RFM12B
    TRISAbits.TRISA2 = 1; //RA2=IRQ-from RFM12B
    IOCA2 = 1;            //Make RA2 to accept Interrupts

    //Configure RB7 as Interrupt for Taster
    //TRISBbits.TRISB7 = 1; //RB7=IRQ-from switch
    //IOCB7 = 1;            //Make RB7 to accept Interrupts

    //INTEDG = 1;           //Interrupt should occur on falling edge
    RABIE = 1;            //Enable Port-A/B-Change Interrupts
    INTEDG = 0;           //Interrupt should occur on rising edge
    GIE=1;                //Enable Interrupts globally

    //SSPIF = 1;           //Clear SPI Interface Flag
    nSS = 1;
    ssDisplay = 0;
    LED = 0;

    uchar answer;

    spi_w(0x80D8);       //Enable Register, 433MHz, 12.5pF
    spi_w(0x82D8);       //enable receive, !PA
    spi_w(0xA640);       //Frequency
    spi_w(0xC6A0);       //Bitrate
    spi_w(0x94C0);       //VDI Fast, 67kHz kHz, 0dbm, -103dBm
    spi_w(0xC2AC);       //Data Filter & Clock Recovery
    spi_w(0xCC76);       //Taktgenerator
    spi_w(0xCA83);       //FIFO8, SYNC
    spi_w(0xCED4);       //Synch Pattern
    spi_w(0xC487);       //Auto Frequency Control
//    spi_w(0xC623);       //Bitrate
    //spi_w(0xC647);
    //spi_w(0xC6F0);
    //spi_w(0x94A0);       //VDI Fast, 134 kHz, 0dbm, -103dBm
    //spi_w(0xCC77);
    //spi_w(0x9850);       //!mp, 9810=30kHz, MAX Out
    spi_w(0x9820);       //!mp, 45kHz, MAX Out


    spi_w(0xE000);       //Wake-Up Timer: Not use
    //spi_w(0xC8C9);       //Low-Duty-Cycle: D=100, enabled
    spi_w(0xC800);       //Low-Duty-Cycle: disabled
    spi_w(0xC000);       //1.0 MHz, 2.2V


    //Read Status register and display the answer
    answer = spi_w(0x0000);
    displayChar(answer);


    while(1)
    {
//        SLEEP(); //Sleep and wait for Interrupt;

        /*
         * //Read Status register and display the answer
        answer = spi_w(0x0000);
        display(answer);
*/

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

//         ringbufferpos = 0;
         //spi_w(0xCA81);       //Disable ...
         //spi_w(0xCA83);       //... and enable FIFO to start new synch detection

    }

}

