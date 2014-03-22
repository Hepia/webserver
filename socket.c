#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "include/socket.h"

int create_socket(int iPort) {


	if (iPort < NUM_PORT_MIN) {
		fprintf(stderr, "Port %d is lower than %d\n", iPort, NUM_PORT_MIN);
		return(-101);
	}
	if (iPort > NUM_PORT_MAX) {
		fprintf(stderr, "Port %d is higher than %d\n", iPort, NUM_PORT_MAX);
		return(-102);
	}


}