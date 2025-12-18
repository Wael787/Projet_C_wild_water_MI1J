#include "histo.h"
#include <sys/stat.h> // avertissement d'écrasement

// FONCTIONS UTILITAIRES AVL INTERNES

int max(int a, int b) { 
  return (a > b) ? a : b; 
}
int hauteur(pUsine u) { 
  return (u == NULL) ? 0 : u->hauteur; 
}
int getEquilibre(pUsine u) { 
    if (u == NULL) return 0; 
    return hauteur(u->fg) - hauteur(u->fd); 
}
void majHauteur(pUsine u) { 
    if (u != NULL) { 
        u->hauteur = 1 + max(hauteur(u->fg), hauteur(u->fd)); 
    } 
}

static pUsine rotationDroite(pUsine y) {
    pUsine x = y->fg;      
    pUsine T2 = x->fd;      
    x->fd = y;    
    y->fg = T2;    
    majHauteur(y);
    majHauteur(x);
    return x;
}

static pUsine rotationGauche(pUsine x) {
    pUsine y = x->fd;      
    pUsine T2 = y->fg;      
    y->fg = x;    
    x->fd = T2;    
    majHauteur(x);
    majHauteur(y);
    return y;
}


// CRÉATION ET GESTION DES USINES 

pUsine creerUsine(const char* id, long long vol_capte, long long vol_traite) {
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

    nouvelle->vol_capte = vol_capte;
    nouvelle->vol_traite = vol_traite;

    nouvelle->hauteur = 1;
    nouvelle->fg = NULL;  
    nouvelle->fd = NULL;  
    return nouvelle;
}

pUsine insertUsine(pUsine racine, pUsine nouvelle_data) {
    
    if (racine == NULL){ 
      return nouvelle_data;
    }
    
    int cmp = strcmp(nouvelle_data->id, racine->id);
    
    if (cmp < 0) {
        racine->fg = insertUsine(racine->fg, nouvelle_data);
    } 
    else if (cmp > 0) {
        racine->fd = insertUsine(racine->fd, nouvelle_data);
    } 
    else {      
        racine->vol_capte += nouvelle_data->vol_capte;
        racine->vol_traite += nouvelle_data->vol_traite;
        
        free(nouvelle_data->id);
        free(nouvelle_data);
        return racine;
    }
    
    majHauteur(racine);
    int equilibre = getEquilibre(racine);
    
    if (equilibre > 1 && strcmp(nouvelle_data->id, racine->fg->id) < 0)
        return rotationDroite(racine);
    
    if (equilibre < -1 && strcmp(nouvelle_data->id, racine->fd->id) > 0)
        return rotationGauche(racine);
    
    if (equilibre > 1 && strcmp(nouvelle_data->id, racine->fg->id) > 0) {
        racine->fg = rotationGauche(racine->fg);
        return rotationDroite(racine);
    }

    if (equilibre < -1 && strcmp(nouvelle_data->id, racine->fd->id) < 0) {
        racine->fd = rotationDroite(racine->fd);
        return rotationGauche(racine);
    }
    
    return racine;
}

void libererAVL(pUsine racine) {
    if (racine == NULL) return;
    libererAVL(racine->fg);
    libererAVL(racine->fd);
    free(racine->id);
    free(racine);
}

// FONCTIONS DE CONVERSION ROBUSTES

long long parseLongLong(const char* str) {
    if (strcmp(str, "-") == 0) return 0;
    char* endptr;
    errno = 0;
    long long val = strtoll(str, &endptr, 10);
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        fprintf(stderr, "Avertissement: Erreur de conversion pour long long: %s\n", str);
        return 0;
    }
    return val;
}

double parseDouble(const char* str) {
    if (strcmp(str, "-") == 0){ 
      return 0.0;
    }
    char* endptr;
    errno = 0;
    double val = strtod(str, &endptr);
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        fprintf(stderr, "Avertissement: Erreur de conversion pour double: %s\n", str);
        return 0.0;
    }
    return val;
}

// LOGIQUE MÉTIER PRINCIPALE

void traiterLigne(char* ligne, pUsine* racine_ptr) {
  
    char* ligne_copie = strdup(ligne);
    if (!ligne_copie) {
      return;
    }

    char* ptr = ligne_copie;
    
    // Parsing des 5 champs attendus (Type;Source_ID;Dest_ID;Volume;Taux_Fuite)
    char *type = strtok_r(ptr, ";", &ptr);
    char *source_id = strtok_r(NULL, ";", &ptr);
    char *dest_id = strtok_r(NULL, ";", &ptr);
    char *volume_str = strtok_r(NULL, ";", &ptr);
    char *fuite_str = strtok_r(NULL, ";", &ptr);

    if (!type || !source_id || !dest_id || !volume_str || !fuite_str) {
        free(ligne_copie);
        return;
    }

    long long vol_initial = parseLongLong(volume_str);
    double fuite_pct = parseDouble(fuite_str);          
    double fuite_decimale = fuite_pct / 100.0;
    
    // On traite uniquement les lignes qui représentent un FLUX (avec une destination non vide)
    if (strcmp(dest_id, "-") != 0 && vol_initial > 0) { 
        const char* usine_id = dest_id;

        // 1. volume caté pour le mode "src"
      
        long long vol_capte_ll = vol_initial; 

        // 2. volume réellement traité pour l mode "real"
      
        double vol_traite_double = (double)vol_initial * (1.0 - fuite_decimale);
        long long vol_traite_ll = (long long)round(vol_traite_double); 

        pUsine nouvelle = creerUsine(usine_id, vol_capte_ll, vol_traite_ll); 
        
        *racine_ptr = insertUsine(*racine_ptr, nouvelle);
    }
    
    free(ligne_copie);
}

  // ordre aplphabétique inverse, doit donc partir du sous-arbre droit
void parcoursInfixeInverse(pUsine racine, FILE* fichier, const char* type) {
    if (racine == NULL){
      return;
    }
    
    parcoursInfixeInverse(racine->fd, fichier, type);
    
    long long valeur = 0;

    if (strcmp(type, "src") == 0) {
        valeur = racine->vol_capte;
    } 
    else if (strcmp(type, "real") == 0) {
        valeur = racine->vol_traite;
    } 

    // Écriture : conversion m³ en k.m³ et vérification que la valeur est pertinente
    if (valeur > 0) {
         fprintf(fichier, "%s;%lld\n", racine->id, valeur / KILO);
    }
    
    parcoursInfixeInverse(racine->fg, fichier, type);
}
