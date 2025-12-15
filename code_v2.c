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

pUsine rechercherUsine(pUsine racine, const char* id) {
    // Cas de base : arbre vide ou fin de branche atteinte
    if (racine == NULL)
        return NULL;  // Usine non trouvée
    
    // Comparer l'identifiant recherché avec le nœud actuel
    int cmp = strcmp(id, racine->id);
    
    if (cmp == 0)
        return racine;  // Trouvé ! Retourner le pointeur
    else if (cmp < 0)
        return rechercherUsine(racine->fg, id);  // Chercher à gauche
    else
        return rechercherUsine(racine->fd, id);  // Chercher à droite
}


void parcoursInfixeInverse(pUsine racine, FILE* fichier, const char* type) {
    // Cas de base : nœud vide
    if (racine == NULL)
        return;
    
    // ÉTAPE 1 : Parcourir d'abord le sous-arbre DROIT (pour l'ordre inverse)
    parcoursInfixeInverse(racine->fd, fichier, type);
    
    // ÉTAPE 2 : Traiter le nœud actuel (écrire les données dans le fichier)
    if (strcmp(type, "max") == 0) {
        // Écrire la capacité maximale
        fprintf(fichier, "%s;%lld\n", racine->id, racine->vol_max);
    } 
    else if (strcmp(type, "src") == 0) {
        // Écrire le volume capté
        fprintf(fichier, "%s;%lld\n", racine->id, racine->vol_capte);
    } 
    else if (strcmp(type, "real") == 0) {
        // Écrire le volume traité
        fprintf(fichier, "%s;%lld\n", racine->id, racine->vol_traite);
    }
    
    // ÉTAPE 3 : Parcourir ensuite le sous-arbre GAUCHE
    parcoursInfixeInverse(racine->fg, fichier, type);
}


void libererAVL(pUsine racine) {
    // Cas de base : nœud vide, rien à libérer
    if (racine == NULL)
        return;
    
    // Libérer récursivement les sous-arbres
    libererAVL(racine->fg);
    libererAVL(racine->fd);
    
    // Libérer l'identifiant (chaîne de caractères allouée dynamiquement)
    free(racine->id);
    
    // Libérer la structure elle-même
    free(racine);
}

// ========== FONCTIONS DE CONVERSION ROBUSTES ==========

/**
 * Convertit une chaîne en long long avec gestion d'erreurs
 * 
 * Paramètres :
 * - str : chaîne à convertir
 * 
 * Retourne :
 * - La valeur convertie en cas de succès
 * - 0 si la chaîne est "-" (valeur manquante dans le CSV)
 * - 0 en cas d'erreur de conversion (avec message d'avertissement)
 */
long long parseLongLong(const char* str) {
    // Cas spécial : valeur manquante dans le CSV
    if (strcmp(str, "-") == 0) 
        return 0;
    
    // Conversion avec strtoll (plus robuste que atoll)
    char* endptr;
    errno = 0;
    long long val = strtoll(str, &endptr, 10);
    
    // Vérification des erreurs
    // errno == ERANGE : débordement (nombre trop grand)
    // endptr == str : aucun chiffre n'a été lu
    // *endptr != '\0' : des caractères invalides suivent le nombre
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        fprintf(stderr, "Avertissement: Erreur de conversion pour long long: %s\n", str);
        return 0;
    }
    
    return val;
}


double parseDouble(const char* str) {
    // Cas spécial : valeur manquante
    if (strcmp(str, "-") == 0) 
        return 0.0;
    
    // Conversion avec strtod (plus robuste que atof)
    char* endptr;
    errno = 0;
    double val = strtod(str, &endptr);
    
    // Vérification des erreurs
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        fprintf(stderr, "Avertissement: Erreur de conversion pour double: %s\n", str);
        return 0.0;
    }
    
    return val;
}



// ========== FONCTION PRINCIPALE ==========

int main(int argc, char* argv[]) {
    
    
    if (argc < 3) {
        fprintf(stderr, "Erreur: Usage: %s <fichier_csv> histo <type: max|src|real>\n", argv[0]);
        return 1;  // Code d'erreur (la consigne demande un code > 0 en cas d'erreur)
    }
    
    // Vérifier que l'argument 2 est bien "histo"
    if (strcmp(argv[2], "histo") != 0) {
        fprintf(stderr, "Erreur: Ce programme ne supporte que le traitement 'histo'.\n");
        return 1;
    }
    
    // Vérifier qu'il y a un argument 3 (type)
    if (argc < 4) {
        fprintf(stderr, "Erreur: Argument manquant. Type doit être 'max', 'src' ou 'real'.\n");
        return 1;
    }
    
    // Récupérer les arguments
    char* fichier_entree = argv[1];     // Chemin du fichier CSV
    char* type_traitement = argv[3];    // Type : max, src, ou real
    
    
    if (strcmp(type_traitement, "max") != 0 && 
        strcmp(type_traitement, "src") != 0 && 
        strcmp(type_traitement, "real") != 0) {
        fprintf(stderr, "Erreur: type doit être 'max', 'src' ou 'real'\n");
        return 1;
    }
    
  
    FILE* f_in = fopen(fichier_entree, "r");
    if (!f_in) {
        fprintf(stderr, "Erreur: impossible d'ouvrir le fichier %s\n", fichier_entree);
        return 1;
    }
    
   
    pUsine racine = NULL; 
    char ligne[MAX_LINE];
    
    // Lire le fichier ligne par ligne
    while (fgets(ligne, MAX_LINE, f_in) != NULL) {
        // Supprimer le retour à la ligne '\n' à la fin
        ligne[strcspn(ligne, "\n")] = '\0';
        
        // Traiter la ligne et mettre à jour l'AVL
        traiterLigne(ligne, &racine);
    }
    
    // Fermer le fichier d'entrée
    fclose(f_in);
    
   
    char nom_fichier_sortie[256];
    char* nom_base = NULL;
    char* en_tete_volume = NULL;
    
    // Déterminer le nom du fichier et l'en-tête selon le type
    if (strcmp(type_traitement, "max") == 0) {
        nom_base = "vol_max";
        en_tete_volume = "max volume (k.m3.year-1)";
    } 
    else if (strcmp(type_traitement, "src") == 0) {
        nom_base = "vol_captation";
        en_tete_volume = "source volume (k.m3.year-1)";
    } 
    else { // real
        nom_base = "vol_traitement";
        en_tete_volume = "real volume (k.m3.year-1)";
    }
    
    // Créer le nom complet du fichier
    sprintf(nom_fichier_sortie, "%s.dat", nom_base);
    
    // Ouvrir le fichier de sortie en écriture
    FILE* f_out = fopen(nom_fichier_sortie, "w");
    if (!f_out) {
        fprintf(stderr, "Erreur: impossible de créer le fichier %s\n", nom_fichier_sortie);
        libererAVL(racine);
        return 1;
    }
    
    
    fprintf(f_out, "identifier;%s\n", en_tete_volume);
    
    
    parcoursInfixeInverse(racine, f_out, type_traitement);
    
    
    fclose(f_out);
    
    
    libererAVL(racine);
    
    
    return 0;
}
