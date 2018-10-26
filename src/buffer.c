#include "buffer.h"
#include "packet_interface.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct __attribute__((__packed__)) pkt {
    uint8_t  window:5;
    uint8_t  tr:1;
    ptypes_t type:2;
    uint8_t  seqnum;
    uint16_t length;
    uint32_t timestamp;
    uint32_t crc1;
    uint32_t crc2;
    char* payload;
};

int create_storage_buffer(struct pkt* storageBuf[]){
	int i;
	for(i=0; i<WINDOW_SIZE ; i++){
		storageBuf[i] = (struct pkt*) malloc(sizeof(pkt_t));
		if(storageBuf[i] == NULL){
			fprintf(stderr, "%s\n", "Erreur : malloc sendingBuf");
			return -1;
		}
	}
	if(storageBuf == NULL){
		fprintf(stderr, "%s\n", "Erreur : malloc sendingBuf");
		return -1;
	}

	return 0;
}

void del_storage_buf(struct pkt* storageBuf[]){
	int i;
	for(i=0; i<WINDOW_SIZE; i++){
		if(storageBuf[i] != NULL){
			free(storageBuf[i]);
		}
	}
}

int find_pkt(int seqnum, int window[]){
	int i;
	for(i=0;i<WINDOW_SIZE;i++){
		if(window[i] == seqnum){
			return i;
		}
	}
	return -1;
}

void window_slide(int window[]){
	int i;
	for(i=0;i<WINDOW_SIZE-1;i++){
		window[i] = window[i+1];
	}
	window[WINDOW_SIZE-1] = (window[WINDOW_SIZE-1] + 1) % (MAX_SEQNUM+1);
}

void update_sendingTime(clock_t sendingTime[]){
	int i;
	for(i=0;i<WINDOW_SIZE-1;i++){
		sendingTime[i] = sendingTime[i+1];
	}
	sendingTime[WINDOW_SIZE-1] = -1;
}

void update_binaryReceivingBuf(int buf[]){
	int i;
	for(i=0;i<WINDOW_SIZE-1;i++){
		buf[i] = buf[i+1];
	}
	buf[WINDOW_SIZE-1] = 0;
}