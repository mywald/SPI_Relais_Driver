#include "rfm12.h"

void rfm12_init_common() {
    spi_write_2bytes(0x0000);       //Read Status
    spi_write_2bytes(0x8208);       //Enable Register, 433MHz, 12.5pF
    spi_write_2bytes(0x80D8);       //enable receive, !PA
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
}


void rfm12_init_receiver() {
    spi_write_2bytes(0x0000);       //Read Status
    spi_write_2bytes(0x82D8);       //enable receive, !PA
    spi_write_2bytes(0x0000);       //Read Status
}

void rfm12_init_sender() {
    spi_write_2bytes(0x0000);       //Read Status
    spi_write_2bytes(0x8208);       //enable sender, !PA
    spi_write_2bytes(0x0000);       //Read Status
}

struct Rfm12Status rfm12_read_status() {
    struct Rfm12Status s;
    uint statreg = spi_write_2bytes(0x0000);
    if (statreg & 0x8000){
        s.isFifoFull = true;
    } else {
        s.isFifoFull = false;
    }
    return s;
}

uchar rfm12_read_data(){
    return spi_write_2bytes( 0xB000) & 0x00FF;
}

void rfm12_send_single_data_byte(uchar data){
    bool sendRegisterExpectsByte = false;
    while (!sendRegisterExpectsByte) {
        uint stat = spi_write_2bytes(0x0000); // Read Status register
        sendRegisterExpectsByte = (stat & 0b1000000000000000) > 0;
    }


    spi_write_2bytes(0xB800 | data);
}

void rfm12_send_data(uchar data[]){
    spi_write_2bytes(0x0000); //Read Status register
    spi_write_2bytes(0x8008); //"Disable FIFO Register");
    spi_write_2bytes(0x80D8); //"Enable FIFO Register");

    spi_write_2bytes(0x8238); //"enable transmitter, enable xtal, enable PLL synthesizer");

    delayms(50);

    rfm12_send_single_data_byte(0xAA); // "PREAMBLE senden");
    rfm12_send_single_data_byte(0xAA); // "PREAMBLE senden");
    rfm12_send_single_data_byte(0xAA); // "PREAMBLE senden");
    rfm12_send_single_data_byte(0x2D); // "HI Byte senden");
    rfm12_send_single_data_byte(0xD4); // "LOW Byte for Frame-Detection senden");

    for (uint i = 0; i < sizeof(data); i++) {
        rfm12_send_single_data_byte(data[i]);//, "Send data");
    }

    rfm12_send_single_data_byte(COMMAND_END_OF_STREAM);//, "EOS");
    rfm12_send_single_data_byte(COMMAND_END_OF_STREAM);//, "EOS");
    rfm12_send_single_data_byte(0xAA); // "PREAMBLE senden");
    rfm12_send_single_data_byte(0xAA); // "PREAMBLE senden");

    spi_write_2bytes(0x8208); // "turn off transmitter");
    spi_write_2bytes(0x0000); // "Read Status register");
}



void rfm12_wait_for_new_stream(){
    spi_write_2bytes(0xCA81);       //Disable ...
    spi_write_2bytes(0xCA83);       //... and enable FIFO to start new synch detection
}

