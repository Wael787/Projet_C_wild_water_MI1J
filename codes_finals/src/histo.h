#ifndef HISTO_H
#define HISTO_H

// verison finale
// date 21/12/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Constantes de dimensionnement */
#define MAX_LIGNE 2048
#define MAX_NOEUDS 10000

/* Nœud d'arbre AVL pour stocker identifiants et volumes */
typedef struct Noeud {
    char* id;
    double volume;
    int hauteur;
    struct Noeud* gauche;
    struct Noeud* droit;
} Noeud;

typedef Noeud* pNoeud;

/* Élément extrait de l'AVL pour tri et export */
typedef struct {
    char id[256];
    double volume;
} Element;

/* GESTION AVL */

int hauteur(pNoeud n);
int max(int a, int b);
int equilibre(pNoeud n);

pNoeud creerNoeud(const char* id, double vol);
void libererAVL(pNoeud r);

pNoeud rotationDroite(pNoeud y);
pNoeud rotationGauche(pNoeud x);

pNoeud insererAVL(pNoeud racine, const char* id, double vol);

void convertirAVL(pNoeud r, Element* tab, int* idx);

/* TRI ET ANALYSE */

int comparerDecroissant(const void* a, const void* b);
int comparerId(const void* a, const void* b);

void nettoyer(char* str);
int estTiret(const char* s);
int nonTiret(const char* s);

int analyserLigne(char* ligne, char* champs[], int max_champs);

/* TRAITEMENT DES MODES */

void traiterMax(const char* fichier_donnees, const char* fichier_csv);
void traiterSource(const char* fichier_donnees, const char* fichier_csv);
void traiterReel(const char* fichier_donnees, const char* fichier_csv);

#endif /* HISTO_H */
