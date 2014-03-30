#ifndef _SOCKET_H_
#define _SOCKET_H_

#define NUM_PORT_MAX 		65534
#define NUM_PORT_MIN 		1024
#define TAILLE_READ_BUFFER	256
#define TAILLE_FILE_ECOUTE	128 // Maximum sous Linux.

#define LOCAL 	0
#define DISTANT 1

int* create_socket_stream(const char *host_name, const char *serv_port, const char *proto_name);
int tcp_server(const char *port);
int close_tcp_server(void);
void process_connection(int sock);
int print_socket_address(int sock, int where, char *ext_buffer);

#endif