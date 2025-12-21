#include "histo.h"
// version finale
// date 21/12/2025
// GESTION AVL
 
int hauteur(pNoeud n) {
    return (n == NULL) ? 0 : n->hauteur;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

// Facteur d'équilibre : positif si déséquilibre à gauche, négatif à droite 
int equilibre(pNoeud n) {
    return (n == NULL) ? 0 : hauteur(n->gauche) - hauteur(n->droit);
}

pNoeud creerNoeud(const char* id, double vol) {
    pNoeud n = malloc(sizeof(Noeud));
    if (!n) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        exit(1);
    }
    n->id = strdup(id);
    if (!n->id) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        free(n);
        exit(1);
    }
    n->volume = vol;
    n->hauteur = 1;
    n->gauche = NULL;
    n->droit = NULL;
    return n;
}

void libererAVL(pNoeud r) {
    if (!r) return;
    libererAVL(r->gauche);
    libererAVL(r->droit);
    free(r->id);
    free(r);
}

pNoeud rotationDroite(pNoeud y) {
    pNoeud x = y->gauche;
    pNoeud T2 = x->droit;

    x->droit = y;
    y->gauche = T2;

    y->hauteur = max(hauteur(y->gauche), hauteur(y->droit)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droit)) + 1;

    return x;
}

pNoeud rotationGauche(pNoeud x) {
    pNoeud y = x->droit;
    pNoeud T2 = y->gauche;

    y->gauche = x;
    x->droit = T2;

    x->hauteur = max(hauteur(x->gauche), hauteur(x->droit)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droit)) + 1;

    return y;
}

// Insertion dans l'AVL : cumule les volumes si l'ID existe déjà et maintient l'équilibre de l'arbre via rotations
pNoeud insererAVL(pNoeud racine, const char* id, double vol) {
    
    if (racine == NULL)
        return creerNoeud(id, vol);

    int cmp = strcmp(id, racine->id);

    if (cmp < 0)
        racine->gauche = insererAVL(racine->gauche, id, vol);
    else if (cmp > 0)
        racine->droit = insererAVL(racine->droit, id, vol);
    else {
        racine->volume += vol;
        return racine;
    }

    racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droit));

    int eq = equilibre(racine);

    if (eq > 1 && strcmp(id, racine->gauche->id) < 0)
        return rotationDroite(racine);

    if (eq < -1 && strcmp(id, racine->droit->id) > 0)
        return rotationGauche(racine);

    if (eq > 1 && strcmp(id, racine->gauche->id) > 0) {
        racine->gauche = rotationGauche(racine->gauche);
        return rotationDroite(racine);
    }

    if (eq < -1 && strcmp(id, racine->droit->id) < 0) {
        racine->droit = rotationDroite(racine->droit);
        return rotationGauche(racine);
    }

    return racine;
}

// Parcours infixe inversé (droit-racine-gauche) pour obtenir l'ordre alphabétique inverse 
void convertirAVL(pNoeud r, Element* tab, int* idx) {
    if (!r) return;
    convertirAVL(r->droit, tab, idx);
    if (*idx < MAX_NOEUDS) {
        strncpy(tab[*idx].id, r->id, sizeof(tab[*idx].id) - 1);
        tab[*idx].id[sizeof(tab[*idx].id) - 1] = '\0';
        tab[*idx].volume = r->volume;
        (*idx)++;
    }
    convertirAVL(r->gauche, tab, idx);
}

// TRI

int comparerDecroissant(const void* a, const void* b) {
    double x = ((Element*)a)->volume;
    double y = ((Element*)b)->volume;
    if (x > y) return -1;
    if (x < y) return 1;
    return 0;
}

int comparerId(const void* a, const void* b) {
    return strcmp(((Element*)a)->id, ((Element*)b)->id);
}

// ANALYSE

void nettoyer(char* str) {
    if (!str) return;
    
    char *fin = str + strlen(str) - 1;
    while (fin >= str && (*fin == '\n' || *fin == '\r' || *fin == ' ' || *fin == '\t')) {
        *fin = '\0';
        fin--;
    }
    
    char *debut = str;
    while (*debut && (*debut == ' ' || *debut == '\t')) {
        debut++;
    }
    
    if (debut != str) {
        memmove(str, debut, strlen(debut) + 1);
    }
}

int estTiret(const char* s) {
    if (!s) return 0;
    return (strcmp(s, "-") == 0);
}

int nonTiret(const char* s) {
    if (!s) return 0;
    return (strcmp(s, "-") != 0);
}

int analyserLigne(char* ligne, char* champs[], int max_champs) {
    int compte = 0;
    char* ptr = ligne;
    char* debut = ligne;
    
    while (*ptr && compte < max_champs) {
        if (*ptr == ';') {
            *ptr = '\0';
            champs[compte] = debut;
            nettoyer(champs[compte]);
            compte++;
            debut = ptr + 1;
        }
        ptr++;
    }
    
    if (compte < max_champs && debut) {
        champs[compte] = debut;
        nettoyer(champs[compte]);
        compte++;
    }
    
    return compte;
}

// TRAITEMENT DES MODES

/* MODE MAX : génère histogramme des volumes max par usine
 * Filtre : seul le champs[3] est non-vide (différent de "-")
 * Sortie : Top 10 des usines max + top 50 des min usines */
void traiterMax(const char* fichier_donnees, const char* fichier_csv) {
    FILE* entree = fopen(fichier_donnees, "r");
    if (!entree) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", fichier_donnees);
        exit(1);
    }

    printf("Génération Histogrammes pour les Usines\n");

    pNoeud racine = NULL;
    char ligne[MAX_LIGNE];
    int num_ligne = 0;
    
    if (fgets(ligne, sizeof(ligne), entree)) {
        num_ligne++;
    }

    while (fgets(ligne, sizeof(ligne), entree)) {
        num_ligne++;
        
        char* champs[10] = {NULL};
        int nb_champs = analyserLigne(ligne, champs, 10);

        if (nb_champs < 5) {
            continue;
        }

        if (estTiret(champs[0]) && nonTiret(champs[1]) && estTiret(champs[2]) && 
            nonTiret(champs[3]) && estTiret(champs[4])) 
        {    
            double vol = atof(champs[3]);
            racine = insererAVL(racine, champs[1], vol);
        }
    }
    fclose(entree);

    if (racine == NULL) {
        fprintf(stderr, "Aucune donnée trouvée pour le mode max\n");
        FILE* sortie = fopen(fichier_csv, "w");
        if (sortie) {
            fprintf(sortie, "identifier;max volume (k.m3.year-1)\n");
            fclose(sortie);
        }
        return;
    }

    Element* tab = malloc(MAX_NOEUDS * sizeof(Element));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int compte = 0;
    convertirAVL(racine, tab, &compte);
    qsort(tab, compte, sizeof(Element), comparerDecroissant);

    FILE* sortie = fopen(fichier_csv, "w");
    if (!sortie) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", fichier_csv);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    fprintf(sortie, "=== Top 10 plus grandes usines ===\n");
    int limite = (compte < 10) ? compte : 10;
    fprintf(sortie, "identifier;max volume (k.m3.year-1)\n");
    for (int i = 0; i < limite; i++) {
        fprintf(sortie, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fprintf(sortie, "\n=== Top 50 plus petites usines ===\n");
    int debut = (compte < 50) ? 0 : compte - 50;
    fprintf(sortie, "identifier;max volume (k.m3.year-1)\n");
    for (int i = compte - 1; i >= debut; i--) {
        fprintf(sortie, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fclose(sortie);
    free(tab);
    libererAVL(racine);

    printf("Histogramme généré max : %s\n", fichier_csv);
}

/* MODE SRC : génère histogramme des volumes par source
 * Filtre : champs[0]=="-" sinon le reste est différent de "-"
 * Sortie : Top 10 max sources + top 50 min sources */
void traiterSource(const char* fichier_donnees, const char* fichier_csv) {
    FILE* entree = fopen(fichier_donnees, "r");
    if (!entree) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", fichier_donnees);
        exit(1);
    }

    printf("Génération Histogrammes pour les Sources\n\n");

    pNoeud racine = NULL;
    char ligne[MAX_LIGNE];
    
    while (fgets(ligne, sizeof(ligne), entree)) {
        char* champs[10] = {NULL};
        int nb_champs = analyserLigne(ligne, champs, 10);

        if (nb_champs < 5) continue;

        if (estTiret(champs[0]) && nonTiret(champs[1]) && nonTiret(champs[2]) && 
            nonTiret(champs[3]) && nonTiret(champs[4])) 
        {
            double vol = atof(champs[3]);
            racine = insererAVL(racine, champs[2], vol);
        }
    }
    fclose(entree);

    if (racine == NULL) {
        fprintf(stderr, "Aucune donnée trouvée pour le mode src\n");
        FILE* sortie = fopen(fichier_csv, "w");
        if (sortie) {
            fprintf(sortie, "identifier;source volume (k.m3.year-1)\n");
            fclose(sortie);
        }
        return;
    }

    Element* tab = malloc(MAX_NOEUDS * sizeof(Element));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int compte = 0;
    convertirAVL(racine, tab, &compte);
    qsort(tab, compte, sizeof(Element), comparerDecroissant);

    FILE* sortie = fopen(fichier_csv, "w");
    if (!sortie) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", fichier_csv);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    fprintf(sortie, "=== Top 10 plus grandes sources ===\n");
    int limite = (compte < 10) ? compte : 10;
    fprintf(sortie, "identifier;source volume (k.m3.year-1)\n");
    for (int i = 0; i < limite; i++) {
        fprintf(sortie, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fprintf(sortie, "\n=== Top 50 plus petites sources ===\n");
    int debut = (compte < 50) ? 0 : compte - 50;
    fprintf(sortie, "identifier;source volume (k.m3.year-1)\n");
    for (int i = compte - 1; i >= debut; i--) {
        fprintf(sortie, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fclose(sortie);
    free(tab);
    libererAVL(racine);

    printf("Histogramme généré src : %s\n", fichier_csv);
    printf("\n");
}

/* MODE REEL : génère histogramme des volumes réels par source
 * Calcul : volume_reel = volume × (100 - pourcentage) / 100
 * Filtre : le même que pour le mode SRC (car aussi des sources-usines)
 * Sortie : Top 10 max sources + top 50 min sources */
void traiterReel(const char* fichier_donnees, const char* fichier_csv) {
    FILE* entree = fopen(fichier_donnees, "r");
    if (!entree) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", fichier_donnees);
        exit(1);
    }

    printf("Génération Histogrammes pour les Réels\n\n");

    pNoeud racine = NULL;
    char ligne[MAX_LIGNE];
    
    while (fgets(ligne, sizeof(ligne), entree)) {
        char* champs[10] = {NULL};
        int nb_champs = analyserLigne(ligne, champs, 10);

        if (nb_champs < 5) continue;

        if (estTiret(champs[0]) && nonTiret(champs[1]) && nonTiret(champs[2]) && 
            nonTiret(champs[3]) && nonTiret(champs[4])) {
            
            double volume = atof(champs[3]);
            double pourcentage = atof(champs[4]);
            double resultat = volume * (100.0 - pourcentage) / 100.0;
            racine = insererAVL(racine, champs[2], resultat);
        }
    }
    fclose(entree);

    if (racine == NULL) {
        fprintf(stderr, "Aucune donnée trouvée pour le mode reel\n");
        FILE* sortie = fopen(fichier_csv, "w");
        if (sortie) {
            fprintf(sortie, "identifier;real volume (k.m3.year-1)\n");
            fclose(sortie);
        }
        return;
    }

    Element* tab = malloc(MAX_NOEUDS * sizeof(Element));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int compte = 0;
    convertirAVL(racine, tab, &compte);
    qsort(tab, compte, sizeof(Element), comparerDecroissant);

    FILE* sortie = fopen(fichier_csv, "w");
    if (!sortie) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", fichier_csv);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    fprintf(sortie, "=== Top 10 plus grandes sources (volume réel) ===\n");
    int limite = (compte < 10) ? compte : 10;
    fprintf(sortie, "identifier;real volume (k.m3.year-1)\n");
    for (int i = 0; i < limite; i++) {
        fprintf(sortie, "%s;%.2f\n", tab[i].id, tab[i].volume);
    }

    fprintf(sortie, "\n=== Top 50 plus petites sources (volume réel) ===\n");
    int debut = (compte < 50) ? 0 : compte - 50;
    fprintf(sortie, "identifier;real volume (k.m3.year-1)\n");
    for (int i = compte - 1; i >= debut; i--) {
        fprintf(sortie, "%s;%.2f\n", tab[i].id, tab[i].volume);
    }

    fclose(sortie);
    free(tab);
    libererAVL(racine);

    printf("Histogramme généré reel : %s\n", fichier_csv);
    printf("Nombre de sources: %d\n\n", compte);
}

// MAIN

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s <fichier.dat> {maxC|srcC|reelC}\n", argv[0]);
        return 1;
    }

    char* fichier_donnees = argv[1];
    char* mode = argv[2];

    // Suppression du 'C' final si présent 
    char mode_reel[32];
    strncpy(mode_reel, mode, sizeof(mode_reel) - 1);
    mode_reel[sizeof(mode_reel) - 1] = '\0';
    
    size_t longueur = strlen(mode_reel);
    if (longueur > 0 && mode_reel[longueur - 1] == 'C') {
        mode_reel[longueur - 1] = '\0';
    }

    char fichier_csv[512];
    snprintf(fichier_csv, sizeof(fichier_csv), "csv/histo_%s.csv", mode);

    if (strcmp(mode_reel, "max") == 0) {
        traiterMax(fichier_donnees, fichier_csv);
    }
    else if (strcmp(mode_reel, "src") == 0) {
        traiterSource(fichier_donnees, fichier_csv);
    }
    else if (strcmp(mode_reel, "reel") == 0) {
        traiterReel(fichier_donnees, fichier_csv);
    }
    else {
        fprintf(stderr, "Mode inconnu: %s\n", mode);
        fprintf(stderr, "Modes valides: maxC, srcC, reelC\n");
        return 1;
    }

    return 0;
}
