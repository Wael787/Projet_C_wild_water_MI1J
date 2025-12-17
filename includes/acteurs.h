#ifndef ACTEUR_H
#define ACTEUR_H

typedef struct element_enfant ElementEnfant;
typedef struct noeud_distribution NoeudDistribution;


 // Représente un acteur du réseau (Usine, Stockage, Jonction) et ses propriétés.

struct noeud_distribution {
    char* identifiant;
    double taux_fuite;
    double volume_recu;
    ElementEnfant* enfants;
};


 //Maillon de la liste chaînée représentant les destinataires d'un acteur.

struct element_enfant {
    NoeudDistribution* acteur_destination;
    ElementEnfant* suivant;
};


typedef NoeudDistribution* pNoeudDistribution;
typedef ElementEnfant* pElementEnfant;

// Prototypes des fonctions de gestion des acteurs

pNoeudDistribution creerNoeudDistribution(const char* identifiant, double taux_fuite);
void ajouterEnfant(pNoeudDistribution source, pNoeudDistribution destination);
void libererActeur(pNoeudDistribution acteur);
int compterEnfants(pNoeudDistribution parent);

#endif
