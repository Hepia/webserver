#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <errno.h>

#include "include/socket.h"

int create_socket(char *port) {

	struct addrinfo hints, *result, *rp;
	int status;

	char read_buffer[TAILLE_READ_BUFFER];


	if (atoi(port) < NUM_PORT_MIN) {
		fprintf(stderr, "Port %s is lower than %d\n", port, NUM_PORT_MIN);
		exit(101);
	}
	if (atoi(port) > NUM_PORT_MAX) {
		fprintf(stderr, "Port %s is higher than %d\n", port, NUM_PORT_MAX);
		exit(102);
	}


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;		// Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags = AI_PASSIVE;		// For wildcard IP address
	hints.ai_protocol = 0;				// Any protocol


	if ((status = getaddrinfo(NULL, port, &hints, &result)) != 0 ){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		exit(103);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		
		status = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (status == -1)
			continue;

		if (bind(status, rp->ai_addr, rp->ai_addrlen) == 0)
			break;	// Socket créée

		close(status);
	}

	if (rp == NULL) {
		fprintf(stderr, "Could not bind\n");
		exit(104);
	}

	freeaddrinfo(result);

	struct sockaddr_storage peer_addr;
  	socklen_t peer_addr_len;

  	if(listen(status, TAILLE_FILE_ECOUTE) <0) {
		fprintf(stderr, "listen: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	while(1) {

		peer_addr_len = sizeof(struct sockaddr_storage);
		int new_sock = accept(status, (struct sockaddr *) &peer_addr, &peer_addr_len);

		if (new_sock < 0) {
			fprintf(stderr, "accept: %s\n", strerror(errno));
		}
		else {
			int n_recv;
			fprintf(stdout, "New connexion\n");
			while((n_recv = read(new_sock, read_buffer, TAILLE_READ_BUFFER)) > 0) {
					
				fprintf(stdout,read_buffer,n_recv);

				// Todo send data
			}
		}
	}	
}