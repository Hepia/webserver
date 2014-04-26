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

#define FILE_INDEX "index.html"
#define FILE_403 "./www/403"
#define FILE_404 "./www/404"
#define CRLF "\r\n"

typedef struct stuHttpData stuHttpData;
struct stuHttpData 
{
    int  socketfd;
    char *q_ipcli;
    char *q_method;
    char *q_host;
    char *q_filepath;
    char *q_header;			// En-tête de la requête HTTP
    char *r_status;
    char *r_date;
    char *r_content_mime;
    char *r_header;				// En-tête HTTP de réponse
    int  r_code;
    int  r_content_length;
    int  r_header_size;
};

 /*
 * Prototypes des fonctions de gestion des options.
 */
void* processHttp       (int sockfd, char *qIpCli);
void* readQueryHeader   (stuHttpData *httpData);
void* parseHeader       (stuHttpData *httpData);
void* buildHeader       (stuHttpData *httpData);
void* sendFile          (stuHttpData *httpData);
int   fileInfo          (stuHttpData *httpData);

#endif
