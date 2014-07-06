#include "rfm12.h"

void rfm12_init_receiver() {
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

/*bool rfm12_is_fifo_full (uint _status){
    return _status & 0x8000;
} */

uchar rfm12_read_data(){
    return spi_write_2bytes( 0xB000) & 0x00FF;
}

void rfm12_wait_for_new_stream(){
    spi_write_2bytes(0xCA81);       //Disable ...
    spi_write_2bytes(0xCA83);       //... and enable FIFO to start new synch detection
}

