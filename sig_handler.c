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
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "include/sig_handler.h"
#include "include/histo.h"
#include "include/server_const.h"

extern FILE *fp_log;
extern struct queue_hist *logs;

struct sigaction *list_action;

/*
 * La fonction init_handler alloue dynamiquement de la mémoire pour une structure
 * sigaction, paramètre cette structure et défini les signaux UNIX à traiter.
 */

void init_handler(struct sigaction *list_action)
{
	// Allocation dynamique.
	list_action = calloc(1, sizeof(struct sigaction));

	// Liaison de la structure avec la fonction handler.
	list_action->sa_handler = handler;
	sigemptyset(&(list_action->sa_mask));
	// Paramètrage du comportement des signaux.
	list_action->sa_flags = 0;

	// liaison du signal SIGINT avec la structure sigaction.
	if(sigaction(SIGINT, list_action, NULL) != 0)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}

/*
 * La fonction delete_handler libère la mémoire allouée pour la structure
 * sigaction.
 */

void delete_handler(struct sigaction *list_action)
{
	free(list_action);
}

/*
 * La fonction handler est le gestionnaire de signaux qui est appelé lorsqu'un
 * signal unix est envoyé au serveur.
 */

void handler(int num)
{
	switch(num)
	{
		case SIGINT : // Capture du signal SIGINT.
			fprintf(stdout, "Processus %d : signal SIGINT\nInterruption du processus\n", getpid());

			//fclose(fp_log);
			//delete_queue((void *)logs);
			// Supression de l'adresse de la socket.
			unlink(AFUNIX_SOCKET_PATH);
			delete_handler(list_action);
			
			// On termine les processus proprement.
			exit(EXIT_SUCCESS);

		default :
			break;
	}
}

void handler_log(int num)
{
	int nb_elem_hist = 0;
	struct elem_hist *q_elem_pop = NULL;
	char buffer_log[1024];

	switch(num)
	{
		case SIGINT : // Capture du signal SIGINT.
			fprintf(stdout, "Processus %d : signal SIGINT\nInterruption du processus\n", getpid());
			fprintf(stdout, "Libération des logs de la mémoire.\nFermeture du fichier de logs.\n");

			delete_queue((void *)logs);

			fclose(fp_log);
			// Supression de l'adresse de la socket.
			unlink(AFUNIX_SOCKET_PATH);
			delete_handler(list_action);
			
			// On termine les processus proprement.
			exit(EXIT_SUCCESS);

		case SIGUSR1 : // Capture du signal SIGUSR1
			nb_elem_hist = logs->get_nb_elem(logs);

			fprintf(stdout, "Le processus log [%d] a reçu le signal SIGUSR1.\n", getpid());

			for(int i = 0; i < nb_elem_hist; i++)
			{
				q_elem_pop = (struct elem_hist *)(logs->pop(logs));

				sprintf(buffer_log, "%s - %s - %s - %d\n", 
						q_elem_pop->q_url,
						q_elem_pop->q_ipcli,
						q_elem_pop->q_date,
						q_elem_pop->q_staterr);

				if((fwrite(buffer_log, sizeof(char), strlen(buffer_log), fp_log)) != strlen(buffer_log))
				{
					perror("fwrite");
					exit(EXIT_FAILURE);
				}	

				delete_elem_hist((void *)q_elem_pop);
			}

			break;

		default :
			break;
	}
}

