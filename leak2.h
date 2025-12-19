#ifndef LEAKS_H
#define LEAKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structures pour le réseau
typedef struct Enfant {
    struct NoeudReseau* noeud;
    double taux_fuite;
    struct Enfant* suivant;
} Enfant;

typedef struct NoeudReseau {
    char* id;
    double volume_entrant;
    int nb_enfants;
    Enfant* liste_fils;
} NoeudReseau;

// Structure pour l'index AVL
typedef struct NoeudAVL {
    char* id;
    NoeudReseau* ptr_noeud;
    struct NoeudAVL *fg, *fd;
    int hauteur;
} NoeudAVL;

// Prototypes
NoeudAVL* inserer_avl(NoeudAVL* node, const char* id, NoeudReseau** resultat);
void ajouter_enfant(NoeudReseau* parent, NoeudReseau* enfant, double fuite);
void calculer_pertes_recursif(NoeudReseau* n, double* total_pertes);
void liberer_avl(NoeudAVL* a);

#endif
