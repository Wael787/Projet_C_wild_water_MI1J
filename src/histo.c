#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // vérification du fichier
#include <errno.h>
#include "histo.h" 
#include "codes_erreur.h"

#define MAX_LINE 4096
#define COL_MAX 3      // Volume brut (colonne 4 du fichier, index 3)
#define COL_SRC_REAL 4 // Taux de fuite (colonne 5 du fichier, index 4)

// Fonction d'analyse de double, similaire à celle de fuites.c
static double analyserDoubleHisto(const char* str) {
    if (strcmp(str, "-") == 0){
      return 0.0;
    }
    char* endptr;
    errno = 0;
    double val = strtod(str, &endptr);
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
      return 0.0;
    }
     return val; // On ne divise pas par 100 ici si on veut le taux brut
}

// Fonction de traitement pour la commande 'histo' (histogramme).
 
int traiter_histo(int argc, char *argv[]) {
    
    const char *mode = argv[2];
    const char *fichier_entree = argv[3];       
    const char *fichier_sortie = argv[4];  
    
    // Détermination de la colonne à traiter
    int index_colonne;
    if (strcmp(mode, "max") == 0) {
        index_colonne = COL_MAX;
    } else if (strcmp(mode, "src") == 0 || strcmp(mode, "real") == 0) {
        index_colonne = COL_SRC_REAL;
    } else {
        return ERR_USAGE_C;
    }

    // AVERTISSEMENT D'ÉCRASEMENT
    struct stat buffer;
    if (stat(fichier_sortie, &buffer) == 0) {
        fprintf(stderr, "AVERTISSEMENT: Le fichier de sortie '%s' existe et sera ÉCRASÉ.\n", fichier_sortie);
    }
    

    FILE* f_entree = fopen(fichier_entree, "r");
    if (!f_entree){
      return ERR_FICHIER_IN;
    }
    
    FILE* f_sortie = fopen(fichier_sortie, "w");
    if (!f_sortie) {
        fclose(f_entree);
        return ERR_FICHIER_OUT;
    }

    fprintf(f_sortie, "Mode;Colonne_Traitee;Valeur_Extrait\n");

    char ligne[MAX_LINE];
    // Ignorer la première ligne
    if (fgets(ligne, MAX_LINE, f_entree) == NULL) {
        fclose(f_entree);
        fclose(f_sortie);
        return 0; // Fichier vide
    }

    // Traitement selon le mode
    while (fgets(ligne, MAX_LINE, f_entree) != NULL) {
        ligne[strcspn(ligne, "\n")] = '\0';
        if (ligne[0] == '\0'){ 
          continue;
        }

        char ligne_tampon[MAX_LINE];
        strcpy(ligne_tampon, ligne);
        
        char *sauvegarde_ptr = NULL;
        char *col[5];
        
        // Parsing de toutes les colonnes nécessaires (0 à 4)
        for (int i = 0; i < 5; i++) {
            col[i] = strtok_r((i == 0) ? ligne_tampon : NULL, ";", &sauvegarde_ptr);
            if (!col[i]) {
                // Erreur de format de ligne
                fprintf(stderr, "Erreur C: Ligne du fichier d'entrée mal formatée.\n");
                continue;
            }
        }
        
        double valeur_traitee = 0.0;
        
        if (index_colonne == COL_MAX) {
            // Lecture du Volume (long long)
            long long volume_ll = analyserLongLong(col[index_colonne]); 
            valeur_traitee = (double)volume_ll;
            // TODO: Ajouter ici la logique de calcul de classes/fréquences pour l'histogramme
            
        } else if (index_colonne == COL_SRC_REAL) {
            // Lecture du Taux de fuite (double)
            valeur_traitee = analyserDoubleHisto(col[index_colonne]);
            // TODO: Ajouter ici la logique de calcul de classes/fréquences pour l'histogramme
        }

        fprintf(f_sortie, "%s;%d;%.2f\n", mode, index_colonne, valeur_traitee);

        // NOTE : Le mode 'real' impliquerait de ne traiter que les données 
        // jugées "réelles" après un filtrage, ce qui n'est pas implémenté ici.
    }

     
    
    fclose(f_entree);
    fclose(f_sortie);
    
    return 0;
}
