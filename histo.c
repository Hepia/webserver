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
#include "include/log_process.h"
#include "include/server_const.h"

// Références externe au fichier de logs et au chemin d'accès du fichier de logs.

extern FILE *fp_log;
extern char *log_path;
extern char *log_file_name;

/*
 * La fonction create_new_elem_hist crée un nouvel élément et remplis les champs de la
 * structure correspondante.
 */

void * create_new_elem_hist(char *url, char *ipcli,
							int staterr)
{
	struct elem_hist *elem = NULL;

	if((elem = calloc(1, sizeof(struct elem_hist))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	// Création et initialisation du champs q_url.
	if((elem->q_url = calloc(strlen(url) + 1, sizeof(char))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	strcpy(elem->q_url, url);

	// Création et initialisation du champs q_ipcli.
	if((elem->q_ipcli = calloc(strlen(ipcli) + 1, sizeof(char))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	strcpy(elem->q_ipcli, ipcli);

	// Initialisation du champs q_staterr.
	elem->q_staterr = staterr;

	if(get_local_time(&(elem->q_date), 0) == -1)
	{
		fprintf(stderr, "L'appel à get_local_time a échoué.\n");
		return NULL;
	}

	// Initialisation du champs q_next.
	elem->q_next = NULL;

	return (void *)elem;
}

/*
 * La fonction delete_elem_hist permet de supprimer un élément crée dynamiquement.
 */
 
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

/*
 * La fonction new_queue permet de créer dynamiquement et initialiser une nouvelle
 * structure de type queue_hist. Celle-ci prend en paramètre des pointeurs sur
 * les fonctions permettant la manipulation de la file (FIFO).
 */

void * new_queue(long (*get_size_queue)(void *),
				 long (*get_max_size_queue)(void *),
				 int (*push)(void *, void *),
				 void * (*pop)(void *),
				 void * (*get_elem)(void *, int),
				 int (*get_nb_elem)(void *),
				 long (*get_size_elem) (void *),
				 long max_size_queue)
{
	struct queue_hist *queue = NULL;
	int size_file_name = 0;
	
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

	// Création du nom du fichier de log.
	size_file_name = get_file_name(&log_file_name);

	if((log_path = calloc(size_file_name + 1 + strlen(LOG_FOLDER), sizeof(char))) == NULL)
	{
		perror("calloc");
		return NULL;
	}

	// Cération du chemin d'accès relatif du fichier de log.
	strcpy(log_path, LOG_FOLDER);
	strcat(log_path, log_file_name);

	// Ouverture du fichier de log.
	fprintf(stdout, "Le fichier de log \"%s\" a été crée.\n", log_path);
	fp_log = my_fopen(log_path, "aw+", 1);
	
	return (void *)queue;
}

/*
 * La fonction delete_queue permet de supprimer une structure de type queue_hist.
 */

void delete_queue(void *q_this)
{
	int nb_elem = ((struct queue_hist *)q_this)->nb_elem;
	struct elem_hist *q_elem_pop = NULL;
	char buffer_log[1024];

	// On boucle en utilisant la fonction pop pour retirer les éléments
	// et les supprimer.

	fprintf(stdout, "Sauvegarde des logs sur le disque.\n");

	// Boucle de supression des éléments de la file.
	for(int i = 0; i < nb_elem; i++)
	{
		// On sort l'élément.
		q_elem_pop = (struct elem_hist *)(((struct queue_hist *)q_this)->pop(q_this));

		// On formate le contenu de l'élément sorti dans un buffer.
		sprintf(buffer_log, "%s - %s - %s - %d\n", 
				q_elem_pop->q_url,
				q_elem_pop->q_ipcli,
				q_elem_pop->q_date,
				q_elem_pop->q_staterr);

		// On sauvegarde le buffer dans le fichier de logs.
		if((fwrite(buffer_log, sizeof(char), strlen(buffer_log), fp_log)) != strlen(buffer_log))
		{
			perror("fwrite");
			exit(EXIT_FAILURE);
		}

		// On supprime définitivement l'élément sorti.
		delete_elem_hist((void *)q_elem_pop);
	}

	((struct queue_hist *)q_this)->first_elem = NULL;
	((struct queue_hist *)q_this)->last_elem  = NULL;

	free(q_this);
}

/*
 * La fonction get_size_queue retourne la taille courante de la file (FIFO).
 */

long get_size_queue(void *q_this)
{
	if(q_this != NULL)
		return ((struct queue_hist *)q_this)->size_queue;
	return -1;
}

/*
 * La fonction get_max_size_queue retourne la taille maximum de la file (FIFO).
 */

long get_max_size_queue(void *q_this)
{
	if(q_this != NULL)
		return ((struct queue_hist *)q_this)->max_size_queue;
	return -1;
}

/*
 * La fonction push permet de pousser un nouvel élément dans la file (FIFO). Celui-ci
 * se trouvera en dernière position.
 */

int push(void *q_this, void *q_elem)
{
	int size_elem = 0;
	void *q_elem_tmp = NULL;
	struct elem_hist *q_elem_pop = NULL;
	char buffer_log[1024];

	if((q_this != NULL) && (q_elem != NULL))
	{
		size_elem = get_size_elem(q_elem);

		// Si la somme de la taille totale de la file et du nouvel élément est
		// supérieur à la taille maximum authorisée, alors le plus ancien
		// élément.

		while((((struct queue_hist *)q_this)->get_size_queue(q_this) + size_elem) > 
			((struct queue_hist *)q_this)->get_max_size_queue(q_this))
		{
			if(((struct queue_hist *)q_this)->get_nb_elem(q_this) == 0)
				return -1;

			// On sort l'élément.
			q_elem_pop = (struct elem_hist *)(((struct queue_hist *)q_this)->pop(q_this));

			// On formate le contenu de l'élément sorti dans un buffer.
			sprintf(buffer_log, "%s - %s - %s - %d\n", 
					q_elem_pop->q_url,
					q_elem_pop->q_ipcli,
					q_elem_pop->q_date,
					q_elem_pop->q_staterr);

			// On sauvegarde le buffer dans le fichier de logs.
			if((fwrite(buffer_log, sizeof(char), strlen(buffer_log), fp_log)) != strlen(buffer_log))
			{
				perror("fwrite");
				exit(EXIT_FAILURE);
			}

			delete_elem_hist((void *)q_elem_pop);
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

/*
 * La fonction pop permet de sortir l'élément en tête de file (FIFO). Celui-ci sera
 * détruit.
 */

void* pop(void *q_this)
{
	int size_first_elem = 0;
	void *q_elem_tmp = NULL;
	void *q_elem_last_tmp = NULL;

	if(q_this != NULL)
	{
		size_first_elem = ((struct queue_hist *)q_this)->get_size_elem(((struct queue_hist *)q_this)->first_elem);

		// Cette fonction n'est pas utilisable si la file (FIFO) ne contient aucun élément.
		if(((struct queue_hist *)q_this)->nb_elem < 1)
		{
			return NULL;
		}
		// Suppression si il y a un seul élément dans la file (FIFO).
		else if(((struct queue_hist *)q_this)->nb_elem == 1)
		{
			q_elem_tmp = ((struct queue_hist *)q_this)->first_elem;
			((struct queue_hist *)q_this)->first_elem = NULL;
			((struct queue_hist *)q_this)->last_elem = NULL;	
		}
		// Lorsqu'il y a plusieurs élément, suppression de l'élément en tête de file (FIFO).
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

		// Mise à jour du nombre d'élément contenu dans la file (FIFO) et de la taille en
		// octet de la file (FIFO).
		((struct queue_hist *)q_this)->nb_elem--;
		((struct queue_hist *)q_this)->size_queue -= size_first_elem;

		//delete_elem_hist(q_elem_tmp);

		return q_elem_tmp;
	}

	return NULL;
}

/*
 * La fonction get_elem permet de retourner une référence sur l'élément situé à la position
 * index dans la file. Cette fonction part du dernier élément en direction du premier élément.
 */

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

/*
 * La fonction get_nb_elem renvoie le nombre d'éléments contenu dans la file (FIFO).
 */

int get_nb_elem(void *q_this)
{
	if(q_this != NULL)
		return ((struct queue_hist *)q_this)->nb_elem;
	return -1;
}

/*
 * La fonction get_size_elem retourne la taille d'un élément en octets.
 */

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

/*
 * La fonction print_queue affiche la file (FIFO) sur la sortie standard.
 */

void print_queue(void *q_this)
{
	struct queue_hist *q = (struct queue_hist *)q_this;
	struct elem_hist *e = (struct elem_hist *)(q->last_elem);

	fprintf(stdout, "\nprint queue @ = %p\n-----------\n", q_this);

	for(int i = 0; i < q->get_nb_elem(q_this); i++)
	{
		print_elem((void *)e);
		e = (struct elem_hist *)(((struct elem_hist *)e)->q_next);
	}
}

/*
 * La fonction print_elem affiche un élément sur la sortie standard.
 */

void print_elem(void *q_elem)
{
	struct elem_hist *e = (struct elem_hist *)q_elem;

	fprintf(stdout,"@@@@@\nsizeof() = %ld [o]\n", get_size_elem(q_elem));
	fprintf(stdout, "+++++\nadr : %p\n+++++\nurl : %s\nipcli : %s\ndate : %s\nstaterr : %d\nq_next : %p\n@@@@@\n", 
			(void *)e, e->q_url, e->q_ipcli, e->q_date, e->q_staterr, (void *)(e->q_next));
}

/*
 * La fonction rand_x_y retourne un nombre pseudo-aléatoire borné entre x et y, bornes incluses.
 */

int rand_x_y(int x, int y)
{
    return (rand() % (y - x) + x);
}

/*
 * La fonction get_file_name retourne un nom de fichier sous la forme d'une chaîne de caractères.
 * Celle-ci est composée de la date et de l'heure courante, ainsi que d'une valeur pseudo-aléatoire
 * comprise entre 1000 et 9999.
 */

int get_file_name(char **buffer)
{
	char *buffer_tmp = NULL;
	char srval[5] = {0, 0, 0, 0, 0};
	int  rval = 0;
	int size = 0;

	srand(time(NULL));

	rval = rand_x_y(1000, 9999);

	sprintf(srval, "%d", rval);

	size = get_local_time(&buffer_tmp, 1);

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