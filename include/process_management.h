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

#ifndef _PROCESS_MANAGEMENT_H_
#define _PROCESS_MANAGEMENT_H_

/*
 * Prototypes des fonctions pour traiter la structure server_process et pour 
 * créer les nouveaux processus.
 */

struct server_process * init_server_process	 (int (*ptr_child_process)(void *), 
                                              int (*ptr_father_process)(void *),
                                              int *sock);
void                    delete_server_process(struct server_process *ptr_sp);
int                     call_fork            (int val, struct server_process *ptr_sp);

/*
 * Prototypes des fonctions correspondant au code des processus fils ou des processus
 * père.
 */

int                     ipv4_process         (void *data);
int                     ipv6_process         (void *data);

int                     child_process        (void *data);
int                     father_process       (void *data);

#endif