#include "histo.h"

// GESTION AVL
 
int hauteur(pNoeud n) {
    return (n == NULL) ? 0 : n->hauteur;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

// Facteur d'équilibre : positif si déséquilibre à gauche, négatif à droite 
int equilibre(pNoeud n) {
    return (n == NULL) ? 0 : hauteur(n->fg) - hauteur(n->fd);
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
    n->fg = NULL;
    n->fd = NULL;
    return n;
}

void libererAVL(pNoeud r) {
    if (!r) return;
    libererAVL(r->fg);
    libererAVL(r->fd);
    free(r->id);
    free(r);
}


pNoeud rotationDroite(pNoeud y) {
    pNoeud x = y->fg;
    pNoeud T2 = x->fd;

    x->fd = y;
    y->fg = T2;

    y->hauteur = max(hauteur(y->fg), hauteur(y->fd)) + 1;
    x->hauteur = max(hauteur(x->fg), hauteur(x->fd)) + 1;

    return x;
}


pNoeud rotationGauche(pNoeud x) {
    pNoeud y = x->fd;
    pNoeud T2 = y->fg;

    y->fg = x;
    x->fd = T2;

    x->hauteur = max(hauteur(x->fg), hauteur(x->fd)) + 1;
    y->hauteur = max(hauteur(y->fg), hauteur(y->fd)) + 1;

    return y;
}

// Insertion dans l'AVL : cumule les volumes si l'ID existe déjà et maintient l'équilibre de l'arbre via rotations

pNoeud insertAVL(pNoeud racine, const char* id, double vol) {
    
    if (racine == NULL)
        return creerNoeud(id, vol);

    int cmp = strcmp(id, racine->id);

    if (cmp < 0)
        racine->fg = insertAVL(racine->fg, id, vol);
    else if (cmp > 0)
        racine->fd = insertAVL(racine->fd, id, vol);
    else {
        racine->volume += vol;
        return racine;
    }

    
    racine->hauteur = 1 + max(hauteur(racine->fg), hauteur(racine->fd));

    
    int eq = equilibre(racine);

    if (eq > 1 && strcmp(id, racine->fg->id) < 0)
        return rotationDroite(racine);

    if (eq < -1 && strcmp(id, racine->fd->id) > 0)
        return rotationGauche(racine);

    if (eq > 1 && strcmp(id, racine->fg->id) > 0) {
        racine->fg = rotationGauche(racine->fg);
        return rotationDroite(racine);
    }

    if (eq < -1 && strcmp(id, racine->fd->id) < 0) {
        racine->fd = rotationDroite(racine->fd);
        return rotationGauche(racine);
    }

    return racine;
}

// Parcours infixe inversé (droite-racine-gauche) pour obtenir l'ordre alphabétique inverse 

void convertirAVL(pNoeud r, elem_t* tab, int* idx) {
    if (!r) return;
    convertirAVL(r->fd, tab, idx);
    if (*idx < MAX_NODES) {
        strncpy(tab[*idx].id, r->id, sizeof(tab[*idx].id) - 1);
        tab[*idx].id[sizeof(tab[*idx].id) - 1] = '\0';
        tab[*idx].volume = r->volume;
        (*idx)++;
    }
    convertirAVL(r->fg, tab, idx);
}

// TRI

int cmp_desc(const void* a, const void* b) {
    double x = ((elem_t*)a)->volume;
    double y = ((elem_t*)b)->volume;
    if (x > y) return -1;
    if (x < y) return 1;
    return 0;
}

int cmp_id(const void* a, const void* b) {
    return strcmp(((elem_t*)a)->id, ((elem_t*)b)->id);
}

// PARSING

void trim(char* str) {
    if (!str) return;
    
    char *end = str + strlen(str) - 1;
    while (end >= str && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
    
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t')) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int is_dash(const char* s) {
    if (!s) return 0;
    return (strcmp(s, "-") == 0);
}

int not_dash(const char* s) {
    if (!s) return 0;
    return (strcmp(s, "-") != 0);
}

int parse_line(char* line, char* fields[], int max_fields) {
    int count = 0;
    char* ptr = line;
    char* start = line;
    
    while (*ptr && count < max_fields) {
        if (*ptr == ';') {
            *ptr = '\0';
            fields[count] = start;
            trim(fields[count]);
            count++;
            start = ptr + 1;
        }
        ptr++;
    }
    
    if (count < max_fields && start) {
        fields[count] = start;
        trim(fields[count]);
        count++;
    }
    
    return count;
}

// TRAITEMENT DES MODES

/* -MODE MAX : génère histogramme des volumes max par usine
 - Filtre : seul le champs[3]est non-vide (différent de "-")
 - Sortie : Top 10 des usines max + top 50 des min usines */

void process_max(const char* datafile, const char* csvfile) {
    FILE* fin = fopen(datafile, "r");
    if (!fin) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", datafile);
        exit(1);
    }

    printf("Génération Histogrammes pour les Usines\n");

    pNoeud racine = NULL;
    char line[MAX_LINE];
    int line_num = 0;
    
    if (fgets(line, sizeof(line), fin)) {
        line_num++;
    }

    while (fgets(line, sizeof(line), fin)) {
        line_num++;
        
        char* fields[10] = {NULL};
        int field_count = parse_line(line, fields, 10);

        if (field_count < 5) {
            continue;
        }

        if (is_dash(fields[0]) && not_dash(fields[1]) && is_dash(fields[2]) && 
            not_dash(fields[3]) && is_dash(fields[4])) 
        {    
            double vol = atof(fields[3]);
            racine = insertAVL(racine, fields[1], vol);
        }
    }
    fclose(fin);

    if (racine == NULL) {
        fprintf(stderr, "Aucune donnée trouvée pour le mode max\n");
        FILE* fout = fopen(csvfile, "w");
        if (fout) {
            fprintf(fout, "identifier;max volume (k.m3.year-1)\n");
            fclose(fout);
        }
        return;
    }

    elem_t* tab = malloc(MAX_NODES * sizeof(elem_t));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int count = 0;
    convertirAVL(racine, tab, &count);
    qsort(tab, count, sizeof(elem_t), cmp_desc);

    FILE* fout = fopen(csvfile, "w");
    if (!fout) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", csvfile);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    fprintf(fout, "=== Top 10 plus grandes usines ===\n");
    int lim = (count < 10) ? count : 10;
    fprintf(fout, "identifier;max volume (k.m3.year-1)\n");
    for (int i = 0; i < lim; i++) {
        fprintf(fout, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fprintf(fout, "\n=== Top 50 plus petites usines ===\n");
    int start = (count < 50) ? 0 : count - 50;
    fprintf(fout, "identifier;max volume (k.m3.year-1)\n");
    for (int i = count - 1; i >= start; i--) {
        fprintf(fout, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fclose(fout);
    free(tab);
    libererAVL(racine);

    printf("Histogramme généré max : %s\n", csvfile);
}

/* MODE SRC : génère histogramme des volumes par source
 * Filtre : champs[0]=="-" sinon le reste est différent de "-"
 * Sortie : Top 10 max usines + top 50 min usines */

void process_src(const char* datafile, const char* csvfile) {
    FILE* fin = fopen(datafile, "r");
    if (!fin) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", datafile);
        exit(1);
    }

    printf("Génération Histogrammes pour les Sources\n\n");

    pNoeud racine = NULL;
    char line[MAX_LINE];
    
    while (fgets(line, sizeof(line), fin)) {
        char* fields[10] = {NULL};
        int field_count = parse_line(line, fields, 10);

        if (field_count < 5) continue;

        if (is_dash(fields[0]) && not_dash(fields[1]) && not_dash(fields[2]) && 
            not_dash(fields[3]) && not_dash(fields[4])) 
        {
            double vol = atof(fields[3]);
            racine = insertAVL(racine, fields[2], vol);
        }
    }
    fclose(fin);

    if (racine == NULL) {
        fprintf(stderr, "Aucune donnée trouvée pour le mode src\n");
        FILE* fout = fopen(csvfile, "w");
        if (fout) {
            fprintf(fout, "identifier;source volume (k.m3.year-1)\n");
            fclose(fout);
        }
        return;
    }

    elem_t* tab = malloc(MAX_NODES * sizeof(elem_t));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int count = 0;
    convertirAVL(racine, tab, &count);
    qsort(tab, count, sizeof(elem_t), cmp_desc);

    FILE* fout = fopen(csvfile, "w");
    if (!fout) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", csvfile);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    fprintf(fout, "=== Top 10 plus grandes sources ===\n");
    int lim = (count < 10) ? count : 10;
    fprintf(fout, "identifier;source volume (k.m3.year-1)\n");
    for (int i = 0; i < lim; i++) {
        fprintf(fout, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fprintf(fout, "\n=== Top 50 plus petites sources ===\n");
    int start = (count < 50) ? 0 : count - 50;
    fprintf(fout, "identifier;source volume (k.m3.year-1)\n");
    for (int i = count - 1; i >= start; i--) {
        fprintf(fout, "%s;%.0f\n", tab[i].id, tab[i].volume);
    }

    fclose(fout);
    free(tab);
    libererAVL(racine);

    printf("Histogramme généré src : %s\n", csvfile);
    printf("\n");
}

/* MODE REEL : génère histogramme des volumes réels par source
 * Calcul : volume_reel = volume × (100 - pourcentage) / 100
 * Filtre : le même que pour le mode SRC (car aussi des sources-usines)
 * Sortie : Top 10 max usines + top 50 min usines */

void process_reel(const char* datafile, const char* csvfile) {
    FILE* fin = fopen(datafile, "r");
    if (!fin) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", datafile);
        exit(1);
    }

    printf("Génération Histogrammes pour les Réels\n\n");

    pNoeud racine = NULL;
    char line[MAX_LINE];
    
    while (fgets(line, sizeof(line), fin)) {
        char* fields[10] = {NULL};
        int field_count = parse_line(line, fields, 10);

        if (field_count < 5) continue;

        if (is_dash(fields[0]) && not_dash(fields[1]) && not_dash(fields[2]) && 
            not_dash(fields[3]) && not_dash(fields[4])) {
            
            double volume = atof(fields[3]);
            double percentage = atof(fields[4]);
            double result = volume * (100.0 - percentage) / 100.0;
            racine = insertAVL(racine, fields[2], result);
        }
    }
    fclose(fin);

    if (racine == NULL) {
        fprintf(stderr, "Aucune donnée trouvée pour le mode reel\n");
        FILE* fout = fopen(csvfile, "w");
        if (fout) {
            fprintf(fout, "identifier;real volume (k.m3.year-1)\n");
            fclose(fout);
        }
        return;
    }

    elem_t* tab = malloc(MAX_NODES * sizeof(elem_t));
    if (!tab) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        libererAVL(racine);
        exit(1);
    }

    int count = 0;
    convertirAVL(racine, tab, &count);
    qsort(tab, count, sizeof(elem_t), cmp_desc);

    FILE* fout = fopen(csvfile, "w");
    if (!fout) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", csvfile);
        free(tab);
        libererAVL(racine);
        exit(1);
    }

    fprintf(fout, "=== Top 10 plus grandes sources (volume réel) ===\n");
    int lim = (count < 10) ? count : 10;
    fprintf(fout, "identifier;real volume (k.m3.year-1)\n");
    for (int i = 0; i < lim; i++) {
        fprintf(fout, "%s;%.2f\n", tab[i].id, tab[i].volume);
    }

    fprintf(fout, "\n=== Top 50 plus petites sources (volume réel) ===\n");
    int start = (count < 50) ? 0 : count - 50;
    fprintf(fout, "identifier;real volume (k.m3.year-1)\n");
    for (int i = count - 1; i >= start; i--) {
        fprintf(fout, "%s;%.2f\n", tab[i].id, tab[i].volume);
    }

    fclose(fout);
    free(tab);
    libererAVL(racine);

    printf("Histogramme généré reel : %s\n", csvfile);
    printf("Nombre de sources: %d\n\n", count);
}

// MAIN

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s <file.dat> {maxC|srcC|reelC}\n", argv[0]);
        return 1;
    }

    char* datafile = argv[1];
    char* mode = argv[2];

    // Suppression du 'C' final si présent 
    
    char actual_mode[32];
    strncpy(actual_mode, mode, sizeof(actual_mode) - 1);
    actual_mode[sizeof(actual_mode) - 1] = '\0';
    
    size_t len = strlen(actual_mode);
    if (len > 0 && actual_mode[len - 1] == 'C') {
        actual_mode[len - 1] = '\0';
    }

    char csvfile[512];
    snprintf(csvfile, sizeof(csvfile), "csv/histo_%s.csv", mode);

    if (strcmp(actual_mode, "max") == 0) {
        process_max(datafile, csvfile);
    }
    else if (strcmp(actual_mode, "src") == 0) {
        process_src(datafile, csvfile);
    }
    else if (strcmp(actual_mode, "reel") == 0) {
        process_reel(datafile, csvfile);
    }
    else {
        fprintf(stderr, "Mode inconnu: %s\n", mode);
        fprintf(stderr, "Modes valides: maxC, srcC, reelC\n");
        return 1;
    }

    return 0;
}
