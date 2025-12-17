#ifndef AVL_INDEX_H
#define AVL_INDEX_H

#include "acteur.h"
#include "codes_erreur.h"

//Nœud d'index AVL pour la recherche rapide des acteurs par ID.

typedef struct index_avl {
    char* identifiant;
    pNoeudDistribution noeud_reseau;
    int hauteur;
    struct index_avl* fils_gauche;
    struct index_avl* fils_droit;
} IndexAVL;

typedef IndexAVL* pIndexAVL;

// Prototypes des fonctions de gestion AVL

pIndexAVL insererIndex(pIndexAVL racine, const char* identifiant, double taux_fuite, pNoeudDistribution* acteur_out);
pNoeudDistribution rechercherActeur(pIndexAVL racine, const char* identifiant);
void libererAVL_Reseau(pIndexAVL racine);

#endif // AVL_INDEX_H
