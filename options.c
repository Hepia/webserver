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

// Si la constante symbolique OPTIONS_LONGUES est définie dans le Makefile, alors la gestion
// des options longues propre à GNU sera prise en compte.
#ifdef OPTIONS_LONGUES
#include <getopt.h>
#endif

#include "include/options.h"
#include "include/server_const.h"

/*
 * La fonction options traite les options courtes, les options longues GNU si la constante
 * symbolique est définie et les sous-options passées en paramètre au serveur.
 * Les options servent à paramètrer le serveur web. Les paramètres fournis sur la ligne
 * de commande ont la priorités. Ensuite viennent les paramètres enregistrés dans des
 * variables d'environnement. Si aucun paramètre n'est fournis, le serveur web utilise
 * des paramètres par défaut.
 */

void options(int argc, char *argv[], 
             char **port_srv, char **chemin_fichiers, 
             int *taille_log, int *max_connexion)
{
    // Copie des varibales d'environnement.
    char *opt_prt_dflt = NULL;
    char *opt_path_fls = NULL;
    int  opt_sz_log    = 0;
    int  opt_max_cli   = 0;

    int options;

	// Liste des options longues.
#ifdef OPTIONS_LONGUES
    int index = 0;
    static struct option opt_long[] = 
    {{"port"   , 1, NULL, 'p'},
     {"options", 1, NULL, 'o'},
     {"help"   , 0, NULL, 'h'},
     {"info"   , 0, NULL, 'i'},
     {NULL     , 0, NULL, 0}};
#endif
    char *ret_environ = NULL;

    // Lecture des variables d'environnement.

    // Lecture de la variable d'environnement OPT_PRT_DLFT.
    ret_environ = getenv("OPT_PRT_DFLT");

    if((ret_environ != NULL) && (strlen(ret_environ) != 0))
    {
        opt_prt_dflt = calloc(strlen(ret_environ) + 1, sizeof(char));

        if(opt_prt_dflt != NULL)
        {
			// Copie du contenu de la variable d'environnement dans une
			// variable locale.
            strcpy(opt_prt_dflt, ret_environ);
            *port_srv = opt_prt_dflt;
        }
        else
        {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
    }

    // Lecture de la variable d'environnement OPT_SZ_LOG.
    ret_environ = getenv("OPT_SZ_LOG");

    if((ret_environ != NULL))
    {
		// Copie du contenu de la variable d'environnement dans une
		// variable locale.
        if(sscanf(ret_environ, "%d", taille_log) == 1)
            opt_sz_log = *taille_log;
    }

    // Lecture de la variable d'environnement OPT_PATH_FLS.
    ret_environ = getenv("OPT_PATH_FLS");

    if((ret_environ != NULL) && (strlen(ret_environ) != 0))
    {
        opt_path_fls = calloc(strlen(ret_environ) + 1, sizeof(char));

        if(opt_path_fls != NULL)
        {
			// Copie du contenu de la variable d'environnement dans une
			// variable locale.
            strcpy(opt_path_fls, ret_environ);
            *chemin_fichiers = opt_path_fls;
        }
        else
        {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
    }

    // Lecture de la variable d'environnement OPT_MAX_CLI.
    ret_environ = getenv("OPT_MAX_CLI");

    if(ret_environ != NULL)
    {
		// Copie du contenu de la variable d'environnement dans une
		// variable locale.
        if(sscanf(ret_environ, "%d", max_connexion) == 1)
            opt_max_cli = *max_connexion;
    }

    // Lecture des options.
    opterr = 1; // Copie du caractère inconnu dans la variable extern 
                // optopt et getopt retourne le caractère '?'.

    while(1)
    {
    #ifdef OPTIONS_LONGUES
		// Récupération des options longues et courtes.
        options = getopt_long(argc, argv, "p:o:h:i", opt_long, &index);
    #else
		// Récupération des options courtes.
        options = getopt(argc, argv, "p:o:h:i");
    #endif

		// Condition de sortie de la boucle lorsqu'il n'y a plus
		// d'option à extraire des arguments.
        if(options == -1) break;

        switch(options)
        {
            case 'p' : // Option p ou port.
                if(opt_prt_dflt != NULL)
                    free(opt_prt_dflt);
                opt_prt_dflt = NULL;
                *port_srv = optarg; // Variable globale externe contenant
                                   // la chaine de l'option.
                break;

            case 'o' : // Option o ou options (sous-options).
				// Traitement des sous-options.
                sous_options(optarg, taille_log, max_connexion, chemin_fichiers);
                break;

            case 'h' :  // Option h ou help.
				// Affichage de l'aide.
                aide(argv[0]);
                exit(EXIT_SUCCESS);

            case 'i' : // Option i ou info.
				// Affichage des paramètes du serveur.
                info(*port_srv, *chemin_fichiers,
                     *taille_log, *max_connexion);
                exit(EXIT_SUCCESS);

            default :
                break;
        }
    }
}

/*
 * La fonction sous-options trait les sous-options liées à l'option o ou option.
 * l'option log permet de définir la taille maximum du fichier de log. L'option
 * path permet de définir le chemin d'accès vers les fichiers html. l'option
 * maxcli permet de défninir un nombre maximum de clients que peut traiter le
 * serveur en parallèle.
 */

void sous_options(char *str_sous_opt, int *taille_log, 
                  int *max_cli, char **chemin_html)
{
    int  sousopt        = 0;
    int  valeur_log     = 0;
    int  valeur_max_cli = 0;
    char *valeur        = NULL;
    
	// tableau de chaines contenant les sous-options
    char *tokens[] = {"log", "path", "maxcli", NULL};
    
	// Boucle de récupération des sous-options.
    while((sousopt = getsubopt(&str_sous_opt, tokens, &valeur)) != -1)
    {
        switch(sousopt)
        {
            case 0 : // sous-option log
                if(valeur == NULL)
                {
                    fprintf(stderr, "taille du fichier log attendue\n");
                    break;
                }
				// Lecture de la taille du fichier de log.
                if(sscanf(valeur, "%d", &valeur_log) != 1)
                {
                    fprintf(stderr, "taille du fichier log invalide\n");
                    break;
                }

                *taille_log = valeur_log;
                break;

            case 1 : // sous-option path
                if(valeur == NULL)
                {
                    fprintf(stderr, "chemin des fichiers html attendu\n");
                    break;
                }

                *chemin_html = valeur;
                break;

            case 2 : // sous-option maxcli
                if(valeur == NULL)
                {
                    fprintf(stderr, "nombre maximum de clients attendu\n");
                    break;
                }
				// Lecture du nombre maximum de clients traités en parallèle par
				// le serveur.
                if(sscanf(valeur, "%d", &valeur_max_cli) != 1)
                {
                    fprintf(stderr, "nombre maximum de clients invalide\n");
                    break;
                }

                *max_cli = valeur_max_cli;
                break;

            default :
                break;
        }
    }
}

/*
 * La fonction aide affiche une aide contextuelle avec toutes les options disponibles.
 */

void aide(char *nom_programme)
{
    fprintf(stdout, "Mini serveur web\n");
    fprintf(stdout, "Syntaxe : %s [options]\n", nom_programme);
    fprintf(stdout, "Options : \n");

#ifdef OPTIONS_LONGUES
    fprintf(stdout, " --help\n");
#endif
    fprintf(stdout, " -h                    Aide\n");
#ifdef OPTIONS_LONGUES
    fprintf(stdout, " --port <numero_port>\n");
#endif
    fprintf(stdout, " -p <numero_port>      Numéro de port TCP\n");
#ifdef OPTIONS_LONGUES
    fprintf(stdout, " --options [sous_options]\n");
#endif
    fprintf(stdout, " -o [sous_options]     Définir des sous-options\n");

    fprintf(stdout, "Sous-options :\n");
    fprintf(stdout, " log=<taille_en_o>    Taille du fichier des logs en [o]\n");
    fprintf(stdout, " path=<chemin>         Chemin des fichiers html\n");
    fprintf(stdout, " maxcli=<nombre>       Nombre maximum de clients simultanés\n");
    fprintf(stdout, "Variables d'environnement :\n");
    fprintf(stdout, " OPT_PRT_DFLT          Variable contenant le numéro de port TCP\n");
    fprintf(stdout, " OPT_SZ_LOG            Variable contenant la taille du fichiers de log en [Ko]\n");
    fprintf(stdout, " OPT_PATH_FLS          Variable contenant le chemin des fichiers html\n");
    fprintf(stdout, " OPT_MAX_CLI           Variable contenant le nombre maximum de clients simultanés\n");
}

/*
 * La fonction info affiche des informations concernant le serveur et la
 * machine hôte.
 */
 
void info(char *port_srv, char *chemin_fichiers,
          int taille_log, int max_connexion)
{
    fprintf(stdout, "Mini Serveur Web\n");
    fprintf(stdout, "Port du serveur                : %s\n", port_srv);
    fprintf(stdout, "Répertoire de travail          : %s\n", chemin_fichiers);
    fprintf(stdout, "Taille des logs                : %d o\n", taille_log);
    fprintf(stdout, "Nombre de clients authorisé    : %d\n", max_connexion);
    fprintf(stdout, "Nombre de coeurs               : %d\n", 8);
}
