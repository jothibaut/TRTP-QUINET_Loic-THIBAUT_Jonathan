#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zlib.h>

#include "packet_interface.h"
#include "create_packet.h"

//Réalisé avec l'aide de Thomas Reniers

void read_write_loop(int sfd){
    struct pollfd fds[2];
    int ret;
    char buf[1024];
    char buf_ack[1024];
    char buf_nack[1024];
    int seqnum = 100;
    size_t *bufLen = (size_t *) malloc(sizeof(size_t));
    struct pkt* thePkt = NULL;
    thePkt = pkt_new();
    if(thePkt == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_data");
	}

	struct pkt* thePkt_ack = NULL;
	thePkt_ack = pkt_new();
    if(thePkt_ack == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_ack");
	}

	struct pkt* thePkt_nack = NULL;
	thePkt_nack = pkt_new();
    if(thePkt_nack == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_nack");
	}

    /* watch stdin for input */
    fds[0].fd = 0; //STDIN_FILENO
    fds[0].events = POLLIN;

    /* watch stdout for ability to write */
    fds[1].fd = sfd;
    fds[1].events = POLLIN;
    
    for(;;){
        ret = poll(fds, 2, -1);
        if (ret == -1) {
            fprintf(stderr, "%s\n", "Echec lors de l execution de poll");
            return;
        }
        if (fds[0].revents & POLLIN){ //STDIN
            int r = read(0, buf, 1024);
            if(r == EOF){
                fprintf(stderr, "%s\n", "stdin a atteint EOF");
                return;
            }
            else if(r == -1){
                fprintf(stderr, "%s\n", "lecture stdin : erreur");
                return;
            }

            create_packet_data(thePkt, buf, seqnum);
            if(thePkt == NULL){
                fprintf(stderr, "%s\n", "Echec lors de la creation du paquet");
                return;
            }
            seqnum++; //Attention provisoire !!! Nomalement connerie de modulo
            fprintf(stderr, "%s\n", "on a créé le paquet");

            uint8_t tr = pkt_get_seqnum(thePkt);
            fprintf(stderr, "%d\n", tr);

            *bufLen = 1024;
            fprintf(stderr, "%s\n", "on a rempli bufLen");
            pkt_status_code statEncode = pkt_encode(thePkt, buf, bufLen);
            if(statEncode != 0){
                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet data");
                return;
            }
            fprintf(stderr, "%s\n", "On a encodé le paquet");

            if(write(sfd, buf, *bufLen) == -1){
                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
                return;
            }
        }




        
        if (fds[1].revents & POLLIN){ //SFD
        	*bufLen = 1024;
	        int r = read(sfd, buf, 1024);
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
	        		//discarde le paquet --> RTO le renverra
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
			    	//On slide la fenêtre et on continue à envoyer des données
			    	continue;
			    }else if(pkt_get_type(thePkt) == PTYPE_NACK){
			    	//On renvoit le paquet
			    }
	        }  
        }
    }

    pkt_del(thePkt);
    pkt_del(thePkt_ack);
    pkt_del(thePkt_nack);
    free(bufLen);

}