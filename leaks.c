#include "leaks.h"
#include "codes_erreur.h"

// --- UTILITAIRES AVL ---
int max(int a, int b) { return (a > b) ? a : b; }
int h(NoeudAVL* n) { return n ? n->hauteur : 0; }

NoeudAVL* rotation_droite(NoeudAVL* y) {
    NoeudAVL* x = y->fg; y->fg = x->fd; x->fd = y;
    y->hauteur = max(h(y->fg), h(y->fd)) + 1;
    x->hauteur = max(h(x->fg), h(x->fd)) + 1;
    return x;
}

NoeudAVL* rotation_gauche(NoeudAVL* x) {
    NoeudAVL* y = x->fd; x->fd = y->fg; y->fg = x;
    x->hauteur = max(h(x->fg), h(x->fd)) + 1;
    y->hauteur = max(h(y->fg), h(y->fd)) + 1;
    return y;
}

NoeudAVL* inserer_avl(NoeudAVL* n, char* id, NoeudReseau* ptr) {
    if (!n) {
        NoeudAVL* nv = malloc(sizeof(NoeudAVL));
        if(!nv) exit(ERR_MEMOIRE);
        nv->id = strdup(id); nv->ptr_noeud = ptr; nv->hauteur = 1;
        nv->fg = nv->fd = NULL; return nv;
    }
    int cmp = strcmp(id, n->id);
    if (cmp < 0) n->fg = inserer_avl(n->fg, id, ptr);
    else if (cmp > 0) n->fd = inserer_avl(n->fd, id, ptr);
    else return n;

    n->hauteur = 1 + max(h(n->fg), h(n->fd));
    int eq = h(n->fg) - h(n->fd);
    if (eq > 1 && strcmp(id, n->fg->id) < 0) return rotation_droite(n);
    if (eq < -1 && strcmp(id, n->fd->id) > 0) return rotation_gauche(n);
    if (eq > 1 && strcmp(id, n->fg->id) > 0) { n->fg = rotation_gauche(n->fg); return rotation_droite(n); }
    if (eq < -1 && strcmp(id, n->fd->id) < 0) { n->fd = rotation_droite(n->fd); return rotation_gauche(n); }
    return n;
}

NoeudReseau* chercher_avl(NoeudAVL* n, char* id) {
    if (!n) return NULL;
    int cmp = strcmp(id, n->id);
    if (cmp == 0) return n->ptr_noeud;
    return (cmp < 0) ? chercher_avl(n->fg, id) : chercher_avl(n->fd, id);
}

// --- PASSE 2 : CALCUL RECURSIF DES FUITES ---
void calculer_rendement_aval(NoeudReseau* n, double* total_pertes) {
    if (n == NULL || n->nb_enfants == 0) return;
    double volume_par_enfant = n->volume_entrant / n->nb_enfants;
    NoeudReseau* enfant = n->premier_enfant;
    while (enfant != NULL) {
        double taux = enfant->volume_entrant; // Taux stocké temporairement
        double perte_troncon = volume_par_enfant * (taux / 100.0);
        *total_pertes += perte_troncon;
        enfant->volume_entrant = volume_par_enfant - perte_troncon;
        calculer_rendement_aval(enfant, total_pertes);
        enfant = enfant->frere_suivant;
    }
}

// --- PASSE 1 : CONSTRUCTION DU RESEAU ---
int traiter_leaks(const char* f_in, const char* f_out, const char* id_target) {
    FILE* in = fopen(f_in, "r");
    if (!in) return ERR_FICHIER_IN;

    char ligne[1024];
    NoeudAVL* index = NULL;
    NoeudReseau* racine = NULL;

    while (fgets(ligne, 1024, in)) {
        char *col[5]; char *tmp = strdup(ligne); char *ptr = tmp;
        for (int i = 0; i < 5; i++) col[i] = strtok_r((i == 0) ? ptr : NULL, ";", &ptr);

        if (strcmp(col[1], id_target) == 0 && !racine) {
            racine = malloc(sizeof(NoeudReseau));
            racine->id = strdup(id_target);
            racine->volume_entrant = (col[3] && strcmp(col[3], "-") != 0) ? atof(col[3]) : 0.0;
            racine->nb_enfants = 0; racine->premier_enfant = NULL;
            index = inserer_avl(index, racine->id, racine);
        }

        NoeudReseau* parent = chercher_avl(index, col[1]);
        if (parent && col[2] && strcmp(col[2], "-") != 0) {
            NoeudReseau* enfant = malloc(sizeof(NoeudReseau));
            enfant->id = strdup(col[2]);
            enfant->nb_enfants = 0; enfant->premier_enfant = NULL;
            enfant->frere_suivant = parent->premier_enfant;
            parent->premier_enfant = enfant;
            parent->nb_enfants++;
            enfant->volume_entrant = (col[4] && strcmp(col[4], "-") != 0) ? atof(col[4]) : 0.0;
            index = inserer_avl(index, enfant->id, enfant);
        }
        free(tmp);
    }

    double total_pertes = 0.0;
    if (racine) calculer_rendement_aval(racine, &total_pertes);

    FILE* out = fopen(f_out, "w");
    if (!racine) fprintf(out, "%s;-1\n", id_target);
    else fprintf(out, "%s;%.3f\n", id_target, total_pertes / 1000.0);

    fclose(in); fclose(out);
    liberer_avl(index); liberer_reseau(racine);
    return SUCCESS;
}

void liberer_reseau(NoeudReseau* n) {
    if (!n) return;
    liberer_reseau(n->premier_enfant);
    liberer_reseau(n->frere_suivant);
    free(n->id); free(n);
}

void liberer_avl(NoeudAVL* a) {
    if (!a) return;
    liberer_avl(a->fg); liberer_avl(a->fd);
    free(a->id); free(a);
}
