#ifndef __CREATE_PACKET_H_
#define __CREATE_PACKET_H_

void create_packet_data(struct pkt *, char *, int, int);

void create_packet_deco(struct pkt *thePkt, int seqnum);

void create_packet_ack(struct pkt *, int, int);

void create_packet_nack(struct pkt *, int, int);

#endif
