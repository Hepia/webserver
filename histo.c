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
#include <string.h>
#include <time.h>
#include <assert.h>

#include "include/histo.h"
#include "include/local_time.h"

void * create_new_elem_hist(char *url, char *ipcli,
							int staterr)
{
	struct elem_hist *elem = NULL;

	if((elem = calloc(1, sizeof(struct elem_hist))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	if((elem->q_url = calloc(strlen(url) + 1, sizeof(char))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	strcpy(elem->q_url, url);

	if((elem->q_ipcli = calloc(strlen(ipcli) + 1, sizeof(char))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	strcpy(elem->q_ipcli, ipcli);

	elem->q_staterr = staterr;

	if(get_local_time(&(elem->q_date), 0) == -1)
	{
		fprintf(stderr, "L'appel à get_local_time a échoué.\n");
		return NULL;
	}

	elem->q_next = NULL;

	return (void *)elem;
}

void delete_elem_hist(void *q_elem)
{
	struct elem_hist *tmp_elem = (struct elem_hist *)q_elem;

	free(tmp_elem->q_url);
	free(tmp_elem->q_ipcli);
	free(tmp_elem->q_date);

	free(q_elem);
	tmp_elem->q_next = NULL;
	tmp_elem = NULL;
}

void * new_queue(long (*get_size_queue)(void *),
				 long (*get_max_size_queue)(void *),
				 int (*push)(void *, void *),
				 int (*pop)(void *),
				 void * (*get_elem)(void *, int),
				 int (*get_nb_elem)(void *),
				 long (*get_size_elem) (void *),
				 long max_size_queue)
{
	struct queue_hist *queue = NULL;
	
	if((queue = calloc(1, sizeof(struct queue_hist))) == NULL)
	{
		perror("calloc");
		return NULL;
	}
	
	queue->nb_elem        = 0;
	queue->size_queue     = 0;
	queue->max_size_queue = max_size_queue;
	queue->first_elem     = NULL;
	queue->last_elem      = NULL;
	
	queue->get_size_queue 	  = get_size_queue;
	queue->get_max_size_queue = get_max_size_queue;
	queue->push           	  = push;
	queue->pop            	  = pop;
	queue->get_elem       	  = get_elem;
	queue->get_nb_elem    	  = get_nb_elem;
	queue->get_size_elem  	  = get_size_elem;
	
	return (void *)queue;
}

void delete_queue(void *q_this)
{
	for(int i = 0; i < ((struct queue_hist *)q_this)->nb_elem; i++)
	{
		delete_elem_hist(((struct queue_hist *)q_this)->get_elem(q_this, 0));
	}

	((struct queue_hist *)q_this)->first_elem = NULL;
	((struct queue_hist *)q_this)->last_elem= NULL;

	free(q_this);
}

long get_size_queue(void *q_this)
{
	if(q_this != NULL)
		return ((struct queue_hist *)q_this)->size_queue;
	return -1;
}

long get_max_size_queue(void *q_this)
{
	if(q_this != NULL)
		return ((struct queue_hist *)q_this)->max_size_queue;
	return -1;
}

int push(void *q_this, void *q_elem)
{
	int size_elem = 0;
	void *q_elem_tmp = NULL;

	if((q_this != NULL) && (q_elem != NULL))
	{
		size_elem = get_size_elem(q_elem);

		// Si la somme de la taille totale de la file et du nouvel élément est
		// supérieur à la taille maximum authorisée, alors le plus ancien
		// élément.

		while((((struct queue_hist *)q_this)->get_size_queue(q_this) + size_elem) > 
			((struct queue_hist *)q_this)->get_max_size_queue(q_this))
		{
			fprintf(stdout, "\n\nTaille trop petite\n\n");
			((struct queue_hist *)q_this)->pop(q_this);
		}

		// Ensuite, on ajoute le nouvel élément en queue de file.

		((struct queue_hist *)q_this)->nb_elem++;
		((struct queue_hist *)q_this)->size_queue += size_elem;

		// La file est vide.
		if((((struct queue_hist *)q_this)->first_elem == NULL) && 
			   (((struct queue_hist *)q_this)->last_elem == NULL))
		{
			((struct queue_hist *)q_this)->first_elem = q_elem;
			((struct queue_hist *)q_this)->last_elem = q_elem;
		}
		// La file n'est pas vide.
		else
		{
			q_elem_tmp = ((struct queue_hist *)q_this)->last_elem;
			((struct queue_hist *)q_this)->last_elem = q_elem;
			((struct elem_hist *)q_elem)->q_next = q_elem_tmp;
		}

		return EXIT_SUCCESS;	
	}

	return -1;
}

int pop(void *q_this)
{
	int size_first_elem = 0;
	void *q_elem_tmp = NULL;
	void *q_elem_last_tmp = NULL;

	if(q_this != NULL)
	{
		size_first_elem = ((struct queue_hist *)q_this)->get_size_elem(((struct queue_hist *)q_this)->first_elem);

		if(((struct queue_hist *)q_this)->nb_elem < 1)
		{
			return -1;
		}
		else if(((struct queue_hist *)q_this)->nb_elem == 1)
		{
			q_elem_tmp = ((struct queue_hist *)q_this)->first_elem;
			((struct queue_hist *)q_this)->first_elem = NULL;
			((struct queue_hist *)q_this)->last_elem = NULL;	
		}
		else if(((struct queue_hist *)q_this)->nb_elem > 1)
		{
			q_elem_last_tmp = ((struct queue_hist *)q_this)->last_elem;
			q_elem_tmp = ((struct queue_hist *)q_this)->first_elem;

			for(int i = 0; i < (((struct queue_hist *)q_this)->nb_elem - 2); i++)
			{
				q_elem_last_tmp = ((struct elem_hist *)q_elem_last_tmp)->q_next;
			}

			((struct elem_hist *)q_elem_last_tmp)->q_next = NULL;
			((struct queue_hist *)q_this)->first_elem = q_elem_last_tmp;
		}

		((struct queue_hist *)q_this)->nb_elem--;
		((struct queue_hist *)q_this)->size_queue -= size_first_elem;

		delete_elem_hist(q_elem_tmp);

		return EXIT_SUCCESS;
	}

	return -1;
}

void * get_elem(void *q_this, int index)
{
	struct queue_hist *q = (struct queue_hist *)q_this;
	struct elem_hist *e = (struct elem_hist *)(q->last_elem);

	for(int i = 0; i < index; i++)
	{
		e = (struct elem_hist *)(((struct elem_hist *)e)->q_next);
	}

	return (void *)e;
}

int get_nb_elem(void *q_this)
{
	if(q_this != NULL)
		return ((struct queue_hist *)q_this)->nb_elem;
	return -1;
}

long get_size_elem(void *q_elem)
{
	long size_elem = 0;

	if(q_elem != NULL)
	{
		// On compte le nombre de caractère dans l'url.
		if(((struct elem_hist *)q_elem)->q_url != NULL)
		{
			for(int i = 0; ((struct elem_hist *)q_elem)->q_url[i] != 0; i++)
				size_elem += sizeof(char);
			size_elem += sizeof(char);
		}

		// On compte le nombre de caractère dans l'adresse IP.
		if(((struct elem_hist *)q_elem)->q_ipcli != NULL)
		{
			for(int i = 0; ((struct elem_hist *)q_elem)->q_ipcli[i] != 0; i++)
				size_elem += sizeof(char);
			size_elem += sizeof(char);
		}

		// On compte le nombre de caractère dans la date.
		if(((struct elem_hist *)q_elem)->q_date != NULL)
		{
			for(int i = 0; ((struct elem_hist *)q_elem)->q_date[i] != 0; i++)
				size_elem += sizeof(char);
			size_elem += sizeof(char);
		}

		// On ajoute la taille d'un entier pour contenir l'erreur.
		size_elem += sizeof(int);

		size_elem += sizeof(void *);

		return size_elem;
	}

	return -1;
}

void print_queue(void *q_this)
{
	struct queue_hist *q = (struct queue_hist *)q_this;
	struct elem_hist *e = (struct elem_hist *)(q->last_elem);

	fprintf(stdout, "\nprint queue\n-----------\n");

	for(int i = 0; i < q->get_nb_elem(q_this); i++)
	{
		print_elem((void *)e);
		e = (struct elem_hist *)(((struct elem_hist *)e)->q_next);
	}
}

void print_elem(void *q_elem)
{
	struct elem_hist *e = (struct elem_hist *)q_elem;

	fprintf(stdout,"@@@@@\nsizeof() = %ld [o]\n", get_size_elem(q_elem));
	fprintf(stdout, "+++++\nadr : %p\n+++++\nurl : %s\nipcli : %s\ndate : %s\nstaterr : %d\nq_next : %p\n@@@@@\n", 
			(void *)e, e->q_url, e->q_ipcli, e->q_date, e->q_staterr, (void *)(e->q_next));
}

int rand_x_y(int x, int y)
{
    return (rand() % (y - x) + x);
}

int get_file_name(char **buffer)
{
	char *buffer_tmp = NULL;
	char srval[5] = {0, 0, 0, 0, 0};
	int  rval = 0;
	int size = 0;

	srand(time(NULL));

	rval = rand_x_y(1000, 9999);

	sprintf(srval, "%d", rval);

	size = get_local_time(&buffer_tmp, 0);

	assert(*buffer == NULL);

	if((*buffer = calloc(size + 1 + 4, sizeof(char))) == NULL)
	{
		perror("calloc");
		return -1;
	} 

	strcpy(*buffer, buffer_tmp);
	strcat(*buffer, srval);

	return (size + 4);
}

// int main(void)
// {
// 	struct queue_hist *queue = NULL;

// 	struct elem_hist *e1 = NULL;
// 	struct elem_hist *e2 = NULL;
// 	struct elem_hist *e3 = NULL;
// 	struct elem_hist *e4 = NULL;
// 	struct elem_hist *e5 = NULL;

// 	queue = new_queue(get_size_queue, get_max_size_queue,
// 					  push, pop,
// 					  get_elem, get_nb_elem,
// 					  get_size_elem, 330);

// 	e1 = create_new_elem_hist("http://localhost1/index.html", "127.0.0.1", 1);
// 	e2 = create_new_elem_hist("http://localhost2", "127.0.0.2", 2);
// 	e3 = create_new_elem_hist("http://localhost3/t.txt", "127.0.0.3", 3);
// 	e4 = create_new_elem_hist("http://www.google.com", "192.168.0.100", 4);
// 	e5 = create_new_elem_hist("http://www.hepia.ch", "192.168.0.200", 5);

// 	fprintf(stdout, "e1 + e2 + e3 + e4 + e5= %ld\n\n\n", get_size_elem((void *)e1) + 
// 														 get_size_elem((void *)e2) + 
// 													 	 get_size_elem((void *)e3) +
// 													 	 get_size_elem((void *)e4) +
// 													 	 get_size_elem((void *)e4));

// 	queue->push((void *)queue, (void *)e1);
// 	queue->push((void *)queue, (void *)e2);
// 	queue->push((void *)queue, (void *)e3);
// 	queue->push((void *)queue, (void *)e4);
// 	queue->push((void *)queue, (void *)e5);

// 	print_queue((void *)queue);

// 	fprintf(stdout, "\n\nelem index %d\n", 2);
// 	print_elem(get_elem((void *)queue, 2));
// 	fprintf(stdout, "\n\n");

// 	queue->pop((void *)queue);
// 	queue->pop((void *)queue);
// 	queue->pop((void *)queue);
// 	queue->pop((void *)queue);

// 	queue->pop((void *)queue);
// 	queue->pop((void *)queue);
// 	queue->pop((void *)queue);

// 	print_queue((void *)queue);

// 	delete_queue(queue);

// 	return EXIT_SUCCESS;
// }