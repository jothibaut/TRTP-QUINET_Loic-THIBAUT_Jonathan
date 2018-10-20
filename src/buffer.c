#include "buffer.h"
#include "packet_interface.h"

#include <stdlib.h>
#include <stdio.h>

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

int create_storage_buffer(struct pkt* sendingBuf[]){
	int i;
	for(i=0; i<WINDOW_SIZE ; i++){
		sendingBuf[i] = (struct pkt*) malloc(sizeof(pkt_t));
		if(sendingBuf[i] == NULL){
			fprintf(stderr, "%s\n", "Erreur : malloc sendingBuf");
			return -1;
		}
	}
	if(sendingBuf == NULL){
		fprintf(stderr, "%s\n", "Erreur : malloc sendingBuf");
		return -1;
	}

	return 0;
}

void del_storage_buf(struct pkt* receivingBuf[]){
	int i;
	for(i=0; i<WINDOW_SIZE; i++){
		free(receivingBuf[i]);
	}
}