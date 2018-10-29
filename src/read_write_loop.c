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

size_t *bufLen = NULL;
struct pkt* thePkt = NULL;
struct pkt* thePkt_ack = NULL;
struct pkt* thePkt_nack = NULL;
struct pkt* (sendingBuf[MAX_WINDOW_SIZE]);
struct pkt* (receivingBuf[MAX_WINDOW_SIZE]);

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

void free_all(){
	if(bufLen != NULL){
		free(bufLen);
	}

	if(thePkt != NULL){
		pkt_del(thePkt);
	}

	if(thePkt_ack != NULL){
		pkt_del(thePkt_ack);
	}

	if(thePkt_nack != NULL){
		pkt_del(thePkt_nack);
	}

	del_storage_buf(sendingBuf);
	del_storage_buf(receivingBuf);
}


//La fonction read_write_loop qui devait être implémentée sur Inginious a été réalisée avec l'aide de Thomas Reniers

void read_write_loop(int sfd, int readFd, FILE* writeFile){

	int timeOut = 500; //Temps d'attente de poll --> [ms]
    struct pollfd fds[2];
    int ret;
    int seqnum = 0;
    int sendPktCount = 0; //Nbr de pkt envoyés
    int sendBufCount = 0; // Nombre de places occupées dans le sendingBuf
    int lastAck = 0;
    int receivingAvaiableSlotCount = 1;
    int receivingWindowSize = MAX_WINDOW_SIZE;
    int receivingEmptySlot = receivingWindowSize;
    int sendingWindowSize = MAX_WINDOW_SIZE;
    int lastAckSeqnum = -1;
    int i;

    int binaryReceivingBuf[MAX_WINDOW_SIZE];
    for(i=0;i<MAX_WINDOW_SIZE;i++){
    	binaryReceivingBuf[i] = 0;
    }

    clock_t sendingTime[MAX_WINDOW_SIZE];
    
    bufLen = (size_t *) malloc(sizeof(size_t));
    if(bufLen == NULL){
    	free_all();
    }

    thePkt = pkt_new();
    if(thePkt == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_data");
		free_all();
		return;
	}
	
	thePkt_ack = pkt_new();
    if(thePkt_ack == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_ack");
		free_all();
		return;
	}
	
	thePkt_nack = pkt_new();
    if(thePkt_nack == NULL){
		fprintf(stderr, "%s\n", "Erreur : pkt_new_nack");
		free_all();
		return;
	}

	int sendingWindow[MAX_WINDOW_SIZE];
	int receivingWindow[MAX_WINDOW_SIZE];
	for(i=0;i<MAX_WINDOW_SIZE;i++){
		sendingWindow[i] = i;
		receivingWindow[i] = i;
	}

	if(create_storage_buffer(sendingBuf) == -1){
		fprintf(stderr, "%s\n", "Erreur lors de la création du sendingBuf");
		free_all();
		return;
	}
	if(create_storage_buffer(receivingBuf) == -1){
		fprintf(stderr, "%s\n", "Erreur lors de la création du receivingBuf");
		free_all();
		return;
	}

    fds[0].fd = readFd;
    fds[0].events = POLLIN;

    fds[1].fd = sfd;
    fds[1].events = POLLIN;
    
    for(;;){
        ret = poll(fds, 2, timeOut);
        if (ret == -1) {
            fprintf(stderr, "%s\n", "Echec lors de l execution de poll");
			free_all();
            return;
        }else if(ret == 0){
        	fprintf(stderr, "%s\n", "Timed out");
			free_all();
        	return;
        }

        if (fds[0].revents & POLLIN){ //STDIN ou FILE

        	//si le RTO d'un ou pls pkt a expiré
        		//On renvoit ce(s) pkt
    		for(i=0;i<MAX_WINDOW_SIZE;i++){
    			clock_t t = clock();
    			float diff = ((float)(t - sendingTime[i]) / 1000000.0F ) * 1000;
    			if(diff > RTO && sendingTime[i] != -1){
    				sendingTime[i] = t;
    				*bufLen = 528;
    				char buf_data[528];
    				pkt_status_code statEncode = pkt_encode(sendingBuf[i], buf_data, bufLen);
		            if(statEncode != 0){
		                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet data (renvoi)");
		                free_all();
		                return;
		            }
		            if(write(fds[1].fd, buf_data, *bufLen) == -1){
		                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
		                free_all();
		                return;
		            }
		            fprintf(stderr, "%s%d\n", "Renvoi - pkt data - ", pkt_get_seqnum(sendingBuf[i]));
    			}
    		}

    		//Si le sendingBuf est plein, on passe directement sur SFD
        	if(receivingAvaiableSlotCount != 0 && sendBufCount != sendingWindowSize){
        		char buf_payload[512];
        		int r = read(fds[0].fd, buf_payload, 512);
	            //On atteint End Of File
	            if(r == 0){ //Pour moi "&& readFd!=0" est inutile
	            	//Si le dernier acquittement reçu correspond bien au paquet suivant qu'on aurait envoyé
	            	if(lastAckSeqnum == sendPktCount % (MAX_SEQNUM+1)){
	            		fflush(stdout);
		                //crée un packet de ou la longueur vaut 0 (signifie une demande de déconnexion)
	            		seqnum = sendPktCount % (MAX_SEQNUM+1);
		                create_packet_deco(thePkt, seqnum);
		            	if(thePkt == NULL){
		                	fprintf(stderr, "%s\n", "Echec lors de la creation du paquet de déconnexion");
		                	free_all();
		                	return;
		                }

		                memcpy(sendingBuf[sendPktCount%MAX_WINDOW_SIZE], thePkt, sizeof(struct pkt));
			            sendBufCount++;
			            sendingTime[sendPktCount%MAX_WINDOW_SIZE] = clock();
		                
		                *bufLen = 528;
		            	char buf_data[528];
		                pkt_status_code statEncode = pkt_encode(thePkt, buf_data, bufLen);
		            	if(statEncode != 0){
		                	fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet deconnexion");
		                	free_all();
		                	return;
		            	}

		            	if(write(fds[1].fd, buf_data, *bufLen) == -1){
		                	fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket pour la deconnection");
		                	free_all();
		                	return;
		            	}
		            	fprintf(stderr, "%s%d\n", "Envoi - pkt déco - ", seqnum);
		                
		                sendPktCount++;
		                receivingAvaiableSlotCount--;
		                free_all();
		                return;
		                //fin de la demande de deconnexion et se deconnecte direct
	            	}
	            }
	            else if(r == -1){
	                fprintf(stderr, "%s\n", "lecture stdin : erreur");
	                free_all();
	                return;
	            }else{
	            	/*
		            int w = fwrite(buf_payload, r, sizeof(char), stdout);
					if(w == 0){
						fprintf(stderr, "%s\n", "Error : ecriture");
					}
					*/
		            seqnum = sendPktCount % (MAX_SEQNUM+1);
		            create_packet_data(thePkt, buf_payload, seqnum, r);
		            if(thePkt == NULL){
		                fprintf(stderr, "%s\n", "Echec lors de la creation du paquet");
		                free_all();
		                return;
		            }
		            memcpy(sendingBuf[sendPktCount%MAX_WINDOW_SIZE], thePkt, sizeof(struct pkt));
		            sendBufCount++;
		            sendingTime[sendPktCount%MAX_WINDOW_SIZE] = clock();

		            *bufLen = 528;
		            char buf_data[528];
		            pkt_status_code statEncode = pkt_encode(thePkt, buf_data, bufLen);
		            if(statEncode != 0){
		                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet data");
		                free_all();
		                return;
		            }

		            if(write(fds[1].fd, buf_data, *bufLen) == -1){
		                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
		                free_all();
		                return;
		            }
		            fprintf(stderr, "%s%d\n", "Envoi - pkt data - ", seqnum);
		            sendPktCount++;
		            receivingAvaiableSlotCount--;

	            }
	            
        	}
        }


        
        if (fds[1].revents & POLLIN){ //SFD
        	char buf_rcv[528];
	        int r = read(fds[1].fd, buf_rcv, 528);
	        if(r == -1){
	            fprintf(stderr, "%s\n", "Echec lors de la lecture du socket");
	            free_all();
	            return;
	        }
	        else if(r == 0){ //Ca n'arrivera normalement jamais
	            fprintf(stderr, "%s\n", "Le socket a atteint EOF");
	            free_all();
	            return;
	        }


	        pkt_status_code statDecode = pkt_decode(buf_rcv, r, thePkt);
	        if(statDecode != 0){ //Erreur qlq part
	        	if(statDecode == E_CRC){ //Le CRC n'a pas fonctionné
	        		//discard le paquet --> RTO le renverra
	        		//discard l'ack si on est dans le sender
	        		//NB : par discard, on entend ne pas stocker le pkt dans le buf
	        		fprintf(stderr, "%s\n","CRC du paquet invalide");
	        		continue;
	        	}else if(statDecode == E_NOMEM){
	        		fprintf(stderr, "%s\n", "Erreur de mémoire");
	        	}
	        }else if(pkt_get_tr(thePkt) == 1){ //Paquet tronqué
	        	if(pkt_get_type(thePkt) == PTYPE_ACK || pkt_get_type(thePkt) == PTYPE_NACK){ //Si TR = 1 et type autre que data, on ignore
	        		//on ignore le paquet
	        		fprintf(stderr, "%s%d\n", "Réception - pkt invalide - ", pkt_get_seqnum(thePkt));
	        		continue;
	        	}else{
	        		seqnum = pkt_get_seqnum(thePkt);
	        		fprintf(stderr, "%s%d\n", "Réception - pkt tronqué - ", seqnum);
	        		create_packet_nack(thePkt_nack, seqnum, receivingEmptySlot);
	        		if(thePkt_nack == NULL){
	        			fprintf(stderr, "%s\n", "Echec lors de la creation du paquet nack");
	        		}

	        		char buf_nack[528];
	        		*bufLen = 528;
		        	pkt_status_code statEncode = pkt_encode(thePkt_nack, buf_nack, bufLen);
		            if(statEncode != 0){
		                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet nack");
		                free_all();
		                return;
		            }

		            if(write(fds[1].fd, buf_nack, *bufLen) == -1){
		                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
		                free_all();
		                return;
		            }
	        	}
	        }else if(pkt_get_length(thePkt)==0 &&  pkt_get_type(thePkt) == PTYPE_DATA){
	        		//on a recu un packet symbolisant la deconnexion donc on se deconnecte aussi
	        		fprintf(stderr, "%s%d\n", "Réception - pkt déco - ", pkt_get_seqnum(thePkt));
	            	fflush(stdout);
					free_all();
	        		return;        
	        }else{ //Cas classique
	        	if(pkt_get_type(thePkt) == PTYPE_DATA){
	        		fprintf(stderr, "%s%d\n", "Réception - pkt data - ", pkt_get_seqnum(thePkt));

	        		int index = find_pkt(pkt_get_seqnum(thePkt), receivingWindow);
	        		if(index == -1){
	        			//envoit paquet avec lastAck
	        			fprintf(stderr, "%s\n", "Le paquet reçu ne figure pas dans le receivingBuf");
	        			/*
	        			int j;
	        			for(j=0;j<MAX_WINDOW_SIZE;j++){
	        				fprintf(stderr, "%d", receivingWindow[j]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/
	        			seqnum = (lastAck+1)%(MAX_SEQNUM+1);
	        			create_packet_ack(thePkt_ack, seqnum, receivingEmptySlot);

	        			char buf_ack[528];
	        			*bufLen = 528;
			        	pkt_status_code statEncode = pkt_encode(thePkt_ack, buf_ack, bufLen);
			            if(statEncode != 0){
			                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet ack");
			                free_all();
			                return;
			            }

			            if(write(fds[1].fd, buf_ack, *bufLen) == -1){
			                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
			                free_all();
			                return;
			            }
			            fprintf(stderr, "%s%d\n", "Envoi - pkt ack - ", seqnum);
	        		}else if(index == 0){

				        if(fwrite(pkt_get_payload(thePkt), sizeof(char), pkt_get_length(thePkt), writeFile) == 0){
		                    fprintf(stderr, "%s\n", "Echec lors de l'écriture sur stdout");
		                    free_all();
		                    return;
		                }
		                if(writeFile == stdout){
		                	fflush(stdout);
		                }
				        lastAck = pkt_get_seqnum(thePkt);
				        /*
				        int k;
	        			for(k=0;k<MAX_WINDOW_SIZE;k++){
	        				fprintf(stderr, "%d", receivingWindow[k]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/
				        window_slide(receivingWindow);
				        /*
				        for(k=0;k<MAX_WINDOW_SIZE;k++){
	        				fprintf(stderr, "%d", receivingWindow[k]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/

				        int j = 1;
				        while(binaryReceivingBuf[j] == 1 && MAX_WINDOW_SIZE > 1){ //Vider le buffer
				        	binaryReceivingBuf[j] = 0;
				        	if(fwrite(pkt_get_payload(receivingBuf[j]), sizeof(char), pkt_get_length(receivingBuf[j]), writeFile) == 0){
			                    fprintf(stderr, "%s\n", "Echec lors de l'écriture sur stdout");
			                    free_all();
			                    return;
			                }
			                if(writeFile == stdout){
			                	fflush(stdout);
			                }
					        lastAck = pkt_get_seqnum(receivingBuf[j]);
					        window_slide(receivingWindow);
					        update_binaryReceivingBuf(binaryReceivingBuf);
					        receivingEmptySlot++;
					        j++;
				        }
				        /*
	        			for(k=0;k<MAX_WINDOW_SIZE;k++){
	        				fprintf(stderr, "%d", receivingWindow[k]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/

	        			seqnum = (lastAck+1)%(MAX_SEQNUM+1);
				        create_packet_ack(thePkt_ack, seqnum, receivingEmptySlot);

				        char buf_ack[528];
				        *bufLen = 528;
				        pkt_status_code statEncode = pkt_encode(thePkt_ack, buf_ack, bufLen);
			            if(statEncode != 0){
			                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet ack");
			                free_all();
			                return;
			            }

			            if(write(fds[1].fd, buf_ack, *bufLen) == -1){
			                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
			                free_all();
			                return;
			            }
			            fprintf(stderr, "%s%d\n", "Envoi - pkt ack - ", seqnum);
	        		}else{
	        			if(receivingWindowSize >1){
	        				memcpy(receivingBuf[index], thePkt, sizeof(struct pkt));
	        				receivingEmptySlot--;
		        			binaryReceivingBuf[index] = 1;

		        			seqnum = (lastAck+1)%(MAX_SEQNUM+1);
		        			create_packet_ack(thePkt_ack, seqnum, receivingEmptySlot);

		        			char buf_ack[528];
		        			*bufLen = 528;
					        pkt_status_code statEncode = pkt_encode(thePkt_ack, buf_ack, bufLen);
				            if(statEncode != 0){
				                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet ack");
				                free_all();
				                return;
				            }

				            if(write(fds[1].fd, buf_ack, *bufLen) == -1){
				                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
				                free_all();
				                return;
				            }
				            fprintf(stderr, "%s%d\n", "Envoi - pkt ack - ", seqnum);
	        			}
	        		}
			    }else if(pkt_get_type(thePkt) == PTYPE_ACK){
			    	//Si ack présent dans la window, je discard les n elem avant cet ack(celui-ci compris)
			    		//Je slide la sending window de n et discard les n premiers elem de sending buf
			    	receivingAvaiableSlotCount = pkt_get_window(thePkt);
			    	int thisSeqnum = pkt_get_seqnum(thePkt);
			    	lastAckSeqnum = thisSeqnum;
			    	fprintf(stderr, "%s%d\n", "Réception - pkt ack - ", thisSeqnum);

			    	int index = find_pkt(thisSeqnum, sendingWindow);
			    	if(index == -1){
			    		fprintf(stderr, "%s\n", "L'ack correspond à un seqnum qui n'est pas présent dans la sendingWindow");
			    		if(thisSeqnum == (sendPktCount+1) % (MAX_SEQNUM+1)){
			    			fprintf(stderr, "%s\n", "Mais on peut quand meme envoyer le paquet suivant");
				    		index = MAX_WINDOW_SIZE;
				    	}
			    	}
			    	while(index != 0){
			    		sendBufCount--;
			    		window_slide(sendingWindow);
			    		update_binaryReceivingBuf(binaryReceivingBuf);
			    		update_sendingTime(sendingTime);
			    		index--;
			    	}
			    	continue;
			    }else if(pkt_get_type(thePkt) == PTYPE_NACK){
			    	//On renvoit le paquet present dans le sending buf correspondant au seqnum
			    	int thisSeqnum = pkt_get_seqnum(thePkt);
			    	fprintf(stderr, "%s%d\n", "Réception - pkt nack - ", thisSeqnum);
			    	lastAckSeqnum = thisSeqnum;
			    	for(i=0;i<MAX_WINDOW_SIZE;i++){
			    		if(pkt_get_seqnum(sendingBuf[i]) == thisSeqnum){
			    			char buf_data[528];
			    			*bufLen = 528;
			    			pkt_status_code statEncode = pkt_encode(sendingBuf[i], buf_data, bufLen);
				            if(statEncode != 0){
				                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet data");
				                free_all();
				                return;
				            }

				            if(write(fds[1].fd, buf_data, *bufLen) == -1){
				                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
				                free_all();
				                return;
				            }
				            fprintf(stderr, "%s%d\n", "Renvoi - pkt data - ", thisSeqnum);
			    		}
			    	}
			    }
	        }  
        }
    }

    free_all();
}
