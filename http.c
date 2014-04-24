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

#include "include/http.h"
#include "include/server_const.h"

extern char *chemin_fichiers;

/**
 * 
 */
void processHttp(int sockfd) {

	char *requestHeader, *filepath;
	requestHeader = readRequestHeader(sockfd);

	filepath = parseHeader(requestHeader);

	sendFile(sockfd, filepath);
}

/**
 * La fonction readRequestHeader lit les premiers octets reçu par le client afin
 * d'extraire l'en-tête de la requête, qui est retournée lors de la sortie de la fonction.
 */
char* readRequestHeader(int sockfd) {

	char *bHeaderTmp = alloca(TAILLE_REQUETE_MAX * sizeof(char));
	char *bHeader, *btmp;
	char *endHeader = NULL;
	int nb;
	int nb_read = 0;

	// Lecture du flux entrant pour récupérer le header de la requête
	do
	{
		btmp = (char *) calloc(TAILLE_READ_BUFFER, sizeof(char));

		if ((nb = read(sockfd, btmp,TAILLE_READ_BUFFER)) < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if (nb_read >= TAILLE_REQUETE_MAX)
			break;
			// TODO
			// respond error 413 Request Entity Too Large
			// http://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html#sec6.1

		strcat(bHeaderTmp, btmp);
		endHeader = strstr(bHeaderTmp, CRLF CRLF);
		free(btmp);

	} while (endHeader == NULL);
	
	bHeader = alloca(strlen(bHeaderTmp) * sizeof(char));
	strcpy(bHeader, bHeaderTmp);

	return bHeader;
}

/**
 * La fonction parseHeader analyse l'en-tête de la requete reçu en paramètre
 * et effectue l'extraction du nom du fichier demandé qui est retourné à la sortie
 */
char* parseHeader(char *header) {

	int		filepathSize;
	char	*filepath, *filename, *method, *requestUri;

	// Récupération des substring Méthode et URL de requete du header
	method = strtok(header, " ");
	requestUri = strtok(NULL, " ");

	// Si l'URL est la racine "/" il faut spécifier le fichier index par défaut
	// Sinon récupérer le URL du fichier sans le premier "/"
	if (strcmp(requestUri, "/") == 0)
		filename = FILE_INDEX;
	else {
		filename = (char *) alloca(strlen(requestUri)-1);
		strcpy(filename, requestUri+1);
	}

	// Création du chemin complet du fichier
	filepathSize = strlen(chemin_fichiers) + strlen(filename);
	filepath = (char *) calloc(filepathSize, sizeof(char));
	sprintf(filepath, "%s%s", chemin_fichiers, filename);

	return filepath;
}

/**
 * La fonction sendFile vérifie l'existance du fichier et envoi se dernier au client via la socket ouverte.
 * Si le fichier n'existe pas, l'erreur 404 est envoyée.
 */
void sendFile(int sockfd, char *filepath) {

	int			fdFile, rd;
	char		bufFile[TAILLE_READ_BUFFER];
	strHeader	*header = (strHeader *) alloca(sizeof(strHeader));

	if (fileInfo(filepath, header) == 404) {
	// Si le fichier n'existe pas, création du header correspondant et
	// et envoi le fichier 404

		filepath = FILE_404;

		if (fileInfo(filepath, header) != 200) {
			fprintf(stderr, "File 404 not found\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (header->http_code == 403) {
		filepath = FILE_403;
		
		if (fileInfo(filepath, header) != 200) {
			fprintf(stderr, "File 403 not found\n");
			exit(EXIT_FAILURE);
		}
	}

	// Création du header de réponse
	buildHeader(header);

	// Envoi du header
	if (write(sockfd, header->str_header, header->str_header_length) < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	// Ouverture du fichier demandé
	if ((fdFile = open(filepath, O_RDONLY)) < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	// Lecture et envoi du fichier demandé
	do {
		if ((rd = read(fdFile, bufFile, TAILLE_READ_BUFFER)) < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if ((write(sockfd, bufFile, rd)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	} while (rd != 0);

	// Fermeture du descripteur du fichier
	close(fdFile);
}

/**
 * Récupération des informations nécessaires et création de l'en-tête
 * pour la réponse du serveur.
 */
void buildHeader(strHeader *header) {

	strHeader *rhd = header;

	rhd->server_info	= SERVER_INFO;
	rhd->http_version	= HTTP_VERSION;
	// rhd->date		= time

	// TODO Mime dynamique
	rhd->content_mime	= "text/html";

	// Status
	if		(rhd->http_code == 200)	rhd->http_status = "OK";
	else if	(rhd->http_code == 403)	rhd->http_status = "Forbidden";
	else if	(rhd->http_code == 404)	rhd->http_status = "Not found";

	// Création du header sous forme d'une chaîne
	char *tmp_str = alloca(TAILLE_READ_BUFFER * sizeof(char));
	sprintf(tmp_str, "HTTP/%s %d %s\nServer: %s\nContent-Length: %d\nContent-Type: %s\n\n", 
		rhd->http_version, rhd->http_code, rhd->http_status, 
		rhd->server_info, rhd->content_length, rhd->content_mime);

	rhd->str_header_length = strlen(tmp_str);
	rhd->str_header = (char *) calloc(rhd->str_header_length + 1, sizeof(char));
	strcpy(rhd->str_header, tmp_str);
}


/**
 * La fonction fileInfo returne le code HTTP selon la disponnibilité ou non
 * du fichier demandé en paramètre. Aussi elle permet de tester si ce fichier existe, 
 * et dans un second temps permettra de vérifier les droits d'accès au fichier.
 */
int fileInfo(char *filepath, strHeader *header) {

	struct	stat fileStat;
	header->content_length = 0;

	if(stat(filepath,&fileStat) < 0) {

		fprintf(stderr, "file %s not found\n", filepath);
		header->http_code = 404;
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

		header->content_length = fileStat.st_size;
		header->http_code = 200;
		return 200;
	}
}

