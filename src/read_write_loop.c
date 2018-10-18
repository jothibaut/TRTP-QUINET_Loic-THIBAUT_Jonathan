#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Réalisé avec l'aide de Thomas Reniers

void read_write_loop(int sfd){
    struct pollfd fds[2];
    int ret;
    char buf[1024];

    /* watch stdin for input */
    fds[0].fd = 0; //STDIN_FILENO
    fds[0].events = POLLIN;

    /* watch stdout for ability to write */
    fds[1].fd = sfd;
    fds[1].events = POLLIN;
    
    for(;;){
        ret = poll(fds, 2, -1);
        if (ret == -1) {
            fprintf(stderr, "%s\n", "Echec lors de l'exécution de poll");
            return;
        }
        if (fds[0].revents & POLLIN){
            int r = read(0, buf, 1024);
            if(r == EOF){
                fprintf(stderr, "%s\n", "stdin a atteint EOF");
                return;
            }
            else if(r == -1){
                fprintf(stderr, "%s\n", "lecture stdin : erreur");
                return;
            }


            if(write(sfd, buf, r) == -1){
                fprintf(stderr, "%s\n", "Echec lors de l'écriture sur le le socket");
                return;
            }
        }
        
        if (fds[1].revents & POLLIN){
        int r = read(sfd, buf, 1024);
        if(r == -1){
            fprintf(stderr, "%s\n", "Echec lors de la lecture du socket");
            return;
        }
        else if(r == EOF){
            fprintf(stderr, "%s\n", "le socket a atteint EOF");
            return;
        }
        if(fwrite(buf, sizeof(char), r, stdout) == 0){
            fprintf(stderr, "%s\n", "Erreur lors de l'écriture sur stdout");
            return;
        }
        fflush(stdout);
        }
    } 
}