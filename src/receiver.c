#include <stdlib.h> /* EXIT_X */
#include <stdio.h> /* fprintf */
#include <unistd.h> /* getopt */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "real_address.h"
#include "create_socket.h"
#include "read_write_loop.h"
#include "wait_for_client.h"
#include "packet_interface.h"
#include "create_packet.h"


int main(int argc, char *argv[])
{
	int port = 12345;
	int opt;
	char *host = "::1"; //localhost
	char *file;
	FILE* writeFile = stdout;
	int isFile =0;
	int sfd;

	if(argc < 3){
		fprintf(stderr, "%s\n", "Erreur : nombre d'arguments insuffisant");
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
			case 'f':
				isFile = 1;
				file = optarg;
				break;
		}
	}

	
	if(isFile == 1){
		writeFile = fopen(file, "w+");
		if(writeFile == NULL){
			fprintf(stderr, "%s\n", "Echec lors de l'ouverture du fichier de lecture");
			return EXIT_FAILURE;
		}
		host = argv[3];
		port = atoi(argv[4]);
	}else{
		host = argv[1];
		port = atoi(argv[2]);
	}

	/* Resolve the hostname */
	struct sockaddr_in6 addr;
	const char *err = real_address(host, &addr); //Fonction a implémenter
	if (err) {
		fprintf(stderr, "Could not resolve hostname %s: %s\n", host, err);
		return EXIT_FAILURE;
	}
	
	sfd = create_socket(&addr, port, NULL, -1); /* Bound */
	if (sfd > 0 && wait_for_client(sfd) < 0) { /* Connected */ //Si j'ai un socket mais qu'il y a une erreur dans wait_for_client
		fprintf(stderr,
				"Could not connect the socket after the first message.\n");
		close(sfd);
		return EXIT_FAILURE;
	}
	if (sfd < 0) {
		fprintf(stderr, "Failed to create the socket!\n");
		return EXIT_FAILURE;
	}

	read_write_loop(sfd, -1, writeFile);

	if(isFile == -1){
		if(fclose(writeFile) != 0){
			fprintf(stderr, "%s\n", "Echec lors de la fermeture du fichier d'écriture");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;


}