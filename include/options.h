#ifndef _OPTION_H_
#define _OPTION_H_

// Liste des variables d'environnement.
// ------------------------------------
// OPT_PRT_DFLT
// OPT_SZ_LOG
// OPT_PATH_FLS
// OPT_MAX_CLI

#define PORT_SERVEUR_DEFAUT   "4321"
#define TAILLE_FICHIER_LOG    1024
#define CHEMIN_FICHIERS_HTML  "./"
#define MAX_CONNEXION_CLIENTS 10

void options(int argc, char *argv[], 
             char **port_srv, char **chemin_fichiers, 
             int *taille_log, int *max_connexion);
void sous_options(char *sousopt, int *taille_log, int *max_cli, char **path_html);
void aide(char *nom_programme);

#endif
