#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef OPTIONS_LONGUES
#include <getopt.h>
#endif

#include "include/options.h"

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

    // Lecture de la variable OPT_PRT_DLFT.
    ret_environ = getenv("OPT_PRT_DFLT");

    if((ret_environ != NULL) && (strlen(ret_environ) != 0))
    {
        opt_prt_dflt = calloc(strlen(ret_environ) + 1, sizeof(char));

        if(opt_prt_dflt != NULL)
        {
            strcpy(opt_prt_dflt, ret_environ);
            *port_srv = opt_prt_dflt;
        }
        else
        {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
    }

    // Lecture de la variable OPT_SZ_LOG.
    ret_environ = getenv("OPT_SZ_LOG");

    if((ret_environ != NULL))
    {
        if(sscanf(ret_environ, "%d", taille_log) == 1)
            opt_sz_log = *taille_log;
    }

    // Lecture de la variable OPT_PATH_FLS.
    ret_environ = getenv("OPT_PATH_FLS");

    if((ret_environ != NULL) && (strlen(ret_environ) != 0))
    {
        opt_path_fls = calloc(strlen(ret_environ) + 1, sizeof(char));

        if(opt_path_fls != NULL)
        {
            strcpy(opt_path_fls, ret_environ);
            *chemin_fichiers = opt_path_fls;
        }
        else
        {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
    }

    // Lecture de la variable OPT_MAX_CLI.
    ret_environ = getenv("OPT_MAX_CLI");

    if(ret_environ != NULL)
    {
        if(sscanf(ret_environ, "%d", max_connexion) == 1)
            opt_max_cli = *max_connexion;
    }

    // Lecture des options.
    opterr = 1; // Copie du caractère inconnu dans la variable extern 
                // optopt et getopt retourne le caractère '?'.

    while(1)
    {
    #ifdef OPTIONS_LONGUES
        options = getopt_long(argc, argv, "p:o:h", opt_long, &index);
    #else
        options = getopt(argc, argv, "p:o:h");
    #endif

        if(options == -1) break;

        switch(options)
        {
            case 'p' :
                if(opt_prt_dflt != NULL)
                    free(opt_prt_dflt);
                opt_prt_dflt = NULL;
                *port_srv = optarg; // Variable globale externe contenant
                                   // la chaine de l'option.
                break;

            case 'o' :
                sous_options(optarg, taille_log, max_connexion, chemin_fichiers);
                break;

            case 'h' :
                aide(argv[0]);
                exit(EXIT_SUCCESS);

            default :
                break;
        }
    }
}

void sous_options(char *str_sous_opt, int *taille_log, int *max_cli, char **chemin_html)
{
    int  sousopt        = 0;
    int  valeur_log     = 0;
    int  valeur_max_cli = 0;
    char *valeur        = NULL;
    
    char *tokens[] = {"log", "path", "maxcli", NULL};
    
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
    fprintf(stdout, " log=<taille_en_ko>    Taille du fichier des logs en [Ko]\n");
    fprintf(stdout, " path=<chemin>         Chemin des fichiers html\n");
    fprintf(stdout, " maxcli=<nombre>       Nombre maximum de clients simultanés\n");
    fprintf(stdout, "Variables d'environnement :\n");
    fprintf(stdout, " OPT_PRT_DFLT          Variable contenant le numéro de port TCP\n");
    fprintf(stdout, " OPT_SZ_LOG            Variable contenant la taille du fichiers de log en [Ko]\n");
    fprintf(stdout, " OPT_PATH_FLS          Variable contenant le chemin des fichiers html\n");
    fprintf(stdout, " OPT_MAX_CLI           Variable contenant le nombre maximum de clients simultanés\n");
}

void info(int argc, char *argv[],
          char *port_srv, char *chemin_fichiers,
          int taille_log, int max_connexion)
{
    fprintf(stdout, "Mini Serveur Web\n");
    fprintf(stdout, "Port du serveur                : %s\n", port_srv);
    fprintf(stdout, "Répertoire de travail          : %s\n", chemin_fichiers);
    fprintf(stdout, "Taille des logs                : %d Ko\n", taille_log);
    fprintf(stdout, "Nombre de clients authorisé    : %d\n", max_connexion);
    fprintf(stdout, "Nombre de coeurs               : %d\n", 8);
}
