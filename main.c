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
#include "include/sig_handler.h"
#include "include/server_const.h"

/*
 * Variable externe se trouvant dans le fichier sig_handler.c.
 */
 
extern struct sigaction *list_action;

/*
 * Prototype des fonctions propres au fichier main.c.
 */

void testoption(int argc, char *argv[], 
                char *port_srv, char *chemin_fichiers,
                int taille_log, int max_connexion);

// Définition par défaut des variables utiles au programme.

char *port_srv        = PORT_SERVEUR_DEFAUT;
char *chemin_fichiers = CHEMIN_FICHIERS_HTML;
int  taille_log       = TAILLE_FICHIER_LOG;
int  max_connexion    = MAX_CONNEXION_CLIENTS;

/*
 * Point d'entrée principale du programme.
 */

int main(int argc, char *argv[])
{
    struct serv_param param;

    param.port_srv        = port_srv;
    param.chemin_fichiers = chemin_fichiers;
    param.taille_log      = taille_log;
    param.max_connexion   = max_connexion;
	
	// Initialisation du gestionnaire de signaux.
    init_handler(list_action);

	// Traitement des options passée en paramètre.
    options(argc, argv, &port_srv, &chemin_fichiers,
            &taille_log, &max_connexion);
	// Test du bon fonctionnement des options après traitement.
    testoption((argc - optind), &(argv[optind]), port_srv, chemin_fichiers,
               taille_log, max_connexion);

	// Lancement du serveur TCP/IP.
    tcp_server((void *)&param);

    return EXIT_SUCCESS;
}

/*
 * La fonction testoption, test les options passées en paramètre au programme.
 */
 
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
