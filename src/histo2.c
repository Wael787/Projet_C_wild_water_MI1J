#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "histo.h"
#include "codes_erreur.h"

#define MAX_ID 256
#define MAX_LINE 4096 

// Structure pour l'AVL des usines
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

// Fonctions utilitaires

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static int hauteur(pUsine u) {
    return (u == NULL) ? 0 : u->hauteur;
}

static int getEquilibre(pUsine u) {
    if (u == NULL) return 0;
    return hauteur(u->fg) - hauteur(u->fd);
}

static void majHauteur(pUsine u) {
    if (u != NULL) {
        u->hauteur = 1 + max(hauteur(u->fg), hauteur(u->fd));
    }
}

// Création d'une usine
static pUsine creerUsine(const char* id, long long vol_max, long long vol_capte, long long vol_traite) {
    pUsine nouvelle = (pUsine)malloc(sizeof(Usine));
    if (!nouvelle) {
        fprintf(stderr, "Erreur C: échec d'allocation mémoire pour l'usine\n");
        exit(ERR_TRAITEMENT);
    }

    nouvelle->id = (char*)malloc(strlen(id) + 1);
    if (!nouvelle->id) {
        fprintf(stderr, "Erreur C: échec d'allocation mémoire pour l'identifiant\n");
        free(nouvelle);
        exit(ERR_TRAITEMENT);
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

// Rotations AVL

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

// Insertion dans l'AVL
static pUsine insertUsine(pUsine racine, pUsine nouvelle_data) {
   
    if (racine == NULL) {
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
        // Nœud existant, mise à jour
        if (nouvelle_data->vol_max > 0) {
            racine->vol_max = nouvelle_data->vol_max;
        }
        racine->vol_capte += nouvelle_data->vol_capte;
        racine->vol_traite += nouvelle_data->vol_traite;
        
        free(nouvelle_data->id);
        free(nouvelle_data);
        
        return racine;
    }
    
    majHauteur(racine);

    int equilibre = getEquilibre(racine);
    
    if (equilibre > 1 && strcmp(nouvelle_data->id, racine->fg->id) < 0) {
        return rotationDroite(racine);
    }
    
    if (equilibre < -1 && strcmp(nouvelle_data->id, racine->fd->id) > 0) {
        return rotationGauche(racine);
    }
    
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

// Parcours infixe inverse (décroissant) pour affichage
static void parcoursInfixeInverse(pUsine racine, FILE* fichier, const char* type) {
    if (racine == NULL) return;
    
    parcoursInfixeInverse(racine->fd, fichier, type);
    
    if (strcmp(type, "max") == 0) {
        fprintf(fichier, "%s;%lld\n", racine->id, racine->vol_max);
    } 
    else if (strcmp(type, "src") == 0) {
        fprintf(fichier, "%s;%lld\n", racine->id, racine->vol_capte);
    } 
    else if (strcmp(type, "real") == 0) {
        fprintf(fichier, "%s;%lld\n", racine->id, racine->vol_traite);
    }
    
    parcoursInfixeInverse(racine->fg, fichier, type);
}

// Libération de l'AVL
static void libererAVL(pUsine racine) {
    if (racine == NULL) return;

    libererAVL(racine->fg);
    libererAVL(racine->fd);
    
    free(racine->id);
    free(racine);
}

// Fonctions de parsing

static long long parseLongLong(const char* str) {
    if (strcmp(str, "-") == 0) return 0;
    
    char* endptr;
    errno = 0;
    long long val = strtoll(str, &endptr, 10);
    
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        return 0;
    }
    
    return val;
}

static double parseDouble(const char* str) {
    if (strcmp(str, "-") == 0) return 0.0;
    
    char* endptr;
    errno = 0;
    double val = strtod(str, &endptr);
  
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        return 0.0;
    }
    
    return val;
}

// Traitement d'une ligne du fichier
static void traiterLigne(char* ligne, pUsine* racine) {
    char *jeton;
    char *sauvegarde_ptr = NULL;

    char ligne_tampon[MAX_LINE];
    if (strlen(ligne) >= MAX_LINE) {
        return; 
    }
    strcpy(ligne_tampon, ligne);
    
    char id[MAX_ID] = {0};
    long long vol1 = 0;
    long long vol2 = 0;
    double fuite = 0.0;

    // Découpage de la ligne
    jeton = strtok_r(ligne_tampon, ";", &sauvegarde_ptr); 

    jeton = strtok_r(NULL, ";", &sauvegarde_ptr);
    if (!jeton) return; 
    char* col2 = strdup(jeton);
    
    jeton = strtok_r(NULL, ";", &sauvegarde_ptr);
    if (!jeton) { free(col2); return; }
    char* col3 = strdup(jeton);

    jeton = strtok_r(NULL, ";", &sauvegarde_ptr);
    if (!jeton) { free(col2); free(col3); return; }
    char* col4 = strdup(jeton);

    jeton = strtok_r(NULL, ";", &sauvegarde_ptr);
    if (!jeton) { free(col2); free(col3); free(col4); return; }
    char* col5 = strdup(jeton);

    // Traitement selon le type de ligne
    if (strcmp(col3, "-") == 0 && strcmp(col5, "-") == 0) {
        // Ligne USINE
        strncpy(id, col2, MAX_ID - 1);
        vol1 = parseLongLong(col4);
    } 
    else if (strcmp(col3, "-") != 0) {
        // Ligne SOURCE -> USINE
        strncpy(id, col3, MAX_ID - 1);
        vol1 = parseLongLong(col4);
        fuite = parseDouble(col5) / 100.0;
        vol2 = (long long)round((double)vol1 * (1.0 - fuite));
    } 
    else {
        free(col2); free(col3); free(col4); free(col5);
        return;
    }

    free(col2); free(col3); free(col4); free(col5);

    pUsine nouvelle = creerUsine(id, vol1, vol1, vol2);
    *racine = insertUsine(*racine, nouvelle);
}

// Fonction principale de traitement histo
int traiter_histo(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Erreur C (histo): Arguments invalides (attendu: 5, reçu: %d).\n", argc);
        return ERR_USAGE_C;
    }

    const char *type_traitement = argv[2];      
    const char *fichier_entree = argv[3];       
    const char *fichier_sortie = argv[4];       
    
    // Validation du type de traitement
    if (strcmp(type_traitement, "max") != 0 && 
        strcmp(type_traitement, "src") != 0 && 
        strcmp(type_traitement, "real") != 0) {
        fprintf(stderr, "Erreur C: Type de traitement invalide '%s' (attendu: max, src ou real).\n", type_traitement);
        return ERR_USAGE_C;
    }
    
    FILE* f_in = fopen(fichier_entree, "r");
    if (!f_in) {
        fprintf(stderr, "Erreur C: impossible d'ouvrir le fichier d'entrée %s\n", fichier_entree);
        return ERR_FICHIER_IN;
    }
    
    pUsine racine = NULL; 
    char ligne[MAX_LINE];
    
    while (fgets(ligne, MAX_LINE, f_in) != NULL) {
        ligne[strcspn(ligne, "\n")] = '\0';
        if (ligne[0] != '\0') {
            traiterLigne(ligne, &racine);
        }
    }
    fclose(f_in);
    
    if (racine == NULL) {
         fprintf(stderr, "Erreur C: Aucune donnée d'usine trouvée dans le fichier filtré.\n");
         return ERR_TRAITEMENT;
    }
    
    FILE* f_out = fopen(fichier_sortie, "w");
    if (!f_out) {
        fprintf(stderr, "Erreur C: impossible de créer le fichier de sortie %s\n", fichier_sortie);
        libererAVL(racine);
        return ERR_FICHIER_OUT;
    }

    // En-tête selon le type
    char* en_tete_volume = NULL;
    if (strcmp(type_traitement, "max") == 0) {
        en_tete_volume = "max volume (k.m3.year-1)";
    } 
    else if (strcmp(type_traitement, "src") == 0) {
        en_tete_volume = "source volume (k.m3.year-1)";
    } 
    else {
        en_tete_volume = "real volume (k.m3.year-1)";
    }

    fprintf(f_out, "identifier;%s\n", en_tete_volume);
    parcoursInfixeInverse(racine, f_out, type_traitement);
    
    fclose(f_out);
    libererAVL(racine);
    
    return 0;
}
