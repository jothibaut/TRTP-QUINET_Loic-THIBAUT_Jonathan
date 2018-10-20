#ifndef __BUFFER_H_
#define __BUFFER_H_

#include "packet_interface.h"

#define BUFFER_SIZE 1024

#define WINDOW_SIZE 3 //Max : (2^n)/2 = 16 --> cfr p.25 sylla

#define MAX_SEQNUM 4 //!!!!!!!! A modifier : 255 !!!!!!!!!!!!

#define RTO 71 //[ms]

int create_storage_buffer(struct pkt **);

void del_storage_buf(struct pkt **);

#endif