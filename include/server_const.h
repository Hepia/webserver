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

#ifndef _SERVER_CONST_H_
#define _SERVER_CONST_H_

// Liste des variables d'environnement.
// ------------------------------------
// OPT_PRT_DFLT
// OPT_SZ_LOG
// OPT_PATH_FLS
// OPT_MAX_CLI

/*
 * Valeurs par défaut, si aucune option n'est passée en paramètre au programme et
 * si aucune variable d'environnement n'est définie.
 */

#define PORT_SERVEUR_DEFAUT   "4321"
#define TAILLE_FICHIER_LOG    1024
#define CHEMIN_FICHIERS_HTML  "./"
#define MAX_CONNEXION_CLIENTS 10

/*
 * Limites pour certain paramètre du serveur.
 */

#define NUM_PORT_MAX 		65534
#define NUM_PORT_MIN 		1024
#define TAILLE_READ_BUFFER	256
#define TAILLE_FILE_ECOUTE	128 // Maximum sous Linux.

/*
 * Symbole faisant référence soit à l'adresse local d'une socket, soit à l'adresse
 * distante.
 */

#define LOCAL 				0
#define DISTANT 			1

#endif