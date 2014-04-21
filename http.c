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

#include "include/http.h"

#define HTTP_VERSION "1/1"
#define HEADER200 "HTTP1/1 200 OK\nContent-length: "
#define HEADER404 "HTTP/1.1 404 Not found\nContent-type: text/html"


void sendFile(int fdSocket, char *filename, char *chemin_fichiers) {

	int				filePathSize, bufHeaderSize, lengthFile, fdFile, rd;
	int				bSize = 16;
	char			*bufHeader, *filepath;
	char			bufFile[bSize];
	response_header *header = malloc(sizeof(response_header));

	// Création du chemin complet du fichier
	filePathSize = strlen(chemin_fichiers) + strlen(filename);
	filepath = (char *) malloc(filePathSize);
	snprintf(filepath, filePathSize + 1, "%s%s", chemin_fichiers, filename);

	if ((header->content_length = fileInfo(filepath)) == 404) {
		sendFile(fdSocket, "404", chemin_fichiers);
	}
	else {

		if (strcmp(filename, "404") == 0)
			buildHeader(404, &header);
		else
			buildHeader(200, &header);

		if (write(fdSocket, header->str_header, header->str_header_length) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}


		fdFile = open(filepath, O_RDONLY);

		do {

			rd = read(fdFile, bufFile, bSize);
			write(fdSocket, bufFile, rd);

		} while (rd != 0);
	}

	close(fdFile);

}

void buildHeader(int http_code, response_header *rhd) {

	rhd->server_info	= "Webserver/0.2 - Felipe Paul Martins & Joachim Schmidt";
	//	rhd->date			= time

	rhd->http_version	= HTTP_VERSION;
	rhd->http_code		= http_code;

	rhd->content_mime	= "text/html";
	
	if		(http_code == 200)	rhd->http_status = "OK";
	else if	(http_code == 404)	rhd->http_status = "Not found";

	rhd->str_header_length = snprintf(rhd->str_header, 0, "HTTP/%s %d %s\nServer: %s\nContent-Length: %d\nContent-Type: %s", rhd->http_version, rhd->http_code, rhd->http_status, rhd->server_info, rhd->content_length, rhd->content_mime);

	printf("snprintf %d\n%s", rhd->str_header_length, rhd->str_header);

	rhd->str_header = malloc(rhd->str_header_length);
	snprintf(rhd->str_header, rhd->str_header_length, "HTTP/%s %d %s\nServer: %s\nContent-Length: %d\nContent-Type: %s", rhd->http_version, rhd->http_code, rhd->http_status, rhd->server_info, rhd->content_length, rhd->content_mime);

	printf("Size: %d, header:\n%s\n", rhd->str_header_length, rhd->str_header);
	return rhd;
}


/**
 * La fonction fileInfo returne la taille, en octets, du fichier qui est passé
 * en paramètre. Aussi elle permet de tester si ce fichier existe, et dans un 
 * second temps permettra de vérifier les droits d'accès au fichier.
 */
int fileInfo(char *filepath) {

	struct	stat fileStat;

	if(stat(filepath,&fileStat) < 0) {

		fprintf(stderr, "file %s not found\n", filepath);
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

		return fileStat.st_size;
	}
}

