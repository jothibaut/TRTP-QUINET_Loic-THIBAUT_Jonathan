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
struct pkt* (sendingBuf[WINDOW_SIZE]);
struct pkt* (receivingBuf[WINDOW_SIZE]);

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

	int timeOut = 7000; //Temps d'attente de poll --> [ms]
    struct pollfd fds[2];
    int ret;
    int seqnum = 0;
    int sendPktCount = 0; //Nbr de pkt envoyés
    int sendBufCount = 0; // Nombre de places occupées dans le sendingBuf
    int lastAck = 0;
    int i;

    int binaryReceivingBuf[WINDOW_SIZE];
    for(i=0;i<WINDOW_SIZE;i++){
    	binaryReceivingBuf[i] = 0;
    }

    clock_t sendingTime[WINDOW_SIZE];
    
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

	int sendingWindow[WINDOW_SIZE];
	int receivingWindow[WINDOW_SIZE];
	for(i=0;i<WINDOW_SIZE;i++){
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
            return;
        }else if(ret == 0){
        	fprintf(stderr, "%s\n", "Timed out");
        	return;
        }

        if (fds[0].revents & POLLIN){ //STDIN ou FILE
        	//If sending buffer rempli
        		//si le RTO d'un ou pls pkt a expiré
        			//On renvoit ce(s) pkt
        		//on passe à la lecture sur sfd
        	if(sendBufCount == WINDOW_SIZE){
        		for(i=0;i<WINDOW_SIZE;i++){
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
        			}
        		}
        	}else{
        		char buf_payload[512];
        		int r = read(fds[0].fd, buf_payload, 512);
		    	if(r==EOF){
					fprintf(stderr, "%s\n", "stdin a atteint EOF");
	                
	                //crée un packet de ou la longueur vaut 0 (signifie une demande de déconnexion)
	                create_packet_deco(thePkt, seqnum);
	            	if(thePkt == NULL){
	                	fprintf(stderr, "%s\n", "Echec lors de la creation du paquet de déconnexion");
	                	free_all();
	                	return;
	                }
	                
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
	                
	                sendPktCount++;
	                free_all();
	                return;
	                //fin de la demande de deconexion et se deconnecte direct
	            }

	            if(r == 0 && readFd!=0){
	                fprintf(stderr, "%s\n", "a atteint la fin du fichier");
	                
	                //crée un packet de ou la longueur vaut 0 (signifie une demande de déconnexion)
	                create_packet_deco(thePkt, seqnum);
	            	if(thePkt == NULL){
	                	fprintf(stderr, "%s\n", "Echec lors de la creation du paquet de déconnexion");
	                	free_all();
	                	return;
	                }
	                
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
	                
	                sendPktCount++;
	                free_all();
	                return;
	                //fin de la demande de deconexion et se deconnecte direct
	            }
	            else if(r == -1){
	                fprintf(stderr, "%s\n", "lecture stdin : erreur");
	                free_all();
	                return;
	            }

	            int w = fwrite(buf_payload, r, sizeof(char), stdout);
				if(w == 0){
					fprintf(stderr, "%s\n", "Error : écriture");
				}
	            seqnum = sendPktCount % (MAX_SEQNUM+1);

	            create_packet_data(thePkt, buf_payload, seqnum, r);
	            if(thePkt == NULL){
	                fprintf(stderr, "%s\n", "Echec lors de la creation du paquet");
	                free_all();
	                return;
	            }
	            memcpy(sendingBuf[sendPktCount%WINDOW_SIZE], thePkt, sizeof(struct pkt));
	            sendBufCount++;
	            sendingTime[sendPktCount%WINDOW_SIZE] = clock();

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
	            
	            sendPktCount++;
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
	        else if(r == 0){
	            fprintf(stderr, "%s\n", "le socket a atteint EOF");
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
	        		fprintf(stderr, "%s\n", "paquet invalide");
	        		continue;
	        	}else{
	        		fprintf(stderr, "%s\n", "paquet tronqué");
	        		create_packet_nack(thePkt_nack, seqnum);

	        		char buf_nack[528];
	        		*bufLen = 528;
		        	pkt_status_code statEncode = pkt_encode(thePkt_nack, buf_nack, bufLen);
		            if(statEncode != 0){
		                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet ack");
		                free_all();
		                return;
		            }

		            if(write(fds[1].fd, buf_nack, *bufLen) == -1){
		                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
		                free_all();
		                return;
		            }
	        	}
	        }else if(pkt_get_length(thePkt)==0){
	        		//on a recu un packet symbolisant la deconnexion donc on se deconnecte aussi
	        		return;        
	        }else{ //Cas classique
	        	if(pkt_get_type(thePkt) == PTYPE_DATA){

	        		int index = find_pkt(pkt_get_seqnum(thePkt), receivingWindow);
	        		if(index == -1){
	        			//envoit paquet avec lastAck
	        			fprintf(stderr, "%s\n", "Le paquet reçu ne figure pas dans le receivingBuf");
	        			int j;
	        			for(j=0;j<WINDOW_SIZE;j++){
	        				fprintf(stderr, "%d", receivingWindow[j]);
	        			}
	        			fprintf(stderr, "\n");
	        			create_packet_ack(thePkt_ack, lastAck);

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
	        		}else if(index == 0){
	        			//
	        			// Possiblement à écrire dans un file
	        			//
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
	        			for(k=0;k<WINDOW_SIZE;k++){
	        				fprintf(stderr, "%d", receivingWindow[k]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/
				        window_slide(receivingWindow);
				        /*
				        for(k=0;k<WINDOW_SIZE;k++){
	        				fprintf(stderr, "%d", receivingWindow[k]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/

				        int j = 1;
				        while(binaryReceivingBuf[j] == 1 && WINDOW_SIZE > 1){ //Vider le buffer
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
					        j++;
				        }
				        /*
	        			for(k=0;k<WINDOW_SIZE;k++){
	        				fprintf(stderr, "%d", receivingWindow[k]);
	        			}
	        			fprintf(stderr, "\n");
	        			*/

				        create_packet_ack(thePkt_ack, lastAck);

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
	        		}else{
	        			if(WINDOW_SIZE >1){
	        				memcpy(receivingBuf[index], thePkt, sizeof(struct pkt));
		        			binaryReceivingBuf[index] = 1;

		        			create_packet_ack(thePkt_ack, lastAck);

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
	        			}
	        		}
			    }else if(pkt_get_type(thePkt) == PTYPE_ACK){
			    	//Si ack présent dans la window, je discard les n elem avant cet ack(celui-ci compris)
			    		//Je slide la sending window de n et discard les n premiers elem de sending buf
			    	int index = find_pkt(pkt_get_seqnum(thePkt), sendingWindow);
			    	if(i == -1){
			    		fprintf(stderr, "%s\n", "L'ack correspond à un seqnum qui n'est pas présent dans la sendingWindow");
			    	}
			    	while(index != -1){
			    		sendBufCount--;
			    		window_slide(sendingWindow);
			    		update_binaryReceivingBuf(binaryReceivingBuf);
			    		update_sendingTime(sendingTime);
			    		index--;
			    	}
			    	continue;
			    }else if(pkt_get_type(thePkt) == PTYPE_NACK){
			    	//On renvoit le paquet present dans le sending buf correspondant au seqnum
			    	for(i=0;i<WINDOW_SIZE;i++){
			    		if(pkt_get_seqnum(receivingBuf[i]) == pkt_get_seqnum(thePkt)){
			    			char buf_data[528];
			    			*bufLen = 528;
			    			pkt_status_code statEncode = pkt_encode(receivingBuf[i], buf_data, bufLen);
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
			    		}
			    	}
			    }
	        }  
        }
    }

    free_all();
}
