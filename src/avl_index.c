#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "avl_index.h"  
#include "acteur.h"     

#define EPSILON 1e-9

// Fonctions utilitaires AVL (déclarations internes avec static)

static int max(int a, int b) { 
  return (a > b) ? a : b; 
}
static int hauteur(pIndexAVL u) { 
  return (u == NULL) ? 0 : u->hauteur; 
}
static void majHauteur(pIndexAVL u) { 
  if (u != NULL) { 
      u->hauteur = 1 + max(hauteur(u->fils_gauche), hauteur(u->fils_droit)); 
  } 
}
static int obtenirEquilibre(pIndexAVL u) { 
  return (u == NULL) ? 0 : hauteur(u->fils_gauche) - hauteur(u->fils_droit); 
}
static pIndexAVL rotationDroite(pIndexAVL y);
static pIndexAVL rotationGauche(pIndexAVL x);


static pIndexAVL rotationDroite(pIndexAVL y) {
    pIndexAVL x = y->fils_gauche;
    pIndexAVL T2 = x->fils_droit;
    
    x->fils_droit = y;
    y->fils_gauche = T2;
    
    majHauteur(y);
    majHauteur(x);

    return x;
}

static pIndexAVL rotationGauche(pIndexAVL x) {
    pIndexAVL y = x->fils_droit;
    pIndexAVL T2 = y->fils_gauche;
    
    y->fils_gauche = x;
    x->fils_droit = T2;
    
    majHauteur(x);
    majHauteur(y);
    
    return y;
}

// Insère un acteur dans l'Index AVL, le crée si nécessaire.
 
pIndexAVL insererIndex(pIndexAVL racine, const char* identifiant, double taux_fuite, pNoeudDistribution* acteur_out) {
    pNoeudDistribution acteur;

    if (racine == NULL) {
        acteur = creerNoeudDistribution(identifiant, taux_fuite);
        
        racine = (pIndexAVL)malloc(sizeof(IndexAVL));
        if (!racine){
          exit(ERR_TRAITEMENT);
        }
        
        racine->identifiant = strdup(identifiant);
        racine->noeud_reseau = acteur;
        racine->hauteur = 1;
        racine->fils_gauche = NULL;
        racine->fils_droit = NULL;
        *acteur_out = acteur;
        return racine;
    }
    
    int cmp = strcmp(identifiant, racine->identifiant);
    
    if (cmp < 0) {
        racine->fils_gauche = insererIndex(racine->fils_gauche, identifiant, taux_fuite, acteur_out);
    }
    else if (cmp > 0) {
        racine->fils_droit = insererIndex(racine->fils_droit, identifiant, taux_fuite, acteur_out);
    }
    else { 
        if (taux_fuite > EPSILON) {
            racine->noeud_reseau->taux_fuite = taux_fuite;
        }
        *acteur_out = racine->noeud_reseau;
        return racine;
    }
    
    majHauteur(racine);

    int equilibre = obtenirEquilibre(racine);
    
    // Application des rotations AVL
    if (equilibre > 1 && strcmp(identifiant, racine->fils_gauche->identifiant) < 0){
      return rotationDroite(racine);
    }
    if (equilibre < -1 && strcmp(identifiant, racine->fils_droit->identifiant) > 0){ 
      return rotationGauche(racine);
    }
    if (equilibre > 1 && strcmp(identifiant, racine->fils_gauche->identifiant) > 0) {
        racine->fils_gauche = rotationGauche(racine->fils_gauche);
        return rotationDroite(racine);
    }
    if (equilibre < -1 && strcmp(identifiant, racine->fils_droit->identifiant) < 0) {
        racine->fils_droit = rotationDroite(racine->fils_droit);
        return rotationGauche(racine);
    }

    return racine;
}

// Recherche un NoeudDistribution dans l'Index AVL.
 
pNoeudDistribution rechercherActeur(pIndexAVL racine, const char* identifiant) {
    if (racine == NULL){
        return NULL;
    }
    int cmp = strcmp(identifiant, racine->identifiant);
    
    if (cmp == 0){
        return racine->noeud_reseau;
    }
    else if (cmp < 0){
        return rechercherActeur(racine->fils_gauche, identifiant);
    }
    else{
        return rechercherActeur(racine->fils_droit, identifiant);
    }
}

// Parcourt l'Index AVL pour libérer tous les NoeudDistribution et l'Index lui-même.
 
void libererAVL_Reseau(pIndexAVL racine) {
    if (racine == NULL){
      return;
    }

    libererAVL_Reseau(racine->fils_gauche);
    libererAVL_Reseau(racine->fils_droit);
    
    // 1. Libérer le NoeudDistribution 
    libererActeur(racine->noeud_reseau);
    
    // 2. Libérer le maillon AVL
    free(racine->identifiant);
    free(racine);
}
