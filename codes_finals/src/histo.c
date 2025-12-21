#include "histo.h"

// GESTION DE L'ARBRE AVL


int hauteur(pNoeud n) {
    return (n == NULL) ? 0 : n->hauteur;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int equilibre(pNoeud n) {
    return (n == NULL) ? 0 : hauteur(n->gauche) - hauteur(n->droit);
}

// Cree un nouveau noeud avec l'id et le volume
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

// Liberation recursive de l'arbre
void libererAVL(pNoeud r) {
    if (!r) return;
    libererAVL(r->gauche);
    libererAVL(r->droit);
    free(r->id);
    free(r);
}

// Réequilibrage
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

// Insert dans l'AVL en cumulant les volumes si l'ID existe deja et reequilibre si il faut

pNoeud insererAVL(pNoeud racine, const char* id, double vol) {
    
    if (racine == NULL)
        return creerNoeud(id, vol);

    int cmp = strcmp(id, racine->id);

    if (cmp < 0)
        racine->gauche = insererAVL(racine->gauche, id, vol);
    else if (cmp > 0)
        racine->droit = insererAVL(racine->droit, id, vol);
    else {
        // ID existe deja
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

// Parcours droit-racine-gauche pour avoir l'ordre alphabetique inverse, remplit avec les elements de l'arbre
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

//  FONCTIONS DE TRI
  
// Compare par volume decroissant (pour qsort)
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

//  PARSING DU FICHIER

// Enleve les espaces et retours à la ligne
void nettoyer(char* str) {
    if (!str) return;
    
    // pr les espaces fin
    char *fin = str + strlen(str) - 1;
    while (fin >= str && (*fin == '\n' || *fin == '\r' || *fin == ' ' || *fin == '\t')) {
        *fin = '\0';
        fin--;
    }
    
    // pr les espaces debut
    char *debut = str;
    while (*debut && (*debut == ' ' || *debut == '\t')) {
        debut++;
    }
    
    if (debut != str) {
        memmove(str, debut, strlen(debut) + 1);
    }
}

// Verifie si c'est un champ vide
int estTiret(const char* s) {
    if (!s) return 0;
    return (strcmp(s, "-") == 0);
}

int nonTiret(const char* s) {
    if (!s) return 0;
    return (strcmp(s, "-") != 0);
}

// Decoupe une ligne CSV en champs separes par ';'
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
    
    // dernier champ
    if (compte < max_champs && debut) {
        champs[compte] = debut;
        nettoyer(champs[compte]);
        compte++;
    }
    
    return compte;
}

// mode max

// Genere l'histogramme des volumes max par usine
// On filtre les lignes avec seulement champs[1] et champs[3] non vides

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
    
    // Skip header
    if (fgets(ligne, sizeof(ligne), entree)) {
        num_ligne++;
    }

    // Lecture et insertion dans l'AVL
    while (fgets(ligne, sizeof(ligne), entree)) {
        num_ligne++;
        
        char* champs[10] = {NULL};
        int nb_champs = analyserLigne(ligne, champs, 10);

        if (nb_champs < 5) continue;

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

    // Conversion AVL en tableau pour le tri
    Element* tab = malloc(MAX_NOEUDS * sizeof(Element));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int compte = 0;
    convertirAVL(racine, tab, &compte);
    qsort(tab, compte, sizeof(Element), comparerDecroissant);

    // Ecriture du fichier CSV
    FILE* sortie = fopen(fichier_csv, "w");
    if (!sortie) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", fichier_csv);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    // Top 10 plus grandes
    fprintf(sortie, "=== Top 10 plus grandes usines ===\n");
    int limite = (compte < 10) ? compte : 10;
    fprintf(sortie, "identifier;max volume (k.m3.year-1)\n");
    for (int i = 0; i < limite; i++) {
        fprintf(sortie, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    // Top 50 plus petites (on prend depuis la fin)
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

// mode src

// Genere l'histogramme des volumes par source
// Le filtre est : toutes les colonnes sauf la premiere doivent etre non-vides
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

        // Filtre : - / usine / source / volume / pourcent
        if (estTiret(champs[0]) && nonTiret(champs[1]) && nonTiret(champs[2]) && 
            nonTiret(champs[3]) && nonTiret(champs[4])) 
        {
            double vol = atof(champs[3]);
            // On groupe par source (champs[2])
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

    // Meme logique que traiterMax
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

// Mode réel

// Calcul le volume reel en enlevant le pourcentage de fuite
// volume_reel = volume × (100 - pourcent) / 100
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
            // Calcul du volume apres fuite
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

// Main


int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s <fichier.dat> {maxC|srcC|reelC}\n", argv[0]);
        return 1;
    }

    char* fichier_donnees = argv[1];
    char* mode = argv[2];

    // On enleve le 'C' à la fin pour avoir max/src/reel
    char mode_reel[32];
    strncpy(mode_reel, mode, sizeof(mode_reel) - 1);
    mode_reel[sizeof(mode_reel) - 1] = '\0';
    
    size_t longueur = strlen(mode_reel);
    if (longueur > 0 && mode_reel[longueur - 1] == 'C') {
        mode_reel[longueur - 1] = '\0';
    }

    // Chemin du fichier de sortie
    char fichier_csv[512];
    snprintf(fichier_csv, sizeof(fichier_csv), "csv/histo_%s.csv", mode);

    // Appel de la bonne fonction selon le mode
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
