#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef OPTIONS_LONGUES
#include <getopt.h>
#endif

#include "include/options.h"

void sous_options(char *str_sous_opt, int *taille_log, int *max_cli, char **chemin_html)
{
    int   sousopt            = 0;
    int   valeur_log         = 0;
    int   valeur_max_cli     = 0;
    char *valeur_chemin_html = NULL;
    char *valeur             = NULL;
    
    char *tokens[] = {"log", "path", "maxcli", NULL};
    
    while((sousopt = getsubopt(str_sous_opt, tokens, &valeur)) != -1)
    {
        switch(sousopt)
        {
            case 0 : // log
                if(valeur == NULL)
                {
                    fprintf(stderr, "Taille du fichier log attendue\n");
                    break;
                }
                if(sscanf(valeur, "%d", &valeur_log) != 1)
                {
                    fprintf(stderr, "taille du fichier log invalide\n");
                    break;
                }
            case 1 : // path
            case 2 : // maxcli
        }
    }
}

void aide(char *nom_programme)
{
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

    fprintf(stdout, " -o [sous_options]\n");
    fprintf(stdout, "Sous-options :\n");
    fprintf(stdout, " log=<taille_en_ko>    Taille du fichier des logs en ko\n");
    fprintf(stdout, " path=<chemin>         Chemin des fichiers html\n");
    fprintf(stdout, " maxcli=<nombre>       Nombre maximum de clients servis en même temps\n");
}
