/**
 * WebServer, a small and light web server.
 * 
 * Copyright (C) 2014 Felipe Paul Martins and Joachim Schmidt
 * 
 * This file is part of WebServer.
 * 
 * WebServer is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * WebServer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with WebServer. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Authors: Felipe Paul Martins, Joachim Schmidt
 */

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
#include "include/process_management.h"
#include "include/server_const.h"
#include "include/http.h"
#include "include/histo.h"

//extern struct queue_hist *q_log;

extern char *port_srv;
extern char *chemin_fichiers;
extern int  taille_log;
extern int  max_connexion;

// structure contenant les logs.
struct queue_hist *q_log = NULL;

/*
 * La fonction create_socket_stream ouvre une socket IPv4 et une socket
 * IPv6 sur un port choisi.
 * Ces deux sockets sont renvoyées sous la forme d'un tableau contenant
 * deux entiers (descripteurs de fichier).
 */

int* create_socket_stream(const char *host_name, const char *serv_port, 
						  const char *proto_name)
{
	int *sock; // Pointeur sur les deux sockets (IPv4 et IPv6).
	int port;
	int status;
	int i;
	int yes = 1;
	int reuse_addr = 1;
	int one_more_time = 4; // Nombre de tentative si l'adresse est déjà utilisé.

	struct hostent  *hostent;
	struct servent  *servent;
	struct protoent *protoent;
	
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *rp;

	// Initialisation de la structure addrinfo à zéro.
	memset(&hints, 0, sizeof(struct addrinfo));
	// Paramètrage de la structure addrinfo.
	hints.ai_family   = AF_UNSPEC; // IPv4 ou IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP.
	hints.ai_flags 	  = AI_PASSIVE;
	hints.ai_protocol = 0;

	// Allocation de mémoire pour stocker les descripteurs des deux sockets.
	sock = calloc(2, sizeof(int));

	// Initialisation de la structure hostent à partir du nom d'hôte de la
	// machine locale.
	if(host_name != NULL)
		if((hostent = gethostbyname(host_name)) == NULL)
		{
			perror("gethostbyname");
			return NULL;
		}

	// Initialisation de la structure protoent à partir du nom du protocole
	// utilisé (TCP).
	if((protoent = getprotobyname(proto_name)) == NULL)
	{
		perror("getprotobyname");
		return NULL;
	}

	// Initialisation de la structure servent à partie du numéro du port.
	// Cette structure est utile si on utilise un numéro de port inférieur
	// à 1024. Dans ce cas, il faut que le programme soit Set-UID root ou
	// qu'il possède la capacité CAP_NET_BIND_SERVICE.
	if(serv_port != NULL)
	{
		// Test de la validité du numéro de port.
		if(sscanf(serv_port, "%d", &port) == 1)
		{
			if(port < 0)
			{
				fprintf(stderr, "Le numéro de port doit être un nombre positif.\n");
				return NULL;
			}
			else if(port < NUM_PORT_MIN) // Uniquement si CAP_NET_BIND_SERVICE ou Set-UID root.
			{
				// Initialisation de la structure servent.
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

	// Initialisation de la structure addrinfo à partir du numéro
	// du port d'écoute du serveur web.
	if((status = getaddrinfo(NULL, serv_port, &hints, &res)) != 0)
	{
		perror("getaddrinfo");
		fprintf(stderr, "%s\n", gai_strerror(status));
		return NULL;
	}

	// Affichage des différents services (IPv4 ou IPv6) disponible sur la machine hôte à
	// partir de la structure addrinfo.
	for(rp = res, i = 1; rp != NULL; rp = rp->ai_next, i++)
	{
		fprintf(stdout, "sevice [%d] = %s\n", i, 
				(rp->ai_family != 2) ? ((rp->ai_family != 10) ? "UNDEF" : "IPv6") : "IPv4");
	}

	// Boucle de création des sockets et affectations des addresses sur les
	// sockets précédement crées.
	for(rp = res, i = 0; rp != NULL; rp = rp->ai_next, i++)
	{
		// En cas d'echec du bind et si la variable globale errno contient EADDRINUSE
		// on a quatre tentative pour binder à nouveau.
		one_more_time = 4;

		// Création d'une nouvelle socket. En cas d'échec on passe à l'itération
		// suivante.
		if((sock[i] = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0)
			continue;

		// Permet de relancer immédiatement un serveur TCP que l'on vient d'interrompre.
		setsockopt(sock[i], SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int));

		// On force la socket AF_INET6 a binder uniquement sur l'adresse IPv6
		// de la machine.
		if(rp->ai_family == AF_INET6)
			setsockopt(sock[i], SOL_IPV6, IPV6_V6ONLY, &yes, sizeof(int));

		do
		{
			// Affectation d'une adresse à la nouvelle socket. En cas d'échec on teste
			// la variable globale errno. Si celle-ci contient l'erreur EADDRINUSE on
			// essaye une nouvelle tentative après avoir effectué une pause d'une demi
			// seconde.
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
				// Condition de sortie de la boucle while.
				one_more_time = 0;
			}
		}while(one_more_time > 0);
	}

	// On retourne un pointeur sur deux sockets. Une en IPv4 et une en IPv6.
	return sock;
}

/*
 * La fonction tcp_server appel la fonction create_socket_stream. Lorsque les deux sockets sont
 * renvoyée, la fonction met un processus en écoute sur la socket liée à l'adresse IPv4 et un
 * autre processus en écoute sur la socket liée à l'adresse IPv6.
 * Lorsqu'un client se connecte sur l'une des deux sockets, un nouveau processus est crée pour
 * traiter indépendament cette connexion.
 */

int tcp_server(void *arg)
{
	int *sock_name;
	//struct serv_param *param = (struct serv_param *)arg;
	//int sock_connected;

	//struct sockaddr_in addr;
	//socklen_t len;

	struct server_process *psp;

	// Création de deux sokets en fonction du numéro de port et du protocole
	// TCP.
	if((sock_name = create_socket_stream(NULL, ((struct serv_param *)arg)->port_srv, "tcp")) == NULL)
	{
		fprintf(stderr, "L'appel à create_socket_stream a échoué.\n");
		return -1;
	}

	// Etant donné que le processus père n'effectue pas de wait sur ses fils,
	// on ignore le signal SIGCHLD afin que les processus fils ne deviennent
	// pas des zombies.
	signal(SIGCHLD, SIG_IGN);

	psp = init_server_process(ipv4_process, ipv6_process, (void *)sock_name);
	call_fork(fork(), psp);
	delete_server_process(psp);

	return 0;
}

/*
 * La fonction close_tcp_server renvoie pour l'instant la valeur de 0.
 */

int close_tcp_server(void)
{
	return 0;
}

/*
 * La fonction process_connection affiche l'adresse IP du serveur local et
 * l'adresse IP du client distant. Elle envoie également un message au client
 * contenant sa propre adresse IP. Pour finir, elle lit le flux envoyé sur la
 * socket par le client.
 *
 * Post interressant: http://stackoverflow.com/a/17850812
 */

void process_connection(int sock)
{

	char ipcli[TAILLE_READ_BUFFER];
	int keepAlive;

	q_log = new_queue(get_size_queue, get_max_size_queue,
                      push, pop,
                      get_elem, get_nb_elem,
                      get_size_elem, taille_log);

	// Affichage de l'adresse IP du serveur local.
	fprintf(stdout, "Connexion : locale   ");
	print_socket_address(sock, LOCAL, NULL);

	// Affichage de l'adresse IP du client distant.
	fprintf(stdout, "\t    distante ");
	print_socket_address(sock, DISTANT, ipcli);

	do
	{
		// Traite la requête HTTP
		keepAlive = processHttp(sock, ipcli);
	} while (keepAlive);

	// Fermeture de la socket
	close(sock);
}

/*
 * La fonction print_socket_address affiche l'adresse IPv4 ou IPv6 du serveur local ou
 * l'adresse IPv4 ou IPv6 du client distant. Cette adresse peut être copié dans un buffer
 * externe à la fonction.
 */

int print_socket_address(int sock, int where, char *ext_buffer)
{
	// Structures pour contenir les adresse IPv4 ou IPv6.
	struct sockaddr_in 	*addr4 = NULL;
	struct sockaddr_in6 *addr6 = NULL;
	struct sockaddr 	*addr  = NULL;
	socklen_t len;

	char buffer [TAILLE_READ_BUFFER];
	char bufferp[TAILLE_READ_BUFFER];

	// Pointeur de fonction, pointant soit sur l'appel système getsockname ou sur l'appel
	// système getpeername.
	int (*getname)(int, struct sockaddr *, socklen_t *);

	addr = calloc(1, sizeof(struct sockaddr));
	len = sizeof(struct sockaddr);

	// Si where == LOCAL, alors on utilisera l'appel système getsockname pour obtenir
	// les infromations du serveur local liée à la socket.
	// Si where == DISTANT, alors on utilisera l'appel système getpeername pour obtenir
	// les information du client distant liée à la socket.
	if(where == 0)
		getname = getsockname;
	else if(where == 1)
		getname = getpeername;
	else
	{
		fprintf(stderr, "Le paramètre where est erroné.\n");
		return -1;
	}

	// On cherche à obetnir la famille de la socket.
	if(getname(sock, addr, &len) < 0)
	{
		perror("getsockname or getpeername");
		return -1;
	}

	// On teste la famille de la socket.
	switch(addr->sa_family)
	{
		case AF_INET : // Famille == IPv4.
			len = sizeof(struct sockaddr_in);
			addr4 = calloc(1, sizeof(struct sockaddr_in));
			// On rempli la structure sockaddr_in correspondant à une socket connectée
			// sur une adresse IPv4.
			if(getname(sock, (struct sockaddr *)addr4, &len) < 0)
			{
				perror("getsockname");
				return -1;
			}
			// Network to presentation.
			// Conversion de l'adresse IP au format réseau en notation pointée.
			inet_ntop(AF_INET, (void *)&(addr4->sin_addr), buffer, 256);
			strcpy(bufferp, "IPv4 = ");
			strcat(bufferp, buffer);

			break;

		case AF_INET6 : // Famille == IPv6.
			len = sizeof(struct sockaddr_in6);
			addr6 = calloc(1, sizeof(struct sockaddr_in6));
			// On rempli la structure sockaddr_in correspondant à une socket connectée
			// sur une adresse IPv6.
			if(getname(sock, (struct sockaddr *)addr6, &len) < 0)
			{
				perror("getsockname");
				return -1;
			}
			// Network to presentation.
			// Conversion de l'adresse IP au format réseau en notation pointée.
			inet_ntop(AF_INET6, (void *)&(addr6->sin6_addr), buffer, 256);
			strcpy(bufferp, "IPv6 = ");
			strcat(bufferp, buffer);

			break;
	}

	fprintf(stdout, "%s, Port = %u\n", bufferp, 
			(addr4 != NULL) ? ntohs(addr4->sin_port) : ntohs(addr6->sin6_port));

	// Copie du buffer dans un buffer externe à la fonction.
	if(ext_buffer != NULL)
	{
		strcpy(ext_buffer, buffer);
		// strcat(ext_buffer, "\n");
	}

	if(addr4 != NULL)
		free(addr4);
	if(addr6 != NULL)
		free(addr6);

	free(addr);

	return 0;
}
