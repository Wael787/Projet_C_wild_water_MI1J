#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codes_erreur.h"
#include "histo.h"
#include "fuites.h"

int main(int argc, char* argv[]) {
    
    if (argc < 3) {
        fprintf(stderr, "Erreur C: Usage: %s {histo|leaks} [args...]\n", argv[0]);
        return ERR_USAGE_C;
    }
    
    const char *commande = argv[1];

    if (strcmp(commande, "histo") == 0) {
        return traiter_histo(argc, argv);
    } 
    else if (strcmp(commande, "leaks") == 0) {
        return traiter_fuites(argc, argv);
    }
    else {
        fprintf(stderr, "Erreur C: Commande inconnue '%s' (attendu: histo ou leaks).\n", commande);
        return ERR_COMMANDE_C;
    }
    
    return 0;
}
