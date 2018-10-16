#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>

#include "real_address.h"

/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval){
    struct addrinfo *result;
    struct addrinfo *hints = NULL;
    hints = (struct addrinfo *) malloc(sizeof(struct addrinfo));
    if(hints == NULL){
        const char *error = "Erreur : malloc";
        return error;
    }
    memset(hints,0,sizeof(struct addrinfo));
    hints->ai_family = AF_INET6;
    hints->ai_flags = AI_CANONNAME; //p-ê AI_NUMERICHOST
    hints->ai_socktype = SOCK_DGRAM; //p-ê 0
    hints->ai_protocol = 0;
    hints->ai_canonname = NULL;
    hints->ai_addr = (struct sockaddr *) rval;
    hints->ai_next = NULL;
    
    int s = getaddrinfo(address, NULL, hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return "Erreur : getaddrinfo";
    }
    
    struct sockaddr_in6 *temp = (struct sockaddr_in6*) result->ai_addr;
    memcpy(rval, temp, sizeof(*temp));
    
    freeaddrinfo(result);
    return NULL;
}