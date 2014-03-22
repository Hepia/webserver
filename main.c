#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/socket.h"

int main(int argc, char *argv[]) {

	if (argc !=2) {
		fprintf(stderr, "usage: webserver <portnumber>\n");
	}

	create_socket(atoi(argv[1]));
    return EXIT_SUCCESS;
}
