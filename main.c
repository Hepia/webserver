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

	//create_socket(atoi(argv[1]));
    return EXIT_SUCCESS;
}

void testoption(int argc, char *argv[], 
                char *port_srv, char *chemin_fichiers,
                int taille_log, int max_connexion)
{
    fprintf(stdout, "Mini Serveur Web\n");
    fprintf(stdout, "Le serveur écoute sur le port : %s\n", port_srv);
    fprintf(stdout, "La racine des fichiers HTML est : %s\n", chemin_fichiers);
    fprintf(stdout, "La taille taille du fichier lod : %d Ko\n", taille_log);
    fprintf(stdout, "Le nombre maximum de clients simultanés : %d\n", max_connexion);

    fprintf(stdout, "Argument supplémentaires : ");
    for(int i = 0; i < argc; i++)
    {
        fprintf(stdout, "%s - ", argv[i]);
    }
    fprintf(stdout, "\n");
}
