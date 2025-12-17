#include <stdio.h>
#include <string.h>
#include "fuites.h"
#include "histo.h"
#include "codes_erreur.h"

int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "Erreur C: Usage: %s {leaks|histo} [args...]\n", argv[0]);
        return ERR_USAGE_C;
    }
    
    const char *commande = argv[1];

    if (strcmp(commande, "leaks") == 0) {
  
        // Attendu : ./prog leaks ID F_In F_Out, donc 5 arguments
        if (argc != 5) {
            fprintf(stderr, "Erreur C: Usage: %s leaks ID_Usine Fichier_In Fichier_Out\n", argv[0]);
            return ERR_USAGE_C;
        }

        return traiter_fuites(argc, argv);
    } 
    else if (strcmp(commande, "histo") == 0) {
        
        // Attendu : ./prog histo mode F_In F_Out -> 5 arguments
        if (argc != 5) {
            fprintf(stderr, "Erreur C: Usage: %s histo {max|src|real} Fichier_In Fichier_Out\n", argv[0]);
            return ERR_USAGE_C;
        }

        const char *mode = argv[2];
        if (strcmp(mode, "max") == 0 || strcmp(mode, "src") == 0 || strcmp(mode, "real") == 0) {
          
            return traiter_histo(argc, argv); 
        } else {
            
            fprintf(stderr, "Erreur C: Mode histogramme invalide '%s' (attendu: max, src ou real).\n", mode);
            return ERR_USAGE_C;
        }
    }
    else {
        fprintf(stderr, "Erreur C: Commande inconnue '%s' (attendu: leaks ou histo).\n", commande);
        return ERR_COMMANDE_C;
    }
    
    return 0;
}
