#ifndef LEAKS_H
#define LEAKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure du réseau (Arbre n-aire)
typedef struct NoeudReseau {
    char* id;
    double volume_entrant;         
    int nb_enfants;                
    struct NoeudReseau* premier_enfant;
    struct NoeudReseau* frere_suivant;
} NoeudReseau;

// Structure de l'AVL pour l'indexation (Recherche rapide)
typedef struct NoeudAVL {
    char* id;
    NoeudReseau* ptr_noeud;
    int hauteur;
    struct NoeudAVL *fg, *fd;
} NoeudAVL;

// Prototypes
int traiter_leaks(const char* f_in, const char* f_out, const char* id_usine);
void calculer_rendement_aval(NoeudReseau* n, double* total_pertes);
void liberer_reseau(NoeudReseau* n);
void liberer_avl(NoeudAVL* a);

#endif
