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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <alloca.h>

#include <sys/types.h>


#include "include/http.h"
#include "include/server_const.h"
#include "include/local_time.h"
#include "include/histo.h"

extern char *chemin_fichiers;

// structure contenant les logs.
extern struct queue_hist *q_log;

/**
 * 
 */
int processHttp(int sockfd, char *ipcli) {

	int              rHttpCode, keepAlive;
	struct elem_hist *elemHist = NULL;
	stuHttpData      *httpData = (stuHttpData *) calloc(1,sizeof(stuHttpData));

	httpData->socketfd = sockfd;
	httpData->q_ipcli  = ipcli;

	readQueryHeader(httpData);
	parseHeader(httpData);

	if ((rHttpCode = fileInfo(httpData)) == 404) {
	// Si le fichier n'existe pas, création du header correspondant et
	// et envoi le fichier 404

		httpData->q_filepath = FILE_404;

		if (fileInfo(httpData) != 200) {
			fprintf(stderr, "File 404 not found\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (rHttpCode == 403) {
		httpData->q_filepath = FILE_403;
		
		if (fileInfo(httpData) != 200) {
			fprintf(stderr, "File 403 not found\n");
			exit(EXIT_FAILURE);
		}
	}

	// Création du header de réponse
	buildHeader(httpData);

	char *fullUri = (char *) alloca((1 + sizeof(httpData->q_host) + sizeof(httpData->q_filename)) * sizeof(char));
	sprintf(fullUri, "%s/%s", httpData->q_host, httpData->q_filename);

	elemHist = (struct elem_hist *) create_new_elem_hist(fullUri, httpData->q_ipcli, rHttpCode);

	// Ajoute d'une nouvelle entrée dans la file de logs.
	q_log->push((void *)q_log, (void *)elemHist);
	print_queue((void *)q_log);

	// Envoi du fichier
	sendFile(httpData);
	keepAlive = httpData->q_keep_alive;

	free(httpData);

	return keepAlive;
}

/**
 * La fonction readQueryHeader lit les premiers octets reçu par le client afin
 * d'extraire l'en-tête de la requête, qui est retournée lors de la sortie de la fonction.
 */
void* readQueryHeader(stuHttpData *httpData) {

	httpData->q_header = (char *) calloc(TAILLE_REQUETE_MAX, sizeof(char));
	char *endHeader = NULL;
	char *btmp;
	int nb;
	int nb_read = 0;

	// Lecture du flux entrant pour récupérer le header de la requête
	do
	{
		btmp = (char *) calloc(TAILLE_READ_BUFFER, sizeof(char));

		if ((nb = read(httpData->socketfd, btmp,TAILLE_READ_BUFFER)) < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if (nb_read >= TAILLE_REQUETE_MAX)
			break;
			// TODO
			// respond error 413 Request Entity Too Large
			// http://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html#sec6.1

		strcat(httpData->q_header, btmp);
		endHeader = strstr(httpData->q_header, CRLF CRLF);
		free(btmp);

	} while (endHeader == NULL);

	httpData->q_header = calloc(strlen(bHeaderTmp) + 1, sizeof(char));
	strcpy(httpData->q_header, bHeaderTmp);

	return 0;
}

/**
 * La fonction parseHeader analyse l'en-tête de la requete reçu en paramètre
 * et effectue l'extraction du nom du fichier demandé qui est retourné à la sortie
 */
void* parseHeader(stuHttpData *httpData) {

<<<<<<< HEAD
	char *token;
	char *rUri 		= calloc(TAILLE_READ_BUFFER, sizeof(char));
	httpData->q_keep_alive = 0;
	httpData->q_method     = calloc(16, sizeof(char));
	httpData->q_host       = calloc(TAILLE_READ_BUFFER, sizeof(char));

	sscanf(httpData->q_header, "%s %s", httpData->q_method, rUri);

	// Extraction des éléments nécessaire dans l'en-tête
	token = strstr(httpData->q_header, "Host:");
	sscanf(token, "%*s %s", httpData->q_host);

	if (strstr(httpData->q_header, "Connection: keep-alive") != NULL) {
		httpData->q_keep_alive = 1;
	}

	// Si l'URL est la racine "/" il faut spécifier le fichier index par défaut
	// Sinon récupérer le URL du fichier sans le premier "/"
	if (strcmp(rUri, "/") == 0)
		httpData->q_filename = FILE_INDEX;
	else {
		httpData->q_filename = (char *) calloc(strlen(rUri)-1, sizeof(char));
		strcpy(httpData->q_filename, rUri+1);
	}

	// Création du chemin complet du fichier
	httpData->q_filepath = (char *) calloc(strlen(chemin_fichiers) + strlen(httpData->q_filename), sizeof(char));
	sprintf(httpData->q_filepath, "%s%s", chemin_fichiers, httpData->q_filename);

	free(rMethod);
	free(rUri);
	free(rHost);

	return 0;
}

/**
 * La fonction sendFile vérifie l'existance du fichier et envoi se dernier au client via la socket ouverte.
 * Si le fichier n'existe pas, l'erreur 404 est envoyée.
 */
void* sendFile(stuHttpData *httpData) {

	int  fdFile, rd;
	char bufFile[TAILLE_READ_BUFFER];

	// Envoi du header
	if (write(httpData->socketfd, httpData->r_header, httpData->r_header_size) < 0) {
		perror("Write Header");

		exit(EXIT_FAILURE);
	}

	// Ouverture du fichier demandé
	if ((fdFile = open(httpData->q_filepath, O_RDONLY)) < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	// Lecture et envoi du fichier demandé
	do {
		if ((rd = read(fdFile, bufFile, TAILLE_READ_BUFFER)) < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if ((write(httpData->socketfd, bufFile, rd)) < 0) {
			perror("Write File");
			exit(EXIT_FAILURE);
		}
	} while (rd != 0);

	// Fermeture du descripteur du fichier
	close(fdFile);

	return 0;
}

/**
 * Récupération des informations nécessaires et création de l'en-tête
 * pour la réponse du serveur.
 */
void* buildHeader(stuHttpData *httpData) {

	// TODO Date
	//httpData->r_date = Date

	// Status
	if		(httpData->r_code == 200)	httpData->r_status = "OK";
	else if	(httpData->r_code == 403)	httpData->r_status = "Forbidden";
	else if	(httpData->r_code == 404)	httpData->r_status = "Not found";

	// Création du header sous forme d'une chaîne
	char *tmp_str = (char *) alloca(TAILLE_READ_BUFFER * sizeof(char));
	sprintf(tmp_str, "HTTP/%s %d %s\n"
					 "Server: %s\n"
					 "Content-Length: %d\n\n", 
					 HTTP_VERSION, httpData->r_code, httpData->r_status, 
					 SERVER_INFO, httpData->r_content_length);

	httpData->r_header_size = strlen(tmp_str);
	httpData->r_header = (char *) calloc(httpData->r_header_size + 1, sizeof(char));
	strcpy(httpData->r_header, tmp_str);

	return 0;
}


/**
 * La fonction fileInfo returne le code HTTP selon la disponnibilité ou non
 * du fichier demandé en paramètre. Aussi elle permet de tester si ce fichier existe, 
 * et dans un second temps permettra de vérifier les droits d'accès au fichier.
 */
int fileInfo(stuHttpData *httpData) {

	struct	stat fileStat;
	httpData->r_content_length = 0;

	if(stat(httpData->q_filepath,&fileStat) < 0) {

		fprintf(stderr, "file %s not found\n", httpData->q_filepath);
		httpData->r_code = 404;
		return 404;
	}
	else {
		/* TODO
		 * Tester si le processus à les droits suffisant pour lire/executer le fichier
		 *
		 * fileStat.st_mode & S_IXUSR) ? "x" : "-"
		 * fileStat.st_mode & S_IRGRP) ? "r" : "-"
		 * fileStat.st_mode & S_IWGRP) ? "w" : "-"
		 * fileStat.st_mode & S_IXGRP) ? "x" : "-"
		 * fileStat.st_mode & S_IWOTH) ? "w" : "-"
		 * fileStat.st_mode & S_IXOTH) ? "x" : "-"
		 *
		 * return 403;
		 */

		httpData->r_content_length = fileStat.st_size;
		httpData->r_code = 200;
		return 200;
	}
}
