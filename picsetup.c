#include "picsetup.h"

void configureports(){
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
}
