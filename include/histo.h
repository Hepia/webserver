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

#ifndef _HISTO_H_
#define _HISTO_H_

struct elem_hist
{
	char             *q_url;
	char             *q_ipcli;
	char		     *q_date;
	int               q_staterr;
	struct elem_hist *q_next;
};

struct queue_hist
{
	int  nb_elem;
	long size_queue;
	long max_size_queue;
	
	void *first_elem;
	void *last_elem;
	
	long   (*get_size_queue)	(void *);
	long   (*get_max_size_queue)(void *);
	int    (*push)          	(void *, void *);
	int    (*pop)           	(void *);
	void * (*get_elem)      	(void *, int);
	int    (*get_nb_elem)   	(void *);
	long   (*get_size_elem) 	(void *);
};

void * create_new_elem_hist	(char *url, char *ipcli,
							 int staterr);
void   delete_elem_hist		(void *q_elem);

void * new_queue    		(long (*get_size_queue)(void *),
							 long (*get_max_size_queue)(void *),
							 int (*push)(void *, void *),
							 int (*pop)(void *),
							 void * (*get_elem)(void *, int),
							 int (*get_nb_elem)(void *),
							 long (*get_size_elem) (void *),
							 long max_size_queue);

void   delete_queue  		(void *q_this);

long   get_size_queue		(void *q_this);
long   get_max_size_queue	(void *q_this);
int    push          		(void *q_this, void *q_elem);
int    pop           		(void *q_this);
void * get_elem      		(void *q_this, int index);
int    get_nb_elem   		(void *q_this);
long   get_size_elem		(void *q_elem);

void   print_queue			(void *q_this);
void   print_elem			(void *q_elem);

int    rand_a_b	  			(int a, int b);
int    get_file_name 		(char **buffer);

#endif