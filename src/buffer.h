#ifndef __BUFFER_H_
#define __BUFFER_H_

#include "packet_interface.h"
#include <time.h>

#define BUFFER_SIZE 528

#define MAX_SEQNUM 255 //!!!!!!!! A modifier : 255 !!!!!!!!!!!!

#define RTO 200 //[ms]

int create_storage_buffer(struct pkt **);

void del_storage_buf(struct pkt **);

int find_pkt(int, int*);

void window_slide(int *);

//void update_sendingTime(clock_t[]);

void update_binaryReceivingBuf(int *);

void update_receivingBuf(struct pkt **);

#endif
