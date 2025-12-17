#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "acteur.h"     
#include "codes_erreur.h"

// Alloue et initialise un nouveau nœud de distribution (acteur).
 
pNoeudDistribution creerNoeudDistribution(const char* identifiant, double taux_fuite) {
    pNoeudDistribution nouveau = (pNoeudDistribution)malloc(sizeof(NoeudDistribution));
    if (!nouveau) {
        fprintf(stderr, "Erreur C: échec d'allocation mémoire pour le NoeudDistribution\n");
        exit(ERR_TRAITEMENT);
    }
    nouveau->identifiant = strdup(identifiant);
    if (!nouveau->identifiant) {
        fprintf(stderr, "Erreur C: échec d'allocation mémoire pour l'identifiant\n");
        free(nouveau);
        exit(ERR_TRAITEMENT);
    }
    nouveau->taux_fuite = taux_fuite;
    nouveau->volume_recu = 0.0;
    nouveau->enfants = NULL;
    return nouveau;
}

// Ajoute un acteur destinataire (enfant) au nœud source.
 
void ajouterEnfant(pNoeudDistribution source, pNoeudDistribution destination) {
    pElementEnfant nouveau_maillon = (pElementEnfant)malloc(sizeof(ElementEnfant));
    if (!nouveau_maillon) {
        fprintf(stderr, "Erreur C: échec d'allocation mémoire pour ElementEnfant\n");
        exit(ERR_TRAITEMENT);
    }
    nouveau_maillon->acteur_destination = destination;
    nouveau_maillon->suivant = source->enfants;
    source->enfants = nouveau_maillon;
}

// Compte le nombre d'enfants (destinataires) d'un acteur.

int compterEnfants(pNoeudDistribution parent) {
    int compte = 0;
    pElementEnfant courant = parent->enfants;
    while (courant != NULL) {
        compte++;
        courant = courant->suivant;
    }
    return compte;
}

// Libère la mémoire allouée pour un NoeudDistribution et ses éléments ElementEnfant.
 
void libererActeur(pNoeudDistribution acteur) {
    if (acteur == NULL){
      return;
    }

    pElementEnfant courant = acteur->enfants;
    pElementEnfant suivant;

    // Libérer les maillons de la liste chaînée.
    while (courant != NULL) {
        suivant = courant->suivant;
        free(courant); 
        courant = suivant;
    }

    // Libérer le NoeudDistribution.
    free(acteur->identifiant);
    free(acteur);
}
