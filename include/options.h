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

#ifndef _OPTION_H_
#define _OPTION_H_

/*
 * Prototypes des fonctions de gestion des options.
 */

void options     (int argc, char *argv[], 
                  char **port_srv, char **chemin_fichiers, 
                  int *taille_log, int *max_connexion);
void sous_options(char *sousopt, int *taille_log, int *max_cli, char **path_html);
void aide        (char *nom_programme);
void info        (char *port_srv, char *chemin_fichiers,
                  int taille_log, int max_connexion);

#endif
