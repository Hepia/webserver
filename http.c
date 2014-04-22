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

extern char *chemin_fichiers;

void sendFile(int fdSocket, char *filename) {

	int			filePathSize, fdFile, rd;
	int			bSize = 16;
	char		*filepath;
	char		bufFile[bSize];
	strHeader	*header = calloc(1, sizeof(strHeader));

	// Création du chemin complet du fichier
	filePathSize = strlen(chemin_fichiers) + strlen(filename);
	filepath = (char *) malloc(filePathSize);
	snprintf(filepath, filePathSize + 1, "%s%s", chemin_fichiers, filename);

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
	if (write(fdSocket, header->str_header, header->str_header_length) < 0) {
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
		if ((rd = read(fdFile, bufFile, bSize)) < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		if ((write(fdSocket, bufFile, rd)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}

	} while (rd != 0);

	// Fermeture du fichier
	close(fdFile);

}

void buildHeader(strHeader *header) {

	strHeader *rhd = header;

	rhd->server_info	= "Webserver/0.2 - Felipe Paul Martins & Joachim Schmidt";
	rhd->http_version	= HTTP_VERSION;
	// rhd->date		= time

	rhd->content_mime	= "text/html";
	
	if		(rhd->http_code == 200)	rhd->http_status = "OK";
	else if	(rhd->http_code == 403)	rhd->http_status = "Not authorized";
	else if	(rhd->http_code == 404)	rhd->http_status = "Not found";

	char *tmp_str = alloca(255 * sizeof(char));

	sprintf(tmp_str, "HTTP/%s %d %s\nServer: %s\nContent-Length: %d\nContent-Type: %s\n\n", rhd->http_version, rhd->http_code, rhd->http_status, rhd->server_info, rhd->content_length, rhd->content_mime);


	rhd->str_header_length = strlen(tmp_str);

	rhd->str_header = calloc(rhd->str_header_length + 1, sizeof(char));
	strcpy(rhd->str_header, tmp_str);
}


/**
 * La fonction fileInfo returne la taille, en octets, du fichier qui est passé
 * en paramètre. Aussi elle permet de tester si ce fichier existe, et dans un 
 * second temps permettra de vérifier les droits d'accès au fichier.
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

