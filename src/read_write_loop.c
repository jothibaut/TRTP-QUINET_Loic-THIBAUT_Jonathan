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
    int seqnum = 0;
    size_t *bufLen;
    struct pkt* thePkt = NULL;

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
            seqnum++;

            *bufLen = 1024;
            pkt_status_code statEncode = pkt_encode(thePkt, buf, bufLen);
            if(statEncode != 0){
                fprintf(stderr, "%s\n", "Echec lors de l encodage du paquet");
                return;
            }

            if(write(sfd, buf, *bufLen) == -1){
                fprintf(stderr, "%s\n", "Echec lors de l ecriture sur le socket");
                return;
            }
        }
        
        if (fds[1].revents & POLLIN){ //SFD
        int r = read(sfd, buf, 1024);
        if(r == -1){
            fprintf(stderr, "%s\n", "Echec lors de la lecture du socket");
            return;
        }
        else if(r == EOF){
            fprintf(stderr, "%s\n", "le socket a atteint EOF");
            return;
        }

        pkt_status_code statDecode = pkt_decode(buf, r, thePkt);
        if(statDecode != 0){
            fprintf(stderr, "%s\n", "Echec lors du decodage du paquet");
                return;
        }

        if(fwrite(buf, sizeof(char), r, stdout) == 0){
            fprintf(stderr, "%s\n", "Erreur lors de l ecriture sur stdout");
            return;
        }
        fflush(stdout);
        }
    } 
}