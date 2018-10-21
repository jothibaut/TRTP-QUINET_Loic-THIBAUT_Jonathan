#ifndef __BUFFER_H_
#define __BUFFER_H_

#include "packet_interface.h"
#include <time.h>

#define BUFFER_SIZE 1024

#define WINDOW_SIZE 2 //Max : (2^n)/2 = 16 --> cfr p.25 sylla

#define MAX_SEQNUM 3 //!!!!!!!! A modifier : 255 !!!!!!!!!!!!

#define RTO 71 //[ms]

int create_storage_buffer(struct pkt **);

void del_storage_buf(struct pkt **);

int find_pkt(int, int*);

void window_slide(int *);

void update_sendingTime(clock_t[]);

#endif