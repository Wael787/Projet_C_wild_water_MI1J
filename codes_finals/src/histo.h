#ifndef HISTO_H
#define HISTO_H

// version finale
// date : 21/12/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Tailles max pour les buffers
#define MAX_LIGNE 2048
#define MAX_NOEUDS 10000

// Structure pour l'arbre AVL
// On stocke l'identifiant et le volume cumulé
typedef struct Noeud {
    char* id;
    double volume;
    int hauteur;  
    struct Noeud* gauche;
    struct Noeud* droit;
} Noeud;

typedef Noeud* pNoeud;

// Pour recuperer les données de l'AVL et les trier
typedef struct {
    char id[256];
    double volume;
} Element;

// Fonctions AVL
// Gestion de base de l'arbre
int hauteur(pNoeud n);
int max(int a, int b);
int equilibre(pNoeud n);
pNoeud creerNoeud(const char* id, double vol);
void libererAVL(pNoeud r);

// Rotations pour réequilibrer
pNoeud rotationDroite(pNoeud y);
pNoeud rotationGauche(pNoeud x);

// Insertion avec cumul des volumes si l'ID existe deja
pNoeud insererAVL(pNoeud racine, const char* id, double vol);

// Convertit l'AVL en tableau pour le tri
void convertirAVL(pNoeud r, Element* tab, int* idx);

// Utilitaires
// Fonctions de comparaison pour qsort
int comparerDecroissant(const void* a, const void* b);
int comparerId(const void* a, const void* b);

// Parser le fichier CSV
void nettoyer(char* str);
int estTiret(const char* s);
int nonTiret(const char* s);
int analyserLigne(char* ligne, char* champs[], int max_champs);

// Traitements
// Une fonction par mode demandé
void traiterMax(const char* fichier_donnees, const char* fichier_csv);
void traiterSource(const char* fichier_donnees, const char* fichier_csv);
void traiterReel(const char* fichier_donnees, const char* fichier_csv);

#endif
