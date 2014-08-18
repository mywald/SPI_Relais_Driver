#ifndef RFM12_H
#define	RFM12_H

#define COMMAND_END_OF_STREAM 0xE5

#include "framework.h"
#include "spi.h"
#include "sevensegment.h"

struct Rfm12Status{
    bool isFifoFull;
};

void rfm12_init_common();
void rfm12_init_receiver();
void rfm12_init_sender();

struct Rfm12Status rfm12_read_status();

uchar rfm12_read_data();

void rfm12_wait_for_new_stream();

void rfm12_send_data(uchar data[]);


#endif	/* RFM12_H */