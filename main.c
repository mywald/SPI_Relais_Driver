/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/
#include "picsetup.h"
#include "framework.h"
#include "spi.h"
#include "sevensegment.h"

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


uchar ringbuffer[6] = { 0, 0, 0, 0, 0, 0 };
uchar ringbufferpos = 0;
uchar maxbytes = 0;

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


void interrupt ISR() {
    if(RABIF ) {              //Check if it is PORTA/B-Change Interrupt and it is the RFM12 IRQ
        uint status = spi_write_2bytes( 0x0000);  //Read Status register

        if (status & 0x8000) { //If FIFO full
            uint data1 = spi_write_2bytes( 0xB000) & 0x00FF; //Read data

            if (ringbufferpos == 0){
                maxbytes=0;
            }

            maxbytes++;

            ringbuffer[ringbufferpos] = data1;
            ringbufferpos++;
            if (ringbufferpos == 6){
                ringbufferpos = 0;
                spi_write_2bytes(0xCA81);       //Disable ...
                spi_write_2bytes(0xCA83);       //... and enable FIFO to start new synch detection
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

    //Setup Timer2
    //T2CON = 0b00001100;  //Enable Timer2 with Postscaler=1:2 and Prescaler=1 = ca. 2kHz

    //Init SPI
    SSPEN = 0;           //Turn off SPI to make it configurable
    SSPSTAT= 0b01000000; //SMP=0, CKE=1,
    SSPCON = 0b00000010; //Enable SPI = false, CKP = 0, Configure as Master with CLK = FOSC/64
    SSPEN = 1;           //Turn on SPI
    SSPSTATbits.BF = 1;

    //Configure RA2 as Interrupt for RFM12B
    TRISAbits.TRISA2 = 1; //RA2=IRQ-from RFM12B
    IOCA2 = 1;            //Make RA2 to accept Interrupts

    //Configure RB7 as Interrupt for Switch
    //TRISBbits.TRISB7 = 1; //RB7=IRQ-from switch
    //IOCB7 = 1;            //Make RB7 to accept Interrupts

    RABIE = 1;            //Enable Port-A/B-Change Interrupts
    INTEDG = 0;           //Interrupt should occur on rising edge
    GIE=1;                //Enable Interrupts globally

    nSPI_SLAVE_SELECT = 1;
    DISPLAY_SLAVE_SELECT = 0;
    LED = 0;

    uchar answer;

    spi_write_2bytes(0x80D8);       //Enable Register, 433MHz, 12.5pF
    spi_write_2bytes(0x82D8);       //enable receive, !PA
    spi_write_2bytes(0xA640);       //Frequency
    spi_write_2bytes(0xC6A0);       //Bitrate
    spi_write_2bytes(0x94C0);       //VDI Fast, 67kHz kHz, 0dbm, -103dBm
    spi_write_2bytes(0xC2AC);       //Data Filter & Clock Recovery
    spi_write_2bytes(0xCC76);       //Taktgenerator
    spi_write_2bytes(0xCA83);       //FIFO8, SYNC
    spi_write_2bytes(0xCED4);       //Synch Pattern
    spi_write_2bytes(0xC487);       //Auto Frequency Control
    spi_write_2bytes(0x9820);       //!mp, 45kHz, MAX Out


    spi_write_2bytes(0xE000);       //Wake-Up Timer: Not use
    //spi_write_2bytes(0xC8C9);       //Low-Duty-Cycle: D=100, enabled
    spi_write_2bytes(0xC800);       //Low-Duty-Cycle: disabled
    spi_write_2bytes(0xC000);       //1.0 MHz, 2.2V


    //Read Status register and display the answer
    answer = spi_write_2bytes(0x0000);
    displayChar(answer);


    while(1)
    {
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

