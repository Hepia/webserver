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
#include <sys/un.h>

#include "include/log_process.h"
#include "include/histo.h"
#include "include/process_management.h"
#include "include/server_const.h"


// variable temporaire
extern int  taille_log;

// <log=149>
// <url=26>127.0.0.1:9999/favicon.ico</url>
// <ipcli=9>127.0.0.1</ipcli>
// <date=41>[localtime = 10/05/14 - 18:07:05 - (Eté)]</date>
// <staterr=3>404</staterr>
// </log>

int main(int argc, char *argv[])
{
	int sock_afunix;
	struct server_process *s_process_log = NULL;

	// Création d'un flux pour la socket AF_UNIX.
	sock_afunix = create_socket_stream_afunix(AFUNIX_SOCKET_PATH);
	// Initialisation de la structure pour la création du processus
	// log_process et passage en paramètre de la socket AF_UNIX.
	s_process_log = init_server_process(log_process, NULL, (void *)sock_afunix);
	
	// Création du nouveau log_process.
	call_fork(fork(), s_process_log);

	delete_server_process(s_process_log);

	wait();

	return EXIT_SUCCESS;
}

int create_socket_stream_afunix(const char *path)
{
	struct sockaddr_un *local = NULL;
	int sock_afunix;
	int struct_len = 0;

	// Supression de l'adresse de la socket.
	unlink(AFUNIX_SOCKET_PATH);

	// Allocation pour une nouvelle structure sockaddr_un.
	if((local = calloc(1, sizeof(struct sockaddr_un))) == NULL)
	{
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	// Création de la socket AF_UNIX.
	if ((sock_afunix = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Configuration de la structure avec l'adresse de la socket.
	local->sun_family = AF_UNIX;
	strcpy(local->sun_path, path);

	struct_len = strlen(local->sun_path) + sizeof(local->sun_family);

	// Affectation à la nouvelle socket l'adresse située sur le disque.
	if(bind(sock_afunix, (struct sockaddr *)local, struct_len) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	return sock_afunix;
}

int log_process(void *data)
{
	struct queue_hist *logs = NULL;
	struct elem_hist *log_e = NULL;
	//char *log_entry   = "127.0.0.1:9999/favicon.ico$127.0.0.1$404";
	char *champs      = NULL;
	char *ligne;
	char **tab_logs   = NULL;
	int c = 0;
	int sock_afunix = (int)data;
	int new_sock_afunix;
	struct sockaddr_un *remote = NULL;
	socklen_t struct_len = 0;

	char *log_entry;
	int n_recv;

	tab_logs = calloc(3, sizeof(char *));

	if((remote = calloc(1, sizeof(struct sockaddr_un))) == NULL)
	{
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	if((log_entry = calloc(2, sizeof(char))) == NULL)
	{
	 	perror("calloc");
	 	exit(EXIT_FAILURE);
	}

	if((ligne = calloc(256, sizeof(char))) == NULL)
	{
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	tab_logs[0] = NULL;
	tab_logs[1] = NULL;
	tab_logs[2] = NULL;

	// Création d'une nouvelle file de log.
	logs = new_queue(get_size_queue, get_max_size_queue,
                      push, pop,
                      get_elem, get_nb_elem,
                      get_size_elem, taille_log);

	listen(sock_afunix, TAILLE_FILE_ECOUTE);

	// On boucle tantque le serveur n'est pas interrompu.
	while(!close_tcp_server())
	{

		new_sock_afunix = accept(sock_afunix, (struct sockaddr *)remote, &struct_len);

		if(new_sock_afunix < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		else 
		{
			int i = 0;

			while((n_recv = read(new_sock_afunix, log_entry, 1)) > 0)
			{
				//write(1, log_entry, n_recv);
				ligne[i] = log_entry[0];
				i++;
			}
		}

		// Début de l'analyse lexicale.
		champs = strtok(ligne, "$");

		while(champs != NULL)
		{
			// Duplication du champs dans le tableau des éléments.
	 		tab_logs[c] = strdup(champs);
	 		champs = strtok(NULL, "$");
	 		c++;
		}

		// Création d'un nouvel élément à insérer dans la liste de log.
		log_e = create_new_elem_hist(tab_logs[0], tab_logs[1], atoi(tab_logs[2]));
		// On pousse le nouvelle élément dans la file.
		logs->push(logs, (void *)log_e);
		log_e = NULL;

		print_queue(logs);
	}

	delete_queue((void *)logs);

	exit(EXIT_SUCCESS);
}