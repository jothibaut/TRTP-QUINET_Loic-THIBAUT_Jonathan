#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include "wait_for_client.h"


int wait_for_client(int sfd){
    
    struct sockaddr_in6 src_addr;
    socklen_t fromlen = sizeof(struct sockaddr_in6);
    int recv_bytes;
    
    recv_bytes = recvfrom(sfd, NULL, 0, MSG_PEEK, (struct sockaddr *) &src_addr, &fromlen);

    if(recv_bytes == -1){
        fprintf(stderr, "Erreur : recvfrom");
        return -1;
    }

    if(connect(sfd, (struct sockaddr *) &src_addr, fromlen) == -1){
        fprintf(stderr, "Erreur : connect");
        return -1;
    }
    
    return 0;
}