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

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "include/socket.h"
#include "include/process_management.h"
#include "include/server_const.h"

/*
 * La fonction init_server_process initialise une nouvelle sructure server_process
 * avec deux fonctions correspondant respectivement au code du processus fils et au
 * code du processus père. Elle référence également une ou plusieurs sockets.
 * la fonction retourne un pointeur sur la nouvelle structure server_process.
 */

struct server_process * init_server_process(int (*ptr_child_process)(void *), 
											int (*ptr_father_process)(void *),
											void *sock)
{
	// fils 		0 1
	// père 		1 0
	// fils et père 1 1

	struct server_process *psp;

	// Allocation de mémoire pour la structure server_process.
	if((psp = calloc(1, sizeof(struct server_process))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	// Référencement des fonctions à l'aide des pointeurs.
	psp->ptr_process[0] = ptr_child_process;
	psp->ptr_process[1] = ptr_father_process;
	psp->data = (void *)sock;
	psp->pid_child_process = -1;
	psp->c_and_f = 0;

	if(psp->ptr_process[0] != NULL)
		psp->c_and_f++;

	if(psp->ptr_process[1] != NULL)
		psp->c_and_f += 2;

	return psp;
}

/*
 * La fonction delete_server_process libère la structure server_process.
 */

void delete_server_process(struct server_process *ptr_sp)
{
	free(ptr_sp);
}

/*
 * La fonction call_fork permet de choisir la fonctions à executer en fonction si on
 * est dans le processus fils ou si l'on se trouve dans le processus père.
 */

int call_fork(int val, struct server_process *ptr_sp)
{
	// fils 		0 1
	// père 		1 0
	// fils et père 1 1

	if(val < 0) return -1;

	ptr_sp->pid_child_process = val; // Sauvegarde du processus fils;

	// Fonction pour processus père et fonction pour processus fils.
	if(ptr_sp->c_and_f == 3)
		(*ptr_sp->ptr_process[((!val) ? 0 : 1)])((void *)ptr_sp->data);

	// Fonction pour processus père.
	else if((ptr_sp->c_and_f == 2) && (val != 0))
		(*ptr_sp->ptr_process[1])((void *)ptr_sp->data);

	// Fonction pour processus fils.
	else if((ptr_sp->c_and_f == 1) && (val == 0))
		(*ptr_sp->ptr_process[0])((void *)ptr_sp->data);
	
	// Sinon pas de fonction spécifique ni pour processus père, ni pour processu fils.
	else
		return -1;

	return EXIT_SUCCESS;
}

/*
 * La fonction ipv4_process est le corp du processus fils qui écoutera les nouvelles
 * connexions en IPv4.
 */

int ipv4_process(void *data)
{
	struct sockaddr_in addr;
	socklen_t len;

	struct server_process *psp;

	int sock_connected;
	int *sock_name = (int *)data;
	// Couple de socket à passer en paramètre lors du fork.
	int **sock_process = calloc(2, sizeof(int *));

	sock_process[0] = &sock_name[0]; // Indice [0] représente la socket IPv4.
	sock_process[1] = &sock_connected;

	// Initialisation de la structure server_process.
	psp = init_server_process(child_process, father_process, (void *)sock_process);

	listen(sock_name[0], TAILLE_FILE_ECOUTE);

	fprintf(stdout, "Mon adresse >> ");

	// Affichage de l'adresse IP locale.
	if((print_socket_address(sock_name[0], LOCAL, NULL)) < 0)
	{
		fprintf(stderr, "L'appel à print_socket_address a échoué.\n");
		return -1;
	}

	// On boucle tantque le serveur n'est pas interrompu.
	while(!close_tcp_server())
	{
		len = sizeof(struct sockaddr_in);
		// Connexion d'un client de la file d'attente sur une nouvelle socket.
		if((sock_connected = accept(sock_name[0], (struct sockaddr *)&addr, &len)) < 0)
		{
			perror("accept");
			return -1;
		}

		// La connexion liée à la nouvelle socket créer par l'appel système accept sera traitée
		// par le processus fils.

		call_fork(fork(), psp);
	}

	delete_server_process(psp);
	close(sock_name[0]);

	return EXIT_SUCCESS;
}

/*
 * La fonction ipv6_process est le corp du processus fils qui écoutera les nouvelles
 * connexions en IPv6.
 */

int ipv6_process(void *data)
{
	struct sockaddr_in addr;
	socklen_t len;

	struct server_process *psp;

	int sock_connected;
	int *sock_name = (int *)data;
	// Couple de socket à passer en paramètre lors du fork.
	int **sock_process = calloc(2, sizeof(int *));

	sock_process[0] = &sock_name[1]; // Indice [1] représente la socket IPv6.
	sock_process[1] = &sock_connected;

	// Initialisation de la structure server_process.
	psp = init_server_process(child_process, father_process, (void *)sock_process);

	listen(sock_name[1], TAILLE_FILE_ECOUTE);
	fprintf(stdout, "Mon adresse >> ");

	// Affichage de l'adresse IP locale.
	if((print_socket_address(sock_name[1], LOCAL, NULL)) < 0)
	{
		fprintf(stderr, "L'appel à print_socket_address a échoué.\n");
		return -1;
	}

	// On boucle tantque le serveur n'est pas interrompu.
	while(!close_tcp_server())
	{
		len = sizeof(struct sockaddr_in);
		// Connexion d'un client de la file d'attente sur une nouvelle socket.
		if((sock_connected = accept(sock_name[1], (struct sockaddr *)&addr, &len)) < 0)
		{
			perror("accept");
			return -1;
		}

		// La connexion liée à la nouvelle socket créer par l'appel système accept sera traitée
		// par le processus fils.

		call_fork(fork(), psp);
	}

	delete_server_process(psp);
	close(sock_name[1]);

	return EXIT_SUCCESS;
}

/*
 * La fonction child_process correspond à un nouveau processus fils qui s'occupera de
 * traiter une nouvelle connexion sur l'une socket IPv4 ou IPv6.
 */

int child_process(void *data)
{
	int **sock = (int **)data;

	close(*(sock[0]));
	process_connection(*(sock[1])); // Traitement de la connexion.
	exit(EXIT_SUCCESS);
}

/*
 * La fonction father_process correspond au processus père qui seulement fermer
 * sa référence sur la nouvelle socket et va retourner en attente sur une nouvelle
 * connexion.
 */

int father_process(void *data)
{
	int **sock = (int **)data;

	close(*(sock[1]));

	return EXIT_SUCCESS;
}
