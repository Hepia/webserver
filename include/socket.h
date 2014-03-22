#ifndef _SOCKET_H_
#define _SOCKET_H_

#define NUM_PORT_MAX 		65534
#define NUM_PORT_MIN 		1024
#define TAILLE_READ_BUFFER	255
#define TAILLE_FILE_ECOUTE	10

int create_socket(char *port);

#endif