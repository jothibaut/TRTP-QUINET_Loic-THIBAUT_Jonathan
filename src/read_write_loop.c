#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zlib.h>
#include <string.h>
#include <time.h>

#include "packet_interface.h"
#include "create_packet.h"
#include "buffer.h"

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


//Réalisé avec l'aide de Thomas Reniers

void read_write_loop(int sfd){
    struct pollfd fds[2];
    int ret;
    int seqnum = 0;
    int sendPktCount = 0; //Nbr de pkt envoyés
    int sendBufCount = 0; // Nombre de places occupées dans le sendingBuf
    //int lastAck = 0;
    int i;

    char buf[BUFFER_SIZE];
    char buf_ack[BUFFER_SIZE];
    char buf_nack[BUFFER_SIZE];

    clock_t sendingTime[WINDOW_SIZE];
    
    size_t *bufLen = (size_t *) malloc(sizeof(size_t));
    *bufLen = BUFFER_SIZE;

    struct pkt* thePkt = NULL;
    thePkt = pkt_new();
    if(thePkt == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_data");
		free(bufLen);
		return;
	}
	struct pkt* thePkt_ack = NULL;
	thePkt_ack = pkt_new();
    if(thePkt_ack == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_ack");
		free(bufLen);
		pkt_del(thePkt);
		return;
	}
	struct pkt* thePkt_nack = NULL;
	thePkt_nack = pkt_new();
    if(thePkt_nack == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_nack");
		free(bufLen);
		pkt_del(thePkt);
		pkt_del(thePkt_ack);
		return;
	}

	//int sendingWindow[WINDOW_SIZE];
	//int receivingWindow[WINDOW_SIZE];

	struct pkt* (sendingBuf[WINDOW_SIZE]);
	struct pkt* (receivingBuf[WINDOW_SIZE]);
	if(create_storage_buffer(sendingBuf) == -1){
		fprintf(stderr, "%s\n", "Erreur lors de la création du sendingBuf");
		free(bufLen);
		pkt_del(thePkt);
		pkt_del(thePkt_ack);
		pkt_del(thePkt_nack);
		return;
	}
	if(create_storage_buffer(receivingBuf) == -1){
		fprintf(stderr, "%s\n", "Erreur lors de la création du receivingBuf");
		free(bufLen);
		pkt_del(thePkt);
		pkt_del(thePkt_ack);
		pkt_del(thePkt_nack);
		del_storage_buf(sendingBuf);
		return;
	}

    fds[0].fd = 0; //STDIN_FILENO
    fds[0].events = POLLIN;

    fds[1].fd = sfd;
    fds[1].events = POLLIN;
    
    for(;;){
        ret = poll(fds, 2, -1);
        if (ret == -1) {
            fprintf(stderr, "%s\n", "Echec lors de l execution de poll");
            return;
        }

        if (fds[0].revents & POLLIN){ //STDIN
        	//If sending buffer rempli
        		//si le RTO d'un ou pls pkt a expiré
        			//On renvoit ce(s) pkt
        		//on passe à la lecture sur sfd
        	if(sendBufCount == WINDOW_SIZE){
        		for(i=0;i<WINDOW_SIZE;i++){
        			clock_t t = clock();
        			float diff = ((float)(t - sendingTime[i]) / 1000000.0F ) * 1000;
        			if(diff > RTO){
        				pkt_status_code statEncode = pkt_encode(sendingBuf[i], buf, bufLen);
			            if(statEncode != 0){
			                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet data (renvoi)");
			                return;
			            }
			            if(write(sfd, buf, *bufLen) == -1){
			                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
			                return;
			            }
        			}
        		}
        	}else{
        		int r = read(0, buf, BUFFER_SIZE);
	            if(r == EOF){
	                fprintf(stderr, "%s\n", "stdin a atteint EOF");
	                return;
	            }
	            else if(r == -1){
	                fprintf(stderr, "%s\n", "lecture stdin : erreur");
	                return;
	            }

	            seqnum = sendPktCount % MAX_SEQNUM;
	            create_packet_data(thePkt, buf, seqnum);
	            if(thePkt == NULL){
	                fprintf(stderr, "%s\n", "Echec lors de la creation du paquet");
	                return;
	            }
	            memcpy(sendingBuf[sendPktCount%WINDOW_SIZE], thePkt, sizeof(struct pkt));
	            sendingTime[sendPktCount%WINDOW_SIZE] = clock();

	            pkt_status_code statEncode = pkt_encode(thePkt, buf, bufLen);
	            if(statEncode != 0){
	                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet data");
	                return;
	            }

	            if(write(sfd, buf, *bufLen) == -1){
	                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
	                return;
	            }
	            sendPktCount++;
        	}
        }




        
        if (fds[1].revents & POLLIN){ //SFD
	        int r = read(sfd, buf, BUFFER_SIZE);
	        if(r == -1){
	            fprintf(stderr, "%s\n", "Echec lors de la lecture du socket");
	            return;
	        }
	        else if(r == EOF){
	            fprintf(stderr, "%s\n", "le socket a atteint EOF");
	            return;
	        }
	        fprintf(stderr, "%s\n", "on a lu le socket");


	        pkt_status_code statDecode = pkt_decode(buf, r, thePkt);
	        if(statDecode != 0){ //Erreur qlq part
	        	if(statDecode == E_CRC){ //Le CRC n'a pas fonctionné
	        		//discard le paquet --> RTO le renverra
	        		//discard l'ack si on est dans le sender
	        	}
	        }else if(pkt_get_tr(thePkt) == 1){ //Paquet tronqué
	        	if(pkt_get_type(thePkt) == PTYPE_ACK || pkt_get_type(thePkt) == PTYPE_NACK){ //Si TR = 1 et type autre que data, on ignore
	        		//on ignore le paquet
	        	}else{
	        		create_packet_nack(thePkt_nack, seqnum);

		        	pkt_status_code statEncode = pkt_encode(thePkt_nack, buf_nack, bufLen);
		            if(statEncode != 0){
		                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet ack");
		                return;
		            }

		            if(write(sfd, buf_ack, *bufLen) == -1){
		                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
		                return;
		            }
	        	}
	        }else{ //Cas classique
	        	if(pkt_get_type(thePkt) == PTYPE_DATA){
	        		//Si seqnum pas dans la rcv win
	        			//J'ignore le paquet
	        		//Si seqnum premier elem de la rcv win
		        		//J'envois un ack du meme seqnum que thePkt
		        		//Je slide la receive window
	        		//Si seqnum dans la rcv win mais pas 1er elem
	        			//Je stock le pkt
		        	create_packet_ack(thePkt_ack, seqnum); //Attention pas le bon seqnum

		        	pkt_status_code statEncode = pkt_encode(thePkt_ack, buf_ack, bufLen);
		            if(statEncode != 0){
		                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet ack");
		                return;
		            }

		            if(write(sfd, buf_ack, *bufLen) == -1){
		                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
		                return;
		            }

			        if(fwrite(buf, sizeof(char), r, stdout) == 0){
	                    fprintf(stderr, "%s\n", "Echec lors de l'écriture sur stdout");
	                    return;
	                }
			        fflush(stdout);
			    }else if(pkt_get_type(thePkt) == PTYPE_ACK){
			    	//Si ack présent dans la window, je discard les n elem avant cet ack(celui-ci compris)
			    		//Je slide la sending window de n et discard les n premiers elem de sending buf

			    	//Si ack ne correspond pas a un num de la window, on renvoit tout le sending buf
			    	continue;
			    }else if(pkt_get_type(thePkt) == PTYPE_NACK){
			    	//On renvoit le paquet present dans le sending buf correspondant au seqnum
			    }
	        }  
        }
    }

    free(bufLen);
    pkt_del(thePkt);
    pkt_del(thePkt_ack);
    pkt_del(thePkt_nack);
    del_storage_buf(sendingBuf);
    del_storage_buf(receivingBuf);
}