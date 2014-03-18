#ifndef _OPTION_H_
#define _OPTION_H_

#define PORT_SERVEUR_DEFAUT   "4321"
#define TAILLE_FICHIER_LOG    1024
#define CHEMIN_FICHIERS_HTML  "/var/webserver"
#define MAX_CONNEXION_CLIENTS 10

void options();
void sous_options(char *sousopt, int *taille_log, int *max_cli, char **path_html);
void aide(char *nom_programme);

#endif
