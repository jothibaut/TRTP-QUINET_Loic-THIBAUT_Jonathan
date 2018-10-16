#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "create_socket.h"

/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr,
                 int src_port,
                 struct sockaddr_in6 *dest_addr,
                 int dst_port){
    if(dst_port > 0){
        dest_addr->sin6_port = (in_port_t) htons(dst_port);
    }
    
    if(src_port > 0){
        source_addr->sin6_port = (in_port_t) htons(src_port);
    }
    
    int sfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP); //Type : p-ê SOCK_STREAM,0
    if(sfd <0){
        fprintf(stderr, "%s\n", "Erreur : appel socket()");
        return -1;
    }
    
    if(source_addr != NULL){
        
        if(bind(sfd,(struct sockaddr *) source_addr,(socklen_t) sizeof(struct sockaddr_in6)) != 0){
            fprintf(stderr, "%s\n", "Erreur : bind source au socket");
            return -1;
        }
    }
        
    if(dest_addr != NULL){
        fprintf(stderr, "%s\n", "pintade");
        if(connect(sfd,(struct sockaddr *) dest_addr,(socklen_t) sizeof(struct sockaddr_in6)) != 0){
            fprintf(stderr, "%s\n", "Erreur : connect socket à la dest");
            return -1;
        }
    }
    //Accept ?  
    return sfd;
}
