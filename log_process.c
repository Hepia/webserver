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
#include <fcntl.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "include/log_process.h"
#include "include/histo.h"
#include "include/process_management.h"
#include "include/server_const.h"
#include "include/socket.h"
#include "include/sig_handler.h"


// Variable contenant la taille maximum des logs.
extern int taille_log;

struct queue_hist *logs = NULL;
FILE *fp_log            = NULL;
char *log_path          = NULL;
char *log_file_name     = NULL;

struct sigaction *list_action_log;

// <log=149>
// <url=26>127.0.0.1:9999/favicon.ico</url>
// <ipcli=9>127.0.0.1</ipcli>
// <date=41>[localtime = 10/05/14 - 18:07:05 - (Eté)]</date>
// <staterr=3>404</staterr>
// </log>

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
	struct elem_hist *log_e = NULL;
	struct sockaddr_un *remote = NULL;
	socklen_t struct_len = 0;

	char *log_entry;
	char *champs      = NULL;
	char *ligne;
	char **tab_logs   = NULL;
	int c = 0;
	int sock_afunix = (int)data;
	int new_sock_afunix;
	int n_recv;
	
	fprintf(stdout, "Lancement du processus [%d] de gestion des logs.\t[OK]\n", getpid());


	// Allocation dynamique.
	list_action_log = calloc(1, sizeof(struct sigaction));

	// Liaison de la structure avec la fonction handler.
	list_action_log->sa_handler = handler_log;
	sigemptyset(&(list_action_log->sa_mask));
	// Paramètrage du comportement des signaux.
	list_action_log->sa_flags = 0;
	list_action_log->sa_flags = SA_RESTART;

	// liaison du signal SIGINT avec la structure sigaction.
	if(sigaction(SIGUSR1, list_action_log, NULL) != 0)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	// liaison du signal SIGINT avec la structure sigaction.
	if(sigaction(SIGINT, list_action_log, NULL) != 0)
	{
	 	perror("sigaction");
	 	exit(EXIT_FAILURE);
	}

	free(list_action_log);

	tab_logs = calloc(3, sizeof(char *));

	if((remote = calloc(1, sizeof(struct sockaddr_un))) == NULL)
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

		close(new_sock_afunix);
		free(log_entry);
		free(ligne);
		free(tab_logs[0]);
		free(tab_logs[1]);
		free(tab_logs[2]);
		champs = NULL;
		tab_logs[0] = NULL;
		tab_logs[1] = NULL;
		tab_logs[2] = NULL;
		c = 0;
		print_queue(logs);
	}

	delete_queue((void *)logs);

	// Supression de l'adresse de la socket.
	unlink(AFUNIX_SOCKET_PATH);

	fprintf(stdout, "Fin du processus de gestion des logs.\t[OK]\n");

	exit(EXIT_SUCCESS);
}

FILE * x_fopen(const char *file_name, const char *flow_mode)
{
	int b_read   = 0;
	int b_write  = 0;
	int b_append = 0;
	int b_creat  = 0;
	int b_trunc  = 0;
	int flags    = 0;
	int len_flow_mode = strlen(flow_mode);

	int fd;
	FILE *fp;

	for(int i = 0; i < len_flow_mode; i++)
		switch(flow_mode[i])
		{
			case 'a' :
				b_write  = 1;
				b_read   = 1;
				b_append = 1;
				break;

			case 'r' :
				b_read = 1;
				break;

			case 'w' :
				b_write = 1;
				b_creat = 1;
				b_trunc = 1;
				break;

			case '+' :
				b_write = 1;
				b_read  = 1;
				break;

			default :
				break;
		}

	if(b_read & b_write)
		flags = O_RDWR;
	else if(b_read)
		flags = O_RDONLY;
	else if(b_write)
		flags = O_WRONLY;
	else
	{
		errno = EINVAL;
		return NULL;
	}

	if(b_creat)
		flags |= O_CREAT;
	if(b_trunc)
		flags |= O_TRUNC;

	flags |= O_EXCL;

	if((fd = open(file_name, flags, 0777)) < 0)
	{
		perror("open");
		return NULL;
	}

	if((fp = fdopen(fd, flow_mode)) == NULL)
	{
		perror("fdopen");
		return NULL;
	}

	return fp;
}

FILE * my_fopen(const char *file_name, const char *mode, int excl)
{
	FILE *fp = NULL;

	fprintf(stdout, "Ouverture %s de %s, mode %s\n", (excl ? "exclusif" : ""), file_name, mode);
	fflush(stdout);

	if(excl)
		fp = x_fopen(file_name, mode);
	else
		fp = fopen(file_name, mode);

	if(fp == NULL)
	{
		perror("fopen");
		return NULL;
	}

	return fp;
}