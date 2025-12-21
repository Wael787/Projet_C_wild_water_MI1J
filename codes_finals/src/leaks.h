#ifndef LEAKS_H
#define LEAKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LONGUEUR_LIGNE 1024
#define MAX_LONGUEUR_ID 256

// Structure pour un nœud de l'arbre de distribution 
typedef struct Noeud {
    char *identifiant;              
    double volume;                 
    double pourcentage_fuite;        
    struct Noeud **enfants;        
    int nb_enfants;              
    int capacite_enfants;         
} Noeud;

// Structure pour un nœud AVL 
typedef struct NoeudAVL {
    char *identifiant;
    Noeud *noeud_donnees;
    struct NoeudAVL *gauche;
    struct NoeudAVL *droit;
    int hauteur;
} NoeudAVL;

// Gestion de l'arbre de distribution 
Noeud *creerNoeud(const char *identifiant, double pourcentage_fuite);
void ajouterEnfant(Noeud *parent, Noeud *enfant);
void libererArbre(Noeud *racine);

// Gestion de l'AVL 
NoeudAVL *creerNoeudAVL(const char *identifiant, Noeud *noeud_donnees);
int hauteurAVL(NoeudAVL *noeud);
int maxAVL(int a, int b);
int facteurEquilibreAVL(NoeudAVL *noeud);
NoeudAVL *rotationDroiteAVL(NoeudAVL *y);
NoeudAVL *rotationGaucheAVL(NoeudAVL *x);
NoeudAVL *insererAVL(NoeudAVL *racine, const char *identifiant, Noeud *noeud_donnees);
Noeud *rechercherAVL(NoeudAVL *racine, const char *identifiant);
void libererAVL(NoeudAVL *racine);

// Calcul des volumes et fuites 
void calculerVolumes(Noeud *noeud, double volume_parent, int nb_freres);
double calculerFuitesTotales(Noeud *noeud);

// Fonctions utilitaires 
int analyserLigneCSV(char *ligne, char *col1, char *col2, char *col3, char *col4, char *col5);
char *nettoyerEspaces(char *str);
char *dupliquerChaine(const char *str);

#endif
