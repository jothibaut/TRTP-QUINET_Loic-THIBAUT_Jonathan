#include "packet_interface.h"
#include <stdlib.h>
#include <stdio.h>

void create_packet_data(struct pkt *thePkt, char *payload, int seqnum){
	thePkt = pkt_new();

	pkt_status_code statTy = pkt_set_type(thePkt, PTYPE_DATA);
	if(statTy != 0){
		thePkt = NULL;
	}

	pkt_status_code statTr = pkt_set_tr(thePkt, 0);
	if(statTr != 0){
		thePkt = NULL;
	}

	pkt_status_code statWin = pkt_set_window(thePkt, 1); //Fenêtre de 1 pour le moment --> A modifier pasr la suite
	if(statWin != 0){
		thePkt = NULL;
	}

	pkt_status_code statSeq = pkt_set_seqnum(thePkt, seqnum);
	if(statSeq != 0){
		thePkt = NULL;
	}

	pkt_status_code statLength = pkt_set_length(thePkt, sizeof(payload));
	if(statLength != 0){
		thePkt = NULL;
	}

	pkt_status_code statTimestamp = pkt_set_timestamp(thePkt,69); //Modifier timestamp
	if(statTimestamp != 0){
		thePkt = NULL;
	}

	pkt_status_code statCrc1 = pkt_set_crc1(thePkt, 0); //OSEF en vrai;
	if(statCrc1 != 0){
		thePkt = NULL;
	}

	pkt_status_code statPayload = pkt_set_payload(thePkt, payload, sizeof(payload));
	if(statPayload != 0){
		thePkt = NULL;
	}

	pkt_status_code statCrc2 = pkt_set_crc2(thePkt, 0);
	if(statCrc2 != 0){
		thePkt = NULL;
	}
}