#ifndef RFM12_H
#define	RFM12_H

#include "framework.h"
#include "spi.h"

struct Rfm12Status{
    bool isFifoFull;
};

void rfm12_init_receiver();

struct Rfm12Status rfm12_read_status();

//bool rfm12_is_fifo_full (uint status);

uchar rfm12_read_data();

void rfm12_wait_for_new_stream();



#endif	/* RFM12_H */