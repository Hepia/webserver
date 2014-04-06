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

struct sigaction *list_action;

void init_handler(struct sigaction *list_action)
{
	list_action = calloc(1, sizeof(struct sigaction));

	list_action->sa_handler = handler;
	sigemptyset(&(list_action->sa_mask));
	list_action->sa_flags = 0;

	if(sigaction(SIGINT, list_action, NULL) != 0)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}

void delete_handler(struct sigaction *list_action)
{
	free(list_action);
}

void handler(int num)
{
	switch(num)
	{
		case SIGINT :
			fprintf(stdout, "Processus %d : signal SIGINT\nInterruption du processus\n", getpid());
			delete_handler(list_action);
			exit(EXIT_SUCCESS);

		default :
			break;
	}
}