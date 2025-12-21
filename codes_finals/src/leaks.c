#include "leaks.h"
// version finale
// date : 21/12/2025

// Gestion de l'arbre de distribution ( tout ce qui est stockages , jonctions etc ... )

// Cée nouveau noeud pour arbre de distribution
Noeud *creerNoeud(const char *identifiant, double pourcentage_fuite) {
    Noeud *noeud = (Noeud *)malloc(sizeof(Noeud));
    if (!noeud) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour le nœud\n");
        return NULL;
    }
    
    noeud->identifiant = dupliquerChaine(identifiant);
    if (!noeud->identifiant) {
        free(noeud);
        return NULL;
    }
    
    noeud->volume = 0.0;
    noeud->pourcentage_fuite = pourcentage_fuite;
    noeud->enfants = NULL;
    noeud->nb_enfants = 0;
    noeud->capacite_enfants = 0;
    
    return noeud;
}

/*Ajoute un enfant au noeud parent
Utilise un tableau dynamique qui s'agrandit automatiquement si nécessaire
*/
void ajouterEnfant(Noeud *parent, Noeud *enfant) {
    if (!parent || !enfant) return;

    // Agrandissement du tableau si plein (doublement de capacité)
    if (parent->nb_enfants >= parent->capacite_enfants) {
        int nouvelle_capacite = parent->capacite_enfants == 0 ? 4 : parent->capacite_enfants * 2;
        Noeud **nouveaux_enfants = (Noeud **)realloc(parent->enfants, nouvelle_capacite * sizeof(Noeud *));
        if (!nouveaux_enfants) {
            fprintf(stderr, "Erreur: Réallocation mémoire échouée\n");
            return;
        }
        parent->enfants = nouveaux_enfants;
        parent->capacite_enfants = nouvelle_capacite;
    }
    
    parent->enfants[parent->nb_enfants] = enfant;
    parent->nb_enfants++;
}

//Liberation récursivement de tout l'arbre
void libererArbre(Noeud *racine) {
    if (!racine) return;
    
    for (int i = 0; i < racine->nb_enfants; i++) {
        libererArbre(racine->enfants[i]);
    }
    
    free(racine->identifiant);
    free(racine->enfants);
    free(racine);
}

// Fonction de gestion de l'AVL

NoeudAVL *creerNoeudAVL(const char *identifiant, Noeud *noeud_donnees) {
    NoeudAVL *noeud = (NoeudAVL *)malloc(sizeof(NoeudAVL));
    if (!noeud) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour le nœud AVL\n");
        return NULL;
    }
    
    noeud->identifiant = dupliquerChaine(identifiant);
    if (!noeud->identifiant) {
        free(noeud);
        return NULL;
    }
    
    noeud->noeud_donnees = noeud_donnees;
    noeud->gauche = NULL;
    noeud->droit = NULL;
    noeud->hauteur = 1;
    
    return noeud;
}

int hauteurAVL(NoeudAVL *noeud) {
    return noeud ? noeud->hauteur : 0;
}

int maxAVL(int a, int b) {
    return (a > b) ? a : b;
}

int facteurEquilibreAVL(NoeudAVL *noeud) {
    return noeud ? hauteurAVL(noeud->gauche) - hauteurAVL(noeud->droit) : 0;
}

NoeudAVL *rotationDroiteAVL(NoeudAVL *y) {
    NoeudAVL *x = y->gauche;
    NoeudAVL *T2 = x->droit;
    
    x->droit = y;
    y->gauche = T2;
    
    y->hauteur = maxAVL(hauteurAVL(y->gauche), hauteurAVL(y->droit)) + 1;
    x->hauteur = maxAVL(hauteurAVL(x->gauche), hauteurAVL(x->droit)) + 1;
    
    return x;
}

NoeudAVL *rotationGaucheAVL(NoeudAVL *x) {
    NoeudAVL *y = x->droit;
    NoeudAVL *T2 = y->gauche;
    
    y->gauche = x;
    x->droit = T2;
    
    x->hauteur = maxAVL(hauteurAVL(x->gauche), hauteurAVL(x->droit)) + 1;
    y->hauteur = maxAVL(hauteurAVL(y->gauche), hauteurAVL(y->droit)) + 1;
    
    return y;
}

NoeudAVL *insererAVL(NoeudAVL *racine, const char *identifiant, Noeud *noeud_donnees) {
    if (!racine) {
        return creerNoeudAVL(identifiant, noeud_donnees);
    }
    
    int cmp = strcmp(identifiant, racine->identifiant);
    if (cmp < 0) {
        racine->gauche = insererAVL(racine->gauche, identifiant, noeud_donnees);
    } else if (cmp > 0) {
        racine->droit = insererAVL(racine->droit, identifiant, noeud_donnees);
    } else {
        return racine;
    }
    
    racine->hauteur = 1 + maxAVL(hauteurAVL(racine->gauche), hauteurAVL(racine->droit));
    
    int equilibre = facteurEquilibreAVL(racine);
    
    if (equilibre > 1 && strcmp(identifiant, racine->gauche->identifiant) < 0) {
        return rotationDroiteAVL(racine);
    }
    
    if (equilibre < -1 && strcmp(identifiant, racine->droit->identifiant) > 0) {
        return rotationGaucheAVL(racine);
    }
    
    if (equilibre > 1 && strcmp(identifiant, racine->gauche->identifiant) > 0) {
        racine->gauche = rotationGaucheAVL(racine->gauche);
        return rotationDroiteAVL(racine);
    }
    
    if (equilibre < -1 && strcmp(identifiant, racine->droit->identifiant) < 0) {
        racine->droit = rotationDroiteAVL(racine->droit);
        return rotationGaucheAVL(racine);
    }
    
    return racine;
}

Noeud *rechercherAVL(NoeudAVL *racine, const char *identifiant) {
    if (!racine) return NULL;
    
    int cmp = strcmp(identifiant, racine->identifiant);
    if (cmp == 0) {
        return racine->noeud_donnees;
    } else if (cmp < 0) {
        return rechercherAVL(racine->gauche, identifiant);
    } else {
        return rechercherAVL(racine->droit, identifiant);
    }
}

void libererAVL(NoeudAVL *racine) {
    if (!racine) return;
    
    libererAVL(racine->gauche);
    libererAVL(racine->droit);
    free(racine->identifiant);
    free(racine);
}

// Fonctions des calculs des volumes et des fuites


/*Calcule le volume d'eau qui passe dans chaque noeud de l'arbre
le volume est réparti équitablement entre les enfants puis les fuites sont appliquées
recursivite aux enfants*/

void calculerVolumes(Noeud *noeud, double volume_parent, int nb_freres) {
    if (!noeud) return;
    
   // Répartition équitable du volume parent
    if (nb_freres > 0) {
        double volume_avant_fuite = volume_parent / nb_freres;
        noeud->volume = volume_avant_fuite * (100.0 - noeud->pourcentage_fuite) / 100.0;
    } else {
        noeud->volume = volume_parent * (100.0 - noeud->pourcentage_fuite) / 100.0;
    }
    
    for (int i = 0; i < noeud->nb_enfants; i++) {
        calculerVolumes(noeud->enfants[i], noeud->volume, noeud->nb_enfants);
    }
}

/*Calcule le total des fuites dans le sous-arbre , volume_perdu = volume_avant - volume_après
où volume_avant = volume_après * 100 / (100 - %fuite)*/
double calculerFuitesTotales(Noeud *noeud) {
    if (!noeud) return 0.0;
    
    double fuites_totales = 0.0;
    
    if (noeud->pourcentage_fuite > 0 && noeud->pourcentage_fuite < 100) {
        double volume_avant = noeud->volume * 100.0 / (100.0 - noeud->pourcentage_fuite);
        double volume_fuite = volume_avant - noeud->volume;
        fuites_totales += volume_fuite;
    }
    
    for (int i = 0; i < noeud->nb_enfants; i++) {
        fuites_totales += calculerFuitesTotales(noeud->enfants[i]);
    }
    
    return fuites_totales;
}

// Fonctions outils

char *dupliquerChaine(const char *str) {
    if (!str) return NULL;
    
    size_t longueur = strlen(str);
    char *dup = (char *)malloc((longueur + 1) * sizeof(char));
    if (!dup) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour la chaîne\n");
        return NULL;
    }
    
    strcpy(dup, str);
    return dup;
}

char *nettoyerEspaces(char *str) {
    if (!str) return NULL;
    
    // Suppression des espaces au début
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    if (*str == 0) return str;
    
    // Suppression des espaces à la fin
    char *fin = str + strlen(str) - 1;
    while (fin > str && (*fin == ' ' || *fin == '\t' || *fin == '\n' || *fin == '\r')) {
        fin--;
    }
    *(fin + 1) = '\0';
    
    return str;
}

int analyserLigneCSV(char *ligne, char *col1, char *col2, char *col3, char *col4, char *col5) {
    char *token;
    int indice_col = 0;
    
    //Initialisation des colonnes avec "-"
    strcpy(col1, "-");
    strcpy(col2, "-");
    strcpy(col3, "-");
    strcpy(col4, "-");
    strcpy(col5, "-");
    
    //Suppression du retour à la ligne
    ligne[strcspn(ligne, "\r\n")] = 0;
    
    //Analyse des colonnes separées par ";"
    token = strtok(ligne, ";");
    while (token != NULL && indice_col < 5) {
        token = nettoyerEspaces(token);
        
        switch(indice_col) {
            case 0: strncpy(col1, token, MAX_LONGUEUR_ID - 1); break;
            case 1: strncpy(col2, token, MAX_LONGUEUR_ID - 1); break;
            case 2: strncpy(col3, token, MAX_LONGUEUR_ID - 1); break;
            case 3: strncpy(col4, token, MAX_LONGUEUR_ID - 1); break;
            case 4: strncpy(col5, token, MAX_LONGUEUR_ID - 1); break;
        }
        
        indice_col++;
        token = strtok(NULL, ";");
    }
    
    return indice_col;
}

//Main
int main(int argc, char *argv[]) {
    //Verif des arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <fichier_donnees> <fichier_sortie> <identifiant_usine>\n", argv[0]);
        return 1;
    }
    
    const char *fichier_donnees = argv[1];
    const char *fichier_sortie = argv[2];
    const char *id_usine = argv[3];
    
    FILE *fp = fopen(fichier_donnees, "r");
    if (!fp) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", fichier_donnees);
        return 2;
    }
    
    NoeudAVL *racine_avl = NULL;
    Noeud **racines_stockage = NULL;
    int nb_stockages = 0;
    int capacite_stockages = 0;
    
    char ligne[MAX_LONGUEUR_LIGNE];
    char col1[MAX_LONGUEUR_ID], col2[MAX_LONGUEUR_ID];
    char col3[MAX_LONGUEUR_ID], col4[MAX_LONGUEUR_ID], col5[MAX_LONGUEUR_ID];
    
    double volume_traite_total = 0.0;
    int usine_trouvee = 0;
    
    /*Calcul du volume total traité par l'usine , on parcourt le ficher pour trouver les lignes sources->usines
    ensuite on somme le volume qui arrive a l'usine (apres fuites )*/
    while (fgets(ligne, sizeof(ligne), fp)) {
        analyserLigneCSV(ligne, col1, col2, col3, col4, col5);

        if (strcmp(col1, "-") == 0 && strcmp(col3, id_usine) == 0 && strcmp(col4, "-") != 0) {
            usine_trouvee = 1;
            double volume_source = atof(col4);
            double pourcentage_fuite = atof(col5);
            double volume_traite = volume_source * (100.0 - pourcentage_fuite) / 100.0;
            volume_traite_total += volume_traite;
        }
    }
    
    // Si l'usine n'est pas trouvée retourner -1
    if (!usine_trouvee) {
        fclose(fp);
        FILE *sortie = fopen(fichier_sortie, "w");
        if (sortie) {
            fprintf(sortie, "identifier;Leak volume (M.m3.year-1)\n");
            fprintf(sortie, "%s;-1\n", id_usine);
            fclose(sortie);
        }
        return 0;
    }
    
    /*Construction de l'arbre de distribution , on reconstruit l'arbre des éléments en aval de l'usine :
     usine->stockage (racines de l'arbre)
     parent->enfant pour les autres niveaux (jonctions, raccordements, usagers)*/

    rewind(fp);
    
    while (fgets(ligne, sizeof(ligne), fp)) {
        analyserLigneCSV(ligne, col1, col2, col3, col4, col5);

        // Lignes usine->stockage
        if (strcmp(col1, "-") == 0 && strcmp(col2, id_usine) == 0 && strcmp(col3, "-") != 0 && strcmp(col5, "-") != 0) {
            double pourcentage_fuite = atof(col5);
            Noeud *noeud_stockage = creerNoeud(col3, pourcentage_fuite);
            
            if (noeud_stockage) {
                if (nb_stockages >= capacite_stockages) {
                    int nouvelle_capacite = capacite_stockages == 0 ? 4 : capacite_stockages * 2;
                    Noeud **nouvelles_racines = (Noeud **)realloc(racines_stockage, nouvelle_capacite * sizeof(Noeud *));
                    if (nouvelles_racines) {
                        racines_stockage = nouvelles_racines;
                        capacite_stockages = nouvelle_capacite;
                    }
                }
                racines_stockage[nb_stockages++] = noeud_stockage;
                racine_avl = insererAVL(racine_avl, col3, noeud_stockage);
            }
        }
            
        // Lignes PARENT → ENFANT
        else if (strcmp(col1, id_usine) == 0 && strcmp(col2, "-") != 0 && strcmp(col3, "-") != 0 && strcmp(col5, "-") != 0) {
            double pourcentage_fuite = atof(col5);
            
            Noeud *parent = rechercherAVL(racine_avl, col2);
            if (parent) {
                Noeud *enfant = creerNoeud(col3, pourcentage_fuite);
                if (enfant) {
                    ajouterEnfant(parent, enfant);
                    racine_avl = insererAVL(racine_avl, col3, enfant);
                }
            }
        }
    }
    
    fclose(fp);
    
    //calcul des volumes dans chaque noeud
    for (int i = 0; i < nb_stockages; i++) {
        calculerVolumes(racines_stockage[i], volume_traite_total, nb_stockages);
    }
    
    //Calcul du total des fuites
    double fuites_totales = 0.0;
    for (int i = 0; i < nb_stockages; i++) {
        fuites_totales += calculerFuitesTotales(racines_stockage[i]);
    }
    
    //Conversion en millions de m³
    double fuites_totales_Mm3 = fuites_totales / 1000.0;
    
    //Resultat
    FILE *sortie = fopen(fichier_sortie, "w");
    if (!sortie) {
        fprintf(stderr, "Erreur: Impossible de créer le fichier %s\n", fichier_sortie);
        for (int i = 0; i < nb_stockages; i++) {
            libererArbre(racines_stockage[i]);
        }
        free(racines_stockage);
        libererAVL(racine_avl);
        return 3;
    }
    
    fprintf(sortie, "identifiant;Leak volume (M.m3.year-1)\n");
    fprintf(sortie, "%s;%.6f\n", id_usine, fuites_totales_Mm3);
    fclose(sortie);
    
    //Libération mémoire
    for (int i = 0; i < nb_stockages; i++) {
        libererArbre(racines_stockage[i]);
    }
    free(racines_stockage);
    libererAVL(racine_avl);
    
    return 0;
}
