#ifndef HISTO_H
#define HISTO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 2048
#define MAX_ENTRIES 1000

// Structure pour stocker les données
typedef struct {
    char nom[300];
    double volume;
} Donnee;

// Variables globales
extern Donnee tab[MAX_ENTRIES];
extern int nb;

// Fonctions utilitaires
void nettoyer(char* s);
int chercher(char* nom);
void ajouter(char* nom, double vol);
int comparer(const void* a, const void* b);

// Fonctions de traitement (retournent 0 si succès, 1 si erreur)
int mode_max(char* fichier_entree, char* fichier_sortie);
int mode_src(char* fichier_entree, char* fichier_sortie);
int mode_reel(char* fichier_entree, char* fichier_sortie);

#endif
