#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 2048
#define MAX_ENTRIES 10000

// Structure simple pour stocker un identifiant et son volume
typedef struct {
    char id[256];
    double volume;
} Entry;

// Tableau global pour stocker toutes les entrées
Entry entries[MAX_ENTRIES];
int nb_entries = 0;

// Fonction pour enlever les espaces en début et fin de chaîne
void trim(char* str) {
    if (!str) return;
    
    // Enlever les espaces à la fin
    int len = strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '\n' || str[len-1] == '\r' || str[len-1] == '\t')) {
        str[len-1] = '\0';
        len--;
    }
    
    // Enlever les espaces au début
    char* start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// Fonction pour vérifier si un champ est un tiret
int is_dash(const char* s) {
    return (strcmp(s, "-") == 0);
}

// Fonction pour découper une ligne CSV avec le séparateur ';'
int split_line(char* line, char* fields[], int max_fields) {
    int count = 0;
    char* token = strtok(line, ";");
    
    while (token != NULL && count < max_fields) {
        fields[count] = token;
        trim(fields[count]);
        count++;
        token = strtok(NULL, ";");
    }
    
    return count;
}

// Fonction pour trouver un identifiant dans le tableau
int find_entry(const char* id) {
    for (int i = 0; i < nb_entries; i++) {
        if (strcmp(entries[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

// Fonction pour ajouter ou mettre à jour une entrée
void add_entry(const char* id, double volume) {
    int index = find_entry(id);
    
    if (index >= 0) {
        // L'identifiant existe déjà, on ajoute le volume
        entries[index].volume += volume;
    } else {
        // Nouvel identifiant
        if (nb_entries >= MAX_ENTRIES) {
            fprintf(stderr, "Erreur: trop d'entrées (max %d)\n", MAX_ENTRIES);
            return;
        }
        strncpy(entries[nb_entries].id, id, sizeof(entries[nb_entries].id) - 1);
        entries[nb_entries].id[sizeof(entries[nb_entries].id) - 1] = '\0';
        entries[nb_entries].volume = volume;
        nb_entries++;
    }
}

// Fonction de comparaison pour trier par volume décroissant
int compare_desc(const void* a, const void* b) {
    Entry* ea = (Entry*)a;
    Entry* eb = (Entry*)b;
    if (ea->volume > eb->volume) return -1;
    if (ea->volume < eb->volume) return 1;
    return 0;
}

// MODE MAX : volume max par usine
void process_max(const char* datafile, const char* csvfile) {
    FILE* fin = fopen(datafile, "r");
    if (!fin) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", datafile);
        exit(1);
    }

    printf("Mode MAX : Calcul des volumes maximaux par usine\n");

    char line[MAX_LINE];
    
    // Ignorer la première ligne (en-tête)
    if (fgets(line, sizeof(line), fin)) {}

    // Lire toutes les lignes
    while (fgets(line, sizeof(line), fin)) {
        char* fields[10];
        int nb_fields = split_line(line, fields, 10);

        if (nb_fields < 5) continue;

        // Condition: champ 1 = "-", champ 2 != "-", champ 3 = "-", champ 4 != "-"
        if (is_dash(fields[0]) && !is_dash(fields[1]) && is_dash(fields[2]) && !is_dash(fields[3])) {
            double volume = atof(fields[3]);
            add_entry(fields[1], volume);
        }
    }
    fclose(fin);

    // Trier par volume décroissant
    qsort(entries, nb_entries, sizeof(Entry), compare_desc);

    // Écrire le résultat
    FILE* fout = fopen(csvfile, "w");
    if (!fout) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", csvfile);
        exit(1);
    }

    fprintf(fout, "identifier;volume\n");
    for (int i = 0; i < nb_entries; i++) {
        fprintf(fout, "%s;%.0f\n", entries[i].id, entries[i].volume);
    }

    fclose(fout);
    printf("Fichier généré: %s (%d usines)\n", csvfile, nb_entries);
}

// MODE SRC : somme des volumes par source
void process_src(const char* datafile, const char* csvfile) {
    FILE* fin = fopen(datafile, "r");
    if (!fin) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", datafile);
        exit(1);
    }

    printf("Mode SRC : Calcul des volumes par source\n");

    char line[MAX_LINE];

    // Lire toutes les lignes
    while (fgets(line, sizeof(line), fin)) {
        char* fields[10];
        int nb_fields = split_line(line, fields, 10);

        if (nb_fields < 5) continue;

        // Condition: champ 1 = "-", champs 2, 3, 4 != "-"
        if (is_dash(fields[0]) && !is_dash(fields[1]) && !is_dash(fields[2]) && !is_dash(fields[3])) {
            double volume = atof(fields[3]);
            add_entry(fields[2], volume);
        }
    }
    fclose(fin);

    // Écrire le résultat (non trié, le script bash le fera)
    FILE* fout = fopen(csvfile, "w");
    if (!fout) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", csvfile);
        exit(1);
    }

    fprintf(fout, "identifier;volume\n");
    for (int i = 0; i < nb_entries; i++) {
        fprintf(fout, "%s;%.0f\n", entries[i].id, entries[i].volume);
    }

    fclose(fout);
    printf("Fichier généré: %s (%d sources)\n", csvfile, nb_entries);
}

// MODE REEL : volume réel = volume * (100 - pourcentage) / 100
void process_reel(const char* datafile, const char* csvfile) {
    FILE* fin = fopen(datafile, "r");
    if (!fin) {
        fprintf(stderr, "Erreur: impossible d'ouvrir '%s'\n", datafile);
        exit(1);
    }

    printf("Mode REEL : Calcul des volumes réels par source\n");

    char line[MAX_LINE];
    
    // Ignorer la première ligne (en-tête)
    if (fgets(line, sizeof(line), fin)) {}

    // Lire toutes les lignes
    while (fgets(line, sizeof(line), fin)) {
        char* fields[10];
        int nb_fields = split_line(line, fields, 10);

        if (nb_fields < 5) continue;

        // Condition: champ 1 = "-", champs 2, 3, 4, 5 != "-"
        if (is_dash(fields[0]) && !is_dash(fields[1]) && !is_dash(fields[2]) && 
            !is_dash(fields[3]) && !is_dash(fields[4])) {
            
            double volume = atof(fields[3]);
            double percentage = atof(fields[4]);
            double volume_reel = volume * (100.0 - percentage) / 100.0;
            add_entry(fields[2], volume_reel);
        }
    }
    fclose(fin);

    // Écrire le résultat (non trié, le script bash le fera)
    FILE* fout = fopen(csvfile, "w");
    if (!fout) {
        fprintf(stderr, "Erreur: impossible de créer '%s'\n", csvfile);
        exit(1);
    }

    fprintf(fout, "identifier;volume\n");
    for (int i = 0; i < nb_entries; i++) {
        fprintf(fout, "%s;%.2f\n", entries[i].id, entries[i].volume);
    }

    fclose(fout);
    printf("Fichier généré: %s (%d sources)\n", csvfile, nb_entries);
}

// MAIN
int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <fichier.dat> <fichier.csv> {maxC|srcC|reelC}\n", argv[0]);
        return 1;
    }

    char* datafile = argv[1];
    char* csvfile = argv[2];
    char* mode = argv[3];

    // Déterminer le mode (enlever le 'C' à la fin)
    if (strcmp(mode, "maxC") == 0) {
        process_max(datafile, csvfile);
    }
    else if (strcmp(mode, "srcC") == 0) {
        process_src(datafile, csvfile);
    }
    else if (strcmp(mode, "reelC") == 0) {
        process_reel(datafile, csvfile);
    }
    else {
        fprintf(stderr, "Mode inconnu: %s\n", mode);
        fprintf(stderr, "Modes valides: maxC, srcC, reelC\n");
        return 1;
    }

    return 0;
}
