#include "leaks.h"

int main(int argc, char** argv) {
    if (argc < 4) return 1; // datafile, temp_csv, usine_id

    const char* f_in_path = argv[1];
    const char* f_out_path = argv[2];
    const char* id_target = argv[3];

    FILE* f_in = fopen(f_in_path, "r");
    if (!f_in) return 2;

    NoeudAVL* index = NULL;
    NoeudReseau* usine = NULL;
    char ligne[1024];

    while (fgets(ligne, 1024, f_in)) {
        char am[64], av[64];
        double vol, fuite;
        // Format attendu: id_amont;id_aval;volume;fuite
        if (sscanf(ligne, "%63[^;];%63[^;];%lf;%lf", am, av, &vol, &fuite) == 4) {
            NoeudReseau *n_am, *n_av;
            index = inserer_avl(index, am, &n_am);
            index = inserer_avl(index, av, &n_av);

            if (strcmp(am, id_target) == 0 && usine == NULL) {
                usine = n_am;
                usine->volume_entrant = vol;
            }
            ajouter_enfant(n_am, n_av, fuite);
        }
    }
    fclose(f_in);

    if (usine) {
        double total_pertes = 0.0;
        calculer_pertes_recursif(usine, &total_pertes);

        FILE* f_out = fopen(f_out_path, "w");
        if (f_out) {
            // Écrit au format: identifier;Leak volume
            fprintf(f_out, "%s;%.3f\n", id_target, total_pertes);
            fclose(f_out);
        }
    } else {
        liberer_avl(index);
        return 3; // Usine introuvable
    }

    liberer_avl(index);
    return 0;
}
