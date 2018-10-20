#include "packet_interface.h"
#include <stdlib.h>
#include <stdio.h>

void create_packet_data(struct pkt *thePkt, char *payload, int seqnum){
	pkt_status_code statTy = pkt_set_type(thePkt, PTYPE_DATA);
	if(statTy != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_type");
		thePkt = NULL;
		return;
	}

	pkt_status_code statTr = pkt_set_tr(thePkt, 0);
	if(statTr != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_tr");
		thePkt = NULL;
		return;
	}

	pkt_status_code statWin = pkt_set_window(thePkt, 1); //Fenêtre de 1 pour le moment --> A modifier pasr la suite
	if(statWin != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_window");
		thePkt = NULL;
		return;
	}

	pkt_status_code statSeq = pkt_set_seqnum(thePkt, seqnum);
	if(statSeq != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_seqnum");
		thePkt = NULL;
		return;
	}

	pkt_status_code statLength = pkt_set_length(thePkt, sizeof(payload));
	if(statLength != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_length");
		thePkt = NULL;
		return;
	}

	pkt_status_code statTimestamp = pkt_set_timestamp(thePkt,69); //Modifier timestamp
	if(statTimestamp != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_timestamp");
		thePkt = NULL;
		return;
	}

	pkt_status_code statCrc1 = pkt_set_crc1(thePkt, 0); //OSEF en vrai;
	if(statCrc1 != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_crc1");
		thePkt = NULL;
		return;
	}

	pkt_status_code statPayload = pkt_set_payload(thePkt, payload, sizeof(payload));
	if(statPayload != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_payload");
		thePkt = NULL;
		return;
	}

	pkt_status_code statCrc2 = pkt_set_crc2(thePkt, 0);
	if(statCrc2 != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_crc2");
		thePkt = NULL;
		return;
	}
}

void create_packet_ack(struct pkt *thePkt, int seqnum){
	pkt_status_code statTy = pkt_set_type(thePkt, PTYPE_ACK);
	if(statTy != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_type");
		thePkt = NULL;
		return;
	}

	pkt_status_code statTr = pkt_set_tr(thePkt, 0);
	if(statTr != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_tr");
		thePkt = NULL;
		return;
	}

	pkt_status_code statWin = pkt_set_window(thePkt, 1); //Fenêtre de 1 pour le moment --> A modifier pasr la suite
	if(statWin != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_window");
		thePkt = NULL;
		return;
	}

	pkt_status_code statSeq = pkt_set_seqnum(thePkt, seqnum);
	if(statSeq != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_seqnum");
		thePkt = NULL;
		return;
	}

	pkt_status_code statLength = pkt_set_length(thePkt, 0);
	if(statLength != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_length");
		thePkt = NULL;
		return;
	}

	pkt_status_code statTimestamp = pkt_set_timestamp(thePkt,69); //Modifier timestamp
	if(statTimestamp != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_timestamp");
		thePkt = NULL;
		return;
	}

	pkt_status_code statCrc1 = pkt_set_crc1(thePkt, 0); //OSEF en vrai;
	if(statCrc1 != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_crc1");
		thePkt = NULL;
		return;
	}

	pkt_status_code statPayload = pkt_set_payload(thePkt, NULL, 0);
	if(statPayload != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_payload");
		thePkt = NULL;
		return;
	}

	pkt_status_code statCrc2 = pkt_set_crc2(thePkt, 0);
	if(statCrc2 != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_crc2");
		thePkt = NULL;
		return;
	}

}

void create_packet_nack(struct pkt *thePkt, int seqnum){
	pkt_status_code statTy = pkt_set_type(thePkt, PTYPE_NACK);
	if(statTy != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_type");
		thePkt = NULL;
		return;
	}

	pkt_status_code statTr = pkt_set_tr(thePkt, 0);
	if(statTr != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_tr");
		thePkt = NULL;
		return;
	}

	pkt_status_code statWin = pkt_set_window(thePkt, 1); //Fenêtre de 1 pour le moment --> A modifier pasr la suite
	if(statWin != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_window");
		thePkt = NULL;
		return;
	}

	pkt_status_code statSeq = pkt_set_seqnum(thePkt, seqnum);
	if(statSeq != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_seqnum");
		thePkt = NULL;
		return;
	}

	pkt_status_code statLength = pkt_set_length(thePkt, 0);
	if(statLength != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_length");
		thePkt = NULL;
		return;
	}

	pkt_status_code statTimestamp = pkt_set_timestamp(thePkt,69); //Modifier timestamp
	if(statTimestamp != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_timestamp");
		thePkt = NULL;
		return;
	}

	pkt_status_code statCrc1 = pkt_set_crc1(thePkt, 0); //OSEF en vrai;
	if(statCrc1 != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_crc1");
		thePkt = NULL;
		return;
	}

	pkt_status_code statPayload = pkt_set_payload(thePkt, NULL, 0);
	if(statPayload != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_payload");
		thePkt = NULL;
		return;
	}

	pkt_status_code statCrc2 = pkt_set_crc2(thePkt, 0);
	if(statCrc2 != 0){
		fprintf(stderr, "%s\n", "Erreur : pkt_set_crc2");
		thePkt = NULL;
		return;
	}

}