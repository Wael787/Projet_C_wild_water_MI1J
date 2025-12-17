#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h> // pour la vérification du fichier
#include "fuites.h"
#include "acteur.h"
#include "avl_index.h"
#include "codes_erreur.h"

#define MAX_LINE 4096
#define EPSILON 1e-9

// Fonctions d'Analyse (Parsing) 

static long long analyserLongLong(const char* str) {
    if (strcmp(str, "-") == 0){
      return 0;
    }
    char* endptr;
    errno = 0;
    long long val = strtoll(str, &endptr, 10);
    if (errno == ERANGE || endptr == str || *endptr != '\0') return 0;
    return val;
}

static double analyserDouble(const char* str) {
    if (strcmp(str, "-") == 0){ 
      return 0.0;
    }
    char* endptr;
    errno = 0;
    double val = strtod(str, &endptr);
    if (errno == ERANGE || endptr == str || *endptr != '\0'){
      return 0.0;
    }
    return val / 100.0;
}

// Logique de Propagation

static double calculer_fuites_recursive(pNoeudDistribution parent) {
    if (parent == NULL){
      return 0.0;
    }
    
    double fuite_locale = parent->volume_recu * parent->taux_fuite;
    double volume_restant = parent->volume_recu - fuite_locale;
    
    double fuite_totale = fuite_locale;
    
    int nombre_enfants = compterEnfants(parent);
    
    if (nombre_enfants == 0) {
        return fuite_totale;
    }

    double volume_par_enfant = volume_restant / nombre_enfants;
    
    pElementEnfant enfant_courant = parent->enfants;
    
    while (enfant_courant != NULL) {
        enfant_courant->acteur_destination->volume_recu = volume_par_enfant;
        fuite_totale += calculer_fuites_recursive(enfant_courant->acteur_destination); 
        enfant_courant = enfant_courant->suivant;
    }
    
    return fuite_totale;
}


// Traitement Principal de leaks

int traiter_fuites(int argc, char *argv[]) {
    
    const char *id_usine_racine = argv[2];      
    const char *fichier_entree = argv[3];       
    const char *fichier_sortie = argv[4];       
    
    // AVERTISSEMENT D'ÉCRASEMENT
    struct stat buffer;
    if (stat(fichier_sortie, &buffer) == 0) {
        fprintf(stderr, "⚠️ AVERTISSEMENT: Le fichier de sortie '%s' existe et sera ÉCRASÉ.\n", fichier_sortie);
    }
    
    
    FILE* f_entree = fopen(fichier_entree, "r");
    if (!f_entree){ 
      return ERR_FICHIER_IN;
    }
    
    pIndexAVL index_racine = NULL;
    char ligne[MAX_LINE];
    
    long long volume_lu_entree = 0;
    double taux_fuite_source_racine = 0.0;
    int ligne_valide = 0;
    
    // Construction du Réseau et de l'Index
    while (fgets(ligne, MAX_LINE, f_entree) != NULL) {
        ligne[strcspn(ligne, "\n")] = '\0';
        if (ligne[0] == '\0') continue;

        char ligne_tampon[MAX_LINE];
        strcpy(ligne_tampon, ligne);
        
        char *sauvegarde_ptr = NULL;
        char *col[5];
        
        for (int i = 0; i < 5; i++) {
            col[i] = strtok_r((i == 0) ? ligne_tampon : NULL, ";", &sauvegarde_ptr);
            if (!col[i]) goto ligne_suivante;
        }

        char *id_source = col[1];
        char *id_destination = col[2];
        long long volume_brut = analyserLongLong(col[3]);
        double taux_fuite = analyserDouble(col[4]);

        pNoeudDistribution acteur_source = NULL;
        pNoeudDistribution acteur_destination = NULL;

        index_racine = insererIndex(index_racine, id_destination, taux_fuite, &acteur_destination);
        index_racine = insererIndex(index_racine, id_source, 0.0, &acteur_source);
        ajouterEnfant(acteur_source, acteur_destination);

        if (strcmp(id_destination, id_usine_racine) == 0) {
            volume_lu_entree = volume_brut;
            taux_fuite_source_racine = acteur_source->taux_fuite; 
            ligne_valide = 1;
        }

        ligne_suivante:;
    }
    fclose(f_entree);
    
    // Préparation et Propagation
    pNoeudDistribution usine_racine = rechercherActeur(index_racine, id_usine_racine);

    if (usine_racine == NULL || volume_lu_entree == 0 || !ligne_valide) {
         libererAVL_Reseau(index_racine);
         return ERR_TRAITEMENT;
    }

    double volume_reel_entree = (double)volume_lu_entree * (1.0 - taux_fuite_source_racine);
    usine_racine->volume_recu = volume_reel_entree;

    double fuite_totale = calculer_fuites_recursive(usine_racine);
    
    //  Écriture du Résultat
    FILE* f_sortie = fopen(fichier_sortie, "w");
    if (!f_sortie) {
        libererAVL_Reseau(index_racine);
        return ERR_FICHIER_OUT;
    }

    fprintf(f_sortie, "identifier;total leaks (k.m3.year-1)\n");
    fprintf(f_sortie, "%s;%.2f\n", id_usine_racine, fuite_totale);

    fclose(f_sortie);
    
    // Nettoyage
    libererAVL_Reseau(index_racine);
    
    return 0;
}
