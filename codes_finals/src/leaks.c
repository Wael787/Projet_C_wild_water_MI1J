#include "leaks.h"
// version finale
// date : 21/12/2025

// FONCTIONS DE GESTION DES NŒUDS DE L'ARBRE DE DISTRIBUTION

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

void ajouterEnfant(Noeud *parent, Noeud *enfant) {
    if (!parent || !enfant) return;
    
    /* Agrandir le tableau si nécessaire */
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

void libererArbre(Noeud *racine) {
    if (!racine) return;
    
    for (int i = 0; i < racine->nb_enfants; i++) {
        libererArbre(racine->enfants[i]);
    }
    
    free(racine->identifiant);
    free(racine->enfants);
    free(racine);
}

// FONCTIONS DE GESTION DE L'AVL

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
    /* Insertion BST classique */
    if (!racine) {
        return creerNoeudAVL(identifiant, noeud_donnees);
    }
    
    int cmp = strcmp(identifiant, racine->identifiant);
    if (cmp < 0) {
        racine->gauche = insererAVL(racine->gauche, identifiant, noeud_donnees);
    } else if (cmp > 0) {
        racine->droit = insererAVL(racine->droit, identifiant, noeud_donnees);
    } else {
        /* Identifiant déjà existant, on ne fait rien */
        return racine;
    }
    
    // Mise à jour de la hauteur 
    racine->hauteur = 1 + maxAVL(hauteurAVL(racine->gauche), hauteurAVL(racine->droit));
    
    // Calcul du facteur d'équilibre 
    int equilibre = facteurEquilibreAVL(racine);
    
    // Cas de déséquilibre 
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

// FONCTIONS DE CALCUL

void calculerVolumes(Noeud *noeud, double volume_parent, int nb_freres) {
    if (!noeud) return;
    
    /* Calcul du volume pour ce nœud */
    if (nb_freres > 0) {
        /* Répartition équitable entre les frères */
        double volume_avant_fuite = volume_parent / nb_freres;
        /* Application des fuites */
        noeud->volume = volume_avant_fuite * (100.0 - noeud->pourcentage_fuite) / 100.0;
    } else {
        noeud->volume = volume_parent * (100.0 - noeud->pourcentage_fuite) / 100.0;
    }
    
    // Propagation récursive aux enfants 
    for (int i = 0; i < noeud->nb_enfants; i++) {
        calculerVolumes(noeud->enfants[i], noeud->volume, noeud->nb_enfants);
    }
}

double calculerFuitesTotales(Noeud *noeud) {
    if (!noeud) return 0.0;
    
    double fuites_totales = 0.0;
    
    if (noeud->pourcentage_fuite > 0 && noeud->pourcentage_fuite < 100) {
        double volume_avant = noeud->volume * 100.0 / (100.0 - noeud->pourcentage_fuite);
        double volume_fuite = volume_avant - noeud->volume;
        fuites_totales += volume_fuite;
    }
    
    /* Somme récursive des fuites des enfants */
    for (int i = 0; i < noeud->nb_enfants; i++) {
        fuites_totales += calculerFuitesTotales(noeud->enfants[i]);
    }
    
    return fuites_totales;
}

// FONCTIONS UTILITAIRES

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
    
    /* Suppression des espaces au début */
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
    
    // Initialisation des colonnes avec "-" 
    strcpy(col1, "-");
    strcpy(col2, "-");
    strcpy(col3, "-");
    strcpy(col4, "-");
    strcpy(col5, "-");
    
    // Suppression du retour à la ligne 
    ligne[strcspn(ligne, "\r\n")] = 0;
    
    // Parse des colonnes séparées par ';'
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

// FONCTION PRINCIPALE

int main(int argc, char *argv[]) {
    /* Vérification des arguments */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <fichier_donnees> <fichier_sortie> <identifiant_usine>\n", argv[0]);
        return 1;
    }
    
    const char *fichier_donnees = argv[1];
    const char *fichier_sortie = argv[2];
    const char *id_usine = argv[3];
    
    // Ouverture du fichier de données 
    FILE *fp = fopen(fichier_donnees, "r");
    if (!fp) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", fichier_donnees);
        return 2;
    }
    
    // Structures de données 
    NoeudAVL *racine_avl = NULL;
    Noeud **racines_stockage = NULL;
    int nb_stockages = 0;
    int capacite_stockages = 0;
    
    char ligne[MAX_LONGUEUR_LIGNE];
    char col1[MAX_LONGUEUR_ID], col2[MAX_LONGUEUR_ID];
    char col3[MAX_LONGUEUR_ID], col4[MAX_LONGUEUR_ID], col5[MAX_LONGUEUR_ID];
    
    double volume_traite_total = 0.0;
    int usine_trouvee = 0;
    
    // Phase 1: Calcul du volume total traité par l'usine 
    while (fgets(ligne, sizeof(ligne), fp)) {
        analyserLigneCSV(ligne, col1, col2, col3, col4, col5);
        
        // SOURCE → USINE 
        if (strcmp(col1, "-") == 0 && strcmp(col3, id_usine) == 0 && strcmp(col4, "-") != 0) {
            usine_trouvee = 1;
            double volume_source = atof(col4);
            double pourcentage_fuite = atof(col5);
            double volume_traite = volume_source * (100.0 - pourcentage_fuite) / 100.0;
            volume_traite_total += volume_traite;
        }
    }
    
    // Si l'usine n'est pas trouvée, retourner -1 
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
    
    // Phase 2: Construction de l'arbre de distribution 
    rewind(fp);
    
    while (fgets(ligne, sizeof(ligne), fp)) {
        analyserLigneCSV(ligne, col1, col2, col3, col4, col5);
        
        // USINE → STOCKAGE
        if (strcmp(col1, "-") == 0 && strcmp(col2, id_usine) == 0 && strcmp(col3, "-") != 0 && strcmp(col5, "-") != 0) {
            double pourcentage_fuite = atof(col5);
            Noeud *noeud_stockage = creerNoeud(col3, pourcentage_fuite);
            
            if (noeud_stockage) {
                // Ajouter à la liste des racines 
                if (nb_stockages >= capacite_stockages) {
                    int nouvelle_capacite = capacite_stockages == 0 ? 4 : capacite_stockages * 2;
                    Noeud **nouvelles_racines = (Noeud **)realloc(racines_stockage, nouvelle_capacite * sizeof(Noeud *));
                    if (nouvelles_racines) {
                        racines_stockage = nouvelles_racines;
                        capacite_stockages = nouvelle_capacite;
                    }
                }
                racines_stockage[nb_stockages++] = noeud_stockage;
                
                // Ajouter à l'AVL 
                racine_avl = insererAVL(racine_avl, col3, noeud_stockage);
            }
        }
        // STOCKAGE → JONCTION, JONCTION → RACCORDEMENT, RACCORDEMENT → USAGER 
        else if (strcmp(col1, id_usine) == 0 && strcmp(col2, "-") != 0 && strcmp(col3, "-") != 0 && strcmp(col5, "-") != 0) {
            double pourcentage_fuite = atof(col5);
            
            /* Rechercher le parent dans l'AVL */
            Noeud *parent = rechercherAVL(racine_avl, col2);
            if (parent) {
                /* Créer le nœud enfant */
                Noeud *enfant = creerNoeud(col3, pourcentage_fuite);
                if (enfant) {
                    ajouterEnfant(parent, enfant);
                    /* Ajouter à l'AVL pour recherches futures */
                    racine_avl = insererAVL(racine_avl, col3, enfant);
                }
            }
        }
    }
    
    fclose(fp);
    
    // Phase 3: Calcul des volumes dans l'arbre 
    for (int i = 0; i < nb_stockages; i++) {
        calculerVolumes(racines_stockage[i], volume_traite_total, nb_stockages);
    }
    
    // Phase 4: Calcul du total des fuites 
    double fuites_totales = 0.0;
    for (int i = 0; i < nb_stockages; i++) {
        fuites_totales += calculerFuitesTotales(racines_stockage[i]);
    }
    
    // Conversion en M.m³ (millions de m³) 
    double fuites_totales_Mm3 = fuites_totales / 1000.0;
    
    // Phase 5: Écriture du résultat 
    FILE *sortie = fopen(fichier_sortie, "w");
    if (!sortie) {
        fprintf(stderr, "Erreur: Impossible de créer le fichier %s\n", fichier_sortie);
        /* Libération de la mémoire */
        for (int i = 0; i < nb_stockages; i++) {
            libererArbre(racines_stockage[i]);
        }
        free(racines_stockage);
        libererAVL(racine_avl);
        return 3;
    }
    
    fprintf(sortie, "identifier;Leak volume (M.m3.year-1)\n");
    fprintf(sortie, "%s;%.6f\n", id_usine, fuites_totales_Mm3);
    fclose(sortie);
    
    // Libération de la mémoire 
    for (int i = 0; i < nb_stockages; i++) {
        libererArbre(racines_stockage[i]);
    }
    free(racines_stockage);
    libererAVL(racine_avl);
    
    return 0;
}
