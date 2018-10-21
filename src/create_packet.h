#ifndef __CREATE_PACKET_H_
#define __CREATE_PACKET_H_

void create_packet_data(struct pkt *, char *, int, int);

void create_packet_ack(struct pkt *, int);

void create_packet_nack(struct pkt *, int);

#endif