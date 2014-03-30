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

#include "include/options.h"
#include "include/socket.h"

void testoption(int argc, char *argv[], 
                char *port_srv, char *chemin_fichiers,
                int taille_log, int max_connexion);

int main(int argc, char *argv[])
{
    static char *port_srv        = PORT_SERVEUR_DEFAUT;
    static char *chemin_fichiers = CHEMIN_FICHIERS_HTML;
    int         taille_log       = TAILLE_FICHIER_LOG;
    int         max_connexion    = MAX_CONNEXION_CLIENTS;

    options(argc, argv, &port_srv, &chemin_fichiers,
            &taille_log, &max_connexion);
    testoption((argc - optind), &(argv[optind]), port_srv, chemin_fichiers,
               taille_log, max_connexion);

    tcp_server(port_srv);

    return EXIT_SUCCESS;
}

void testoption(int argc, char *argv[], 
                char *port_srv, char *chemin_fichiers,
                int taille_log, int max_connexion)
{
    fprintf(stdout, "Mini Serveur Web\n");
    fprintf(stdout, "Le serveur écoute sur le port : %s\n", port_srv);
    fprintf(stdout, "La racine des fichiers HTML est : %s\n", chemin_fichiers);
    fprintf(stdout, "La taille taille du fichier log : %d Ko\n", taille_log);
    fprintf(stdout, "Le nombre maximum de clients simultanés : %d\n", max_connexion);

    fprintf(stdout, "Argument supplémentaires : ");
    for(int i = 0; i < argc; i++)
    {
        fprintf(stdout, "%s - ", argv[i]);
    }
    fprintf(stdout, "\n");
}
