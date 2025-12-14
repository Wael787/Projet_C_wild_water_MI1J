#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

// Taille maximale d'un identifiant (nom d'usine, source, etc.)
#define MAX_ID 256

		// STRUCTURES DE DONNÉES


typedef struct arbre_usine {
    char* id;              // Identifiant unique de l'usine
    long long vol_max;     // Capacité maximale de traitement, milliers de m³.an^-1
    long long vol_capte;   // Volume total capté depuis toutes les sources, milliers de m³
    long long vol_traite;  // Volume total traité (capté - pertes) en milliers de m³
    int hauteur;           
    struct arbre_usine* fg; 
    struct arbre_usine* fd; 
} Usine;

typedef Usine* pUsine;

		// FONCTIONS UTILITAIRES

int max(int a, int b) {
    return (a > b) ? a : b;
}

int hauteur(pUsine u) {
    return (u == NULL) ? 0 : u->hauteur;
}


int getEquilibre(pUsine u) {
    if (u == NULL)
        return 0;
    return hauteur(u->fg) - hauteur(u->fd);
}


void majHauteur(pUsine u) {
    if (u != NULL) {
        u->hauteur = 1 + max(hauteur(u->fg), hauteur(u->fd));
    }
}

		// CRÉATION ET GESTION DES USINES

pUsine creerUsine(const char* id, long long vol_max, long long vol_capte, long long vol_traite) {
    // Allocation de la structure Usine
    pUsine nouvelle = (pUsine)malloc(sizeof(Usine));
    if (!nouvelle) {
        fprintf(stderr, "Erreur: échec d'allocation mémoire pour l'usine\n");
        exit(1);
    }

    nouvelle->id = (char*)malloc(strlen(id) + 1);
    if (!nouvelle->id) {
        fprintf(stderr, "Erreur: échec d'allocation mémoire pour l'identifiant\n");
        free(nouvelle);
        exit(1);
    }
    strcpy(nouvelle->id, id);

    nouvelle->vol_max = vol_max;
    nouvelle->vol_capte = vol_capte;
    nouvelle->vol_traite = vol_traite;
    
    nouvelle->hauteur = 1;
    nouvelle->fg = NULL;  
    nouvelle->fd = NULL;  
    
    return nouvelle;
}

pUsine rotationDroite(pUsine y) {
    pUsine x = y->fg;      
    pUsine T2 = x->fd;     
    
    
    x->fd = y;    
    y->fg = T2;   
    
    // Recalculer les hauteurs (y d'abord car il est maintenant plus bas)
    majHauteur(y);
    majHauteur(x);

    return x;
}

pUsine rotationGauche(pUsine x) {
    pUsine y = x->fd;     
    pUsine T2 = y->fg;     
    
    y->fg = x;    
    x->fd = T2;   
    
    // Recalculer les hauteurs (x d'abord car il est maintenant plus bas)
    majHauteur(x);
    majHauteur(y);
    
    // Retourner la nouvelle racine
    return y;
}



pUsine insertUsine(pUsine racine, pUsine nouvelle_data) {
   
    if (racine == NULL)
        return nouvelle_data;
    
    
    int cmp = strcmp(nouvelle_data->id, racine->id);
    
    if (cmp < 0) {
        racine->fg = insertUsine(racine->fg, nouvelle_data);
    } 
    else if (cmp > 0) {
        racine->fd = insertUsine(racine->fd, nouvelle_data);
    } 
    else {     
        
        // Mise à jour de vol_max seulement si on lit la ligne "USINE" du CSV
        // (reconnaissable par vol_max > 0 dans nouvelle_data)
        if (nouvelle_data->vol_max > 0) 
            racine->vol_max = nouvelle_data->vol_max;
        
        racine->vol_capte += nouvelle_data->vol_capte;
        racine->vol_traite += nouvelle_data->vol_traite;
        
        free(nouvelle_data->id);
        free(nouvelle_data);
        
        return racine;
    }
    
    majHauteur(racine);

    int equilibre = getEquilibre(racine);
    
    // CAS GAUCHE-GAUCHE
    if (equilibre > 1 && strcmp(nouvelle_data->id, racine->fg->id) < 0)
        return rotationDroite(racine);
    
    // CAS DROIT-DROIT
    if (equilibre < -1 && strcmp(nouvelle_data->id, racine->fd->id) > 0)
        return rotationGauche(racine);
    
    // CAS GAUCHE-DROIT
    if (equilibre > 1 && strcmp(nouvelle_data->id, racine->fg->id) > 0) {
        racine->fg = rotationGauche(racine->fg);
        return rotationDroite(racine);
    }
    
    // CAS DROIT-GAUCHE
    if (equilibre < -1 && strcmp(nouvelle_data->id, racine->fd->id) < 0) {
        racine->fd = rotationDroite(racine->fd);
        return rotationGauche(racine);
    }
    

    return racine;
}
