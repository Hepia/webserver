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

#ifndef _HTTP_H_
#define _HTTP_H_

typedef struct response_header response_header;
struct response_header 
{
	char	*http_version;
	char	*http_status;
	int		http_code;
	char	*server_info;
	char	*date;
	int		content_length;
	int		content_mime;
	char	*str_header;
	int		str_header_length;
};


 /*
 * Prototypes des fonctions de gestion des options.
 */

void	sendFile		(int fdSocket, char *filepath, char *chemin_fichiers);
int		fileInfo		(char *filepath);
void 	buildHeader		(int http_code, response_header *rhd);


#endif