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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#ifndef SOL_IPV6
#define SOL_IPV6 41 // From /usr/src/linux-headers-3.11.0-12-generic/include/linux/socket.h:274
#endif

/*
 * Prototypes des fonctions de gestion des sockets et des communications
 * réseaux.
 */

int* create_socket_stream(const char *host_name, const char *serv_port, 
                          const char *proto_name);
int  tcp_server          (void *arg);
int  close_tcp_server    (void);
void process_connection  (int sock);
int  print_socket_address(int sock, int where, char *ext_buffer);

#endif
