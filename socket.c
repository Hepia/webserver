// Socket.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "include/socket.h"

int* create_socket_stream(const char *host_name, const char *serv_port, 
						  const char *proto_name)
{
	int *sock;
	int port;
	int status;
	int i;
	int yes = 1;
	int one_more_time = 4; // Nombre de tentative si l'adresse est déjà utilisé.

	struct hostent  *hostent;
	struct servent  *servent;
	struct protoent *protoent;
	
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family   = AF_UNSPEC; // IPv4 ou IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags 	  = AI_PASSIVE;
	hints.ai_protocol = 0;

	sock = calloc(2, sizeof(int));

	if(host_name != NULL)
		if((hostent = gethostbyname(host_name)) == NULL)
		{
			perror("gethostbyname");
			return NULL;
		}

	if((protoent = getprotobyname(proto_name)) == NULL)
	{
		perror("getprotobyname");
		return NULL;
	}

	if(serv_port != NULL)
	{
		if(sscanf(serv_port, "%d", &port) == 1)
		{
			if(port < 0)
			{
				fprintf(stderr, "Le numéro de port doit être un nombre positif.\n");
				return NULL;
			}
			else if(port < NUM_PORT_MIN) // Uniquement si CAP_NET_BIND_SERVICE ou Set-UID root.
			{
				if((servent = getservbyport(htons(port), protoent->p_name)) == NULL)
				{
					perror("getservbyport");
					return NULL;
				}
			}
			else if(port > NUM_PORT_MAX)
			{
				fprintf(stderr, "Le numéro de port est trop grand.\n");
				return NULL;
			}
		}
		else
		{
			perror("sscanf");
			return NULL;
		}
	}

	if((status = getaddrinfo(NULL, serv_port, &hints, &res)) != 0)
	{
		perror("getaddrinfo");
		fprintf(stderr, "%s\n", gai_strerror(status));
		return NULL;
	}

	for(rp = res, i = 1; rp != NULL; rp = rp->ai_next, i++)
	{
		fprintf(stdout, "sevice [%d] = %s\n", i, 
				(rp->ai_family != 2) ? ((rp->ai_family != 10) ? "UNDEF" : "IPv6") : "IPv4");
	}

	for(rp = res, i = 0; rp != NULL; rp = rp->ai_next, i++)
	{
		one_more_time = 4;

		if((sock[i] = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0)
			continue;

		do
		{
			if(rp->ai_family == AF_INET6)
			{
				setsockopt(sock[i], SOL_IPV6, IPV6_V6ONLY, &yes, sizeof(yes));
			}

			if(bind(sock[i], rp->ai_addr, rp->ai_addrlen) < 0)
			{
				if(errno == EADDRINUSE)
				{
					fprintf(stderr, "L'adresse affecté à la socket est déjà utilisée.\n");
					fprintf(stderr, "Nouvelle tentative d'Identification de la socket.\n");
					one_more_time--;
					usleep(500000);
				}
				else
				{	
					close(sock[i]);
					perror("bind");
					return NULL;
				}
			}
			else
			{
				fprintf(stdout, "La famille d'adresse %s a été affecté à la socket sock[%d].\n", 
						(rp->ai_addr->sa_family != 2) ? ((rp->ai_addr->sa_family != 10) ? "UNDEF" : "IPv6") : "IPv4", i);
				one_more_time = 0;
			}
		}while(one_more_time > 0);
	}

	return sock;
}

int tcp_server(const char *port)
{
	int *sock_name;
	int sock_connected[2];

	struct sockaddr_in addr;
	socklen_t len;

	if((sock_name = create_socket_stream(NULL, port, "tcp")) == NULL)
	{
		fprintf(stderr, "L'appel à create_socket_stream a échoué.\n");
		return -1;
	}

	signal(SIGCHLD, SIG_IGN);

	switch(fork())
	{
		case 0 : // Processus fils écoute sur IPv4.
			listen(sock_name[0], TAILLE_FILE_ECOUTE);
			fprintf(stdout, "Mon adresse >> ");

			if((print_socket_address(sock_name[0], LOCAL, NULL)) < 0)
			{
				fprintf(stderr, "L'appel à print_socket_address a échoué.\n");
				return -1;
			}

			while(!close_tcp_server())
			{
				len = sizeof(struct sockaddr_in);
				if((sock_connected[0] = accept(sock_name[0], (struct sockaddr *)&addr, &len)) < 0)
				{
					perror("accept");
					return -1;
				}

				switch(fork())
				{
					case 0 : // Processus fils.
						close(sock_name[0]);
						process_connection(sock_connected[0]);
						exit(EXIT_SUCCESS);

					case -1 :
						perror("fork");
						return -1;

					default : // Processus père.
						close(sock_connected[0]);

				}
			}

			break;

		case -1 :
			perror("fork");
			return -1;

		default : // Processus père écoute sur IPv6.
			listen(sock_name[1], TAILLE_FILE_ECOUTE);
			fprintf(stdout, "Mon adresse >> ");

			if((print_socket_address(sock_name[1], LOCAL, NULL)) < 0)
			{
				fprintf(stderr, "L'appel à print_socket_address a échoué.\n");
				return -1;
			}

			while(!close_tcp_server())
			{
				len = sizeof(struct sockaddr_in);
				if((sock_connected[1] = accept(sock_name[1], (struct sockaddr *)&addr, &len)) < 0)
				{
					perror("accept");
					return -1;
				}

				switch(fork())
				{
					case 0 : // Processus fils.
						close(sock_name[1]);
						process_connection(sock_connected[1]);
						exit(EXIT_SUCCESS);

					case -1 :
						perror("fork");
						return -1;

					default : // Processus père.
						close(sock_connected[1]);

				}
			}

			break;
	}

	return 0;
}

int close_tcp_server(void)
{
	return 0;
}

void process_connection(int sock)
{
	char buffer[TAILLE_READ_BUFFER];

	fprintf(stdout, "Connexion : locale ");
	print_socket_address(sock, LOCAL, NULL);

	fprintf(stdout, "		   distante ");
	print_socket_address(sock, DISTANT, buffer);

	write(sock, "Votre adresse : ", 16);
	write(sock, buffer, strlen(buffer));

	close(sock);
}

int print_socket_address(int sock, int where, char *ext_buffer)
{
	struct sockaddr_in 	*addr4 = NULL;
	struct sockaddr_in6 *addr6 = NULL;
	struct sockaddr 	*addr  = NULL;
	socklen_t len;

	char buffer [TAILLE_READ_BUFFER];
	char bufferp[TAILLE_READ_BUFFER];

	addr = calloc(1, sizeof(struct sockaddr));
	len = sizeof(struct sockaddr);
	
	if(where == 0)
	{
		if(getsockname(sock, addr, &len) < 0)
		{
			perror("getsockname");
			return -1;
		}
	}
	else if(where == 1)
	{
		if(getpeername(sock, addr, &len) < 0)
		{
			perror("getpeername");
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "Le paramètre where est erroné.\n");
		return -1;
	}

	switch(addr->sa_family)
	{
		case AF_INET :
			len = sizeof(struct sockaddr_in);
			addr4 = calloc(1, sizeof(struct sockaddr_in));
			if(getsockname(sock, (struct sockaddr *)addr4, &len) < 0)
			{
				perror("getsockname");
				return -1;
			}
			inet_ntop(AF_INET, (void *)&(addr4->sin_addr), buffer, 256);
			strcpy(bufferp, "IPv4 = ");
			strcat(bufferp, buffer);

			break;

		case AF_INET6 :
			len = sizeof(struct sockaddr_in6);
			addr6 = calloc(1, sizeof(struct sockaddr_in6));
			if(getsockname(sock, (struct sockaddr *)addr6, &len) < 0)
			{
				perror("getsockname");
				return -1;
			}
			inet_ntop(AF_INET6, (void *)&(addr6->sin6_addr), buffer, 256);
			strcpy(bufferp, "IPv6 = ");
			strcat(bufferp, buffer);

			break;
	}

	fprintf(stdout, "%s, Port = %u\n", bufferp, 
			(addr4 != NULL) ? ntohs(addr4->sin_port) : ntohs(addr6->sin6_port));

	if(ext_buffer != NULL)
	{
		strcpy(ext_buffer, bufferp);
		strcat(ext_buffer, "\n");
	}

	if(addr4 != NULL)
		free(addr4);
	if(addr6 != NULL)
		free(addr6);

	free(addr);

	return 0;
}