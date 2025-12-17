#ifndef HISTO_H
#define HISTO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define MAX_LINE 4096
#define KILO 1000.0 // Pour la conversion m³ -> k.m³

// STRUCTURE DE L'ARBRE AVL (Usine)

typedef struct arbre_usine {
    char* id;              
    long long vol_max;     
    long long vol_capte;   
    long long vol_traite;  
    int hauteur;           
    struct arbre_usine* fg; 
    struct arbre_usine* fd; 
} Usine;

typedef Usine* pUsine;



// Fonctions AVL
pUsine creerUsine(const char* id, long long vol_max, long long vol_capte, long long vol_traite);
pUsine insertUsine(pUsine racine, pUsine nouvelle_data);
void libererAVL(pUsine racine);
void parcoursInfixeInverse(pUsine racine, FILE* fichier, const char* type);


void traiterLigne(char* ligne, pUsine* racine);

// Fonctions de Conversion
long long parseLongLong(const char* str);
double parseDouble(const char* str);

#endif
