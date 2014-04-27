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

#include "include/sig_handler.h"
#include "include/histo.h"

struct sigaction *list_action;

// structure contenant les logs.
extern struct queue_hist *q_log;

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

			// Suppression de la file de log de l'espace d'adressage du processus.
			if(q_log != NULL)
				delete_queue(q_log);
			delete_handler(list_action);
			
			// On termine les processus proprement.
			exit(EXIT_SUCCESS);

		default :
			break;
	}
}
