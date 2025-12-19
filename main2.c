#include "leaks.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <fichier.csv> <id_usine>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) return 2;

    NoeudAVL* index = NULL;
    NoeudReseau* usine = NULL;
    char ligne[1024];

    while (fgets(ligne, 1024, f)) {
        char id_am[50], id_av[50];
        double vol, fuite;
        if (sscanf(ligne, "%[^;];%[^;];%lf;%lf", id_am, id_av, &vol, &fuite) == 4) {
            NoeudReseau *amont, *aval;
            index = inserer_avl(index, id_am, &amont);
            index = inserer_avl(index, id_av, &aval);

            if (strcmp(id_am, argv[2]) == 0 && usine == NULL) {
                usine = amont;
                usine->volume_entrant = vol;
            }
            ajouter_enfant(amont, aval, fuite);
        }
    }

    if (usine) {
        double total_pertes = 0.0;
        double vol_init = usine->volume_entrant;
        calculer_pertes_recursif(usine, &total_pertes);

        printf("--- RÉSULTATS ---\n");
        printf("Volume traité  : %.3f Mm3\n", vol_init / 1000.0);
        printf("Pertes totales : %.3f Mm3\n", total_pertes / 1000.0);
        printf("Rendement      : %.2f %%\n", (1.0 - (total_pertes / vol_init)) * 100.0);
    } else {
        printf("Erreur : Usine %s introuvable.\n", argv[2]);
    }

    liberer_avl(index);
    fclose(f);
    return 0;
}
