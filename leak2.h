#include "leaks.h"

// --- UTILITAIRES AVL ---
int h(NoeudAVL* n) { return n ? n->hauteur : 0; }
int get_eq(NoeudAVL* n) { return n ? h(n->fg) - h(n->fd) : 0; }

NoeudAVL* rot_droite(NoeudAVL* y) {
    NoeudAVL* x = y->fg; NoeudAVL* T2 = x->fd;
    x->fd = y; y->fg = T2;
    y->hauteur = 1 + (h(y->fg) > h(y->fd) ? h(y->fg) : h(y->fd));
    x->hauteur = 1 + (h(x->fg) > h(x->fd) ? h(x->fg) : h(x->fd));
    return x;
}

NoeudAVL* rot_gauche(NoeudAVL* x) {
    NoeudAVL* y = x->fd; NoeudAVL* T2 = y->fg;
    y->fg = x; x->fd = T2;
    x->hauteur = 1 + (h(x->fg) > h(x->fd) ? h(x->fg) : h(x->fd));
    y->hauteur = 1 + (h(y->fg) > h(y->fd) ? h(y->fg) : h(y->fd));
    return y;
}

// --- GESTION MÉMOIRE ---
NoeudReseau* creer_noeud_reseau(const char* id) {
    NoeudReseau* n = malloc(sizeof(NoeudReseau));
    if(!n) exit(1);
    n->id = strdup(id);
    n->volume_entrant = 0.0;
    n->nb_enfants = 0;
    n->liste_fils = NULL;
    return n;
}

NoeudAVL* inserer_avl(NoeudAVL* node, const char* id, NoeudReseau** resultat) {
    if (!node) {
        *resultat = creer_noeud_reseau(id);
        NoeudAVL* nv = malloc(sizeof(NoeudAVL));
        nv->id = strdup(id); nv->ptr_noeud = *resultat;
        nv->fg = nv->fd = NULL; nv->hauteur = 1;
        return nv;
    }
    int cmp = strcmp(id, node->id);
    if (cmp < 0) node->fg = inserer_avl(node->fg, id, resultat);
    else if (cmp > 0) node->fd = inserer_avl(node->fd, id, resultat);
    else { *resultat = node->ptr_noeud; return node; }

    node->hauteur = 1 + (h(node->fg) > h(node->fd) ? h(node->fg) : h(node->fd));
    int eq = get_eq(node);
    if (eq > 1 && strcmp(id, node->fg->id) < 0) return rot_droite(node);
    if (eq < -1 && strcmp(id, node->fd->id) > 0) return rot_gauche(node);
    if (eq > 1 && strcmp(id, node->fg->id) > 0) { node->fg = rot_gauche(node->fg); return rot_droite(node); }
    if (eq < -1 && strcmp(id, node->fd->id) < 0) { node->fd = rot_droite(node->fd); return rot_gauche(node); }
    return node;
}

// --- LOGIQUE RÉSEAU ---
void ajouter_enfant(NoeudReseau* parent, NoeudReseau* enfant, double fuite) {
    Enfant* e = malloc(sizeof(Enfant));
    e->noeud = enfant; e->taux_fuite = fuite;
    e->suivant = parent->liste_fils;
    parent->liste_fils = e;
    parent->nb_enfants++;
}

void calculer_pertes_recursif(NoeudReseau* n, double* total_pertes) {
    if (!n || n->nb_enfants == 0) return;
    double vol_par_fils = n->volume_entrant / n->nb_enfants;
    Enfant* curr = n->liste_fils;
    while (curr) {
        double perte = vol_par_fils * (curr->taux_fuite / 100.0);
        *total_pertes += perte;
        curr->noeud->volume_entrant = vol_par_fils - perte;
        calculer_pertes_recursif(curr->noeud, total_pertes);
        curr = curr->suivant;
    }
}

void liberer_avl(NoeudAVL* a) {
    if (!a) return;
    liberer_avl(a->fg);
    liberer_avl(a->fd);
    Enfant* curr = a->ptr_noeud->liste_fils;
    while(curr) { Enfant* tmp = curr; curr = curr->suivant; free(tmp); }
    free(a->ptr_noeud->id); free(a->ptr_noeud);
    free(a->id); free(a);
}
