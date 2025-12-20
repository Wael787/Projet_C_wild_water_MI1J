#include "fuites.h"


 //FONCTIONS DE GESTION DES NŒUDS DE L'ARBRE DE DISTRIBUTION
 

Node *create_node(const char *identifier, double leak_percentage) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour le nœud\n");
        return NULL;
    }
    
    node->identifier = duplicate_string(identifier);
    if (!node->identifier) {
        free(node);
        return NULL;
    }
    
    node->volume = 0.0;
    node->leak_percentage = leak_percentage;
    node->children = NULL;
    node->num_children = 0;
    node->capacity_children = 0;
    
    return node;
}

void add_child(Node *parent, Node *child) {
    if (!parent || !child) return;
    
    /* Agrandir le tableau si nécessaire */
    if (parent->num_children >= parent->capacity_children) {
        int new_capacity = parent->capacity_children == 0 ? 4 : parent->capacity_children * 2;
        Node **new_children = (Node **)realloc(parent->children, new_capacity * sizeof(Node *));
        if (!new_children) {
            fprintf(stderr, "Erreur: Réallocation mémoire échouée\n");
            return;
        }
        parent->children = new_children;
        parent->capacity_children = new_capacity;
    }
    
    parent->children[parent->num_children] = child;
    parent->num_children++;
}

void free_tree(Node *root) {
    if (!root) return;
    
    for (int i = 0; i < root->num_children; i++) {
        free_tree(root->children[i]);
    }
    
    free(root->identifier);
    free(root->children);
    free(root);
}


 //FONCTIONS DE GESTION DE L'AVL
 

AVLNode *create_avl_node(const char *identifier, Node *data_node) {
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));
    if (!node) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour le nœud AVL\n");
        return NULL;
    }
    
    node->identifier = duplicate_string(identifier);
    if (!node->identifier) {
        free(node);
        return NULL;
    }
    
    node->data_node = data_node;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    
    return node;
}

int avl_height(AVLNode *node) {
    return node ? node->height : 0;
}

int avl_max(int a, int b) {
    return (a > b) ? a : b;
}

int avl_balance_factor(AVLNode *node) {
    return node ? avl_height(node->left) - avl_height(node->right) : 0;
}

AVLNode *avl_rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;
    
    return x;
}

AVLNode *avl_rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = avl_max(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = avl_max(avl_height(y->left), avl_height(y->right)) + 1;
    
    return y;
}

AVLNode *avl_insert(AVLNode *root, const char *identifier, Node *data_node) {
    /* Insertion BST classique */
    if (!root) {
        return create_avl_node(identifier, data_node);
    }
    
    int cmp = strcmp(identifier, root->identifier);
    if (cmp < 0) {
        root->left = avl_insert(root->left, identifier, data_node);
    } else if (cmp > 0) {
        root->right = avl_insert(root->right, identifier, data_node);
    } else {
        /* Identifiant déjà existant, on ne fait rien */
        return root;
    }
    
    /* Mise à jour de la hauteur */
    root->height = 1 + avl_max(avl_height(root->left), avl_height(root->right));
    
    /* Calcul du facteur d'équilibre */
    int balance = avl_balance_factor(root);
    
    /* Cas de déséquilibre */
    /* Cas Gauche-Gauche */
    if (balance > 1 && strcmp(identifier, root->left->identifier) < 0) {
        return avl_rotate_right(root);
    }
    
    /* Cas Droite-Droite */
    if (balance < -1 && strcmp(identifier, root->right->identifier) > 0) {
        return avl_rotate_left(root);
    }
    
    /* Cas Gauche-Droite */
    if (balance > 1 && strcmp(identifier, root->left->identifier) > 0) {
        root->left = avl_rotate_left(root->left);
        return avl_rotate_right(root);
    }
    
    /* Cas Droite-Gauche */
    if (balance < -1 && strcmp(identifier, root->right->identifier) < 0) {
        root->right = avl_rotate_right(root->right);
        return avl_rotate_left(root);
    }
    
    return root;
}

Node *avl_search(AVLNode *root, const char *identifier) {
    if (!root) return NULL;
    
    int cmp = strcmp(identifier, root->identifier);
    if (cmp == 0) {
        return root->data_node;
    } else if (cmp < 0) {
        return avl_search(root->left, identifier);
    } else {
        return avl_search(root->right, identifier);
    }
}

void free_avl(AVLNode *root) {
    if (!root) return;
    
    free_avl(root->left);
    free_avl(root->right);
    free(root->identifier);
    free(root);
}


//FONCTIONS DE CALCUL
 

void calculate_volumes(Node *node, double parent_volume, int num_siblings) {
    if (!node) return;
    
    /* Calcul du volume pour ce nœud */
    if (num_siblings > 0) {
        /* Répartition équitable entre les frères */
        double volume_before_leak = parent_volume / num_siblings;
        /* Application des fuites */
        node->volume = volume_before_leak * (100.0 - node->leak_percentage) / 100.0;
    } else {
        node->volume = parent_volume * (100.0 - node->leak_percentage) / 100.0;
    }
    
    /* Propagation récursive aux enfants */
    for (int i = 0; i < node->num_children; i++) {
        calculate_volumes(node->children[i], node->volume, node->num_children);
    }
}

double calculate_total_leaks(Node *node) {
    if (!node) return 0.0;
    
    double total_leaks = 0.0;
    
    /* Calcul des fuites pour ce tronçon */
    /* Les fuites sont calculées sur le volume AVANT application du pourcentage de fuites */
    /* Pour obtenir ce volume, on inverse le calcul : volume_avant = volume_après / (1 - %/100) */
    /* Mais plus simplement : fuites = volume_après * (%/(100-%)) */
    /* Ou encore : volume_avant = volume_après + fuites, donc fuites = volume_avant * %/100 */
    
    /* En fait, si volume_après = volume_avant * (100 - %) / 100 */
    /* Alors volume_avant = volume_après * 100 / (100 - %) */
    /* Et fuites = volume_avant - volume_après */
    
    if (node->leak_percentage > 0 && node->leak_percentage < 100) {
        double volume_before = node->volume * 100.0 / (100.0 - node->leak_percentage);
        double leak_volume = volume_before - node->volume;
        total_leaks += leak_volume;
    }
    
    /* Somme récursive des fuites des enfants */
    for (int i = 0; i < node->num_children; i++) {
        total_leaks += calculate_total_leaks(node->children[i]);
    }
    
    return total_leaks;
}


//FONCTIONS UTILITAIRES


char *duplicate_string(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char *dup = (char *)malloc((len + 1) * sizeof(char));
    if (!dup) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour la chaîne\n");
        return NULL;
    }
    
    strcpy(dup, str);
    return dup;
}

char *trim_whitespace(char *str) {
    if (!str) return NULL;
    
    /* Suppression des espaces au début */
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    if (*str == 0) return str;
    
    /* Suppression des espaces à la fin */
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    *(end + 1) = '\0';
    
    return str;
}

int parse_csv_line(char *line, char *col1, char *col2, char *col3, char *col4, char *col5) {
    char *token;
    int col_index = 0;
    
    /* Initialisation des colonnes avec "-" */
    strcpy(col1, "-");
    strcpy(col2, "-");
    strcpy(col3, "-");
    strcpy(col4, "-");
    strcpy(col5, "-");
    
    /* Suppression du retour à la ligne */
    line[strcspn(line, "\r\n")] = 0;
    
    /* Parse des colonnes séparées par ';' */
    token = strtok(line, ";");
    while (token != NULL && col_index < 5) {
        token = trim_whitespace(token);
        
        switch(col_index) {
            case 0: strncpy(col1, token, MAX_IDENTIFIER_LENGTH - 1); break;
            case 1: strncpy(col2, token, MAX_IDENTIFIER_LENGTH - 1); break;
            case 2: strncpy(col3, token, MAX_IDENTIFIER_LENGTH - 1); break;
            case 3: strncpy(col4, token, MAX_IDENTIFIER_LENGTH - 1); break;
            case 4: strncpy(col5, token, MAX_IDENTIFIER_LENGTH - 1); break;
        }
        
        col_index++;
        token = strtok(NULL, ";");
    }
    
    return col_index;
}

/* ============================================================================
 * FONCTION PRINCIPALE
 * ============================================================================ */

int main(int argc, char *argv[]) {
    /* Vérification des arguments */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <fichier_donnees> <fichier_sortie> <identifiant_usine>\n", argv[0]);
        return 1;
    }
    
    const char *data_file = argv[1];
    const char *output_file = argv[2];
    const char *facility_id = argv[3];
    
    /* Ouverture du fichier de données */
    FILE *fp = fopen(data_file, "r");
    if (!fp) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", data_file);
        return 2;
    }
    
    /* Structures de données */
    AVLNode *avl_root = NULL;
    Node **storage_roots = NULL;  /* Tableau des racines (stockages) */
    int num_storages = 0;
    int capacity_storages = 0;
    
    char line[MAX_LINE_LENGTH];
    char col1[MAX_IDENTIFIER_LENGTH], col2[MAX_IDENTIFIER_LENGTH];
    char col3[MAX_IDENTIFIER_LENGTH], col4[MAX_IDENTIFIER_LENGTH], col5[MAX_IDENTIFIER_LENGTH];
    
    double total_treated_volume = 0.0;
    int facility_found = 0;
    
    /* Phase 1: Calcul du volume total traité par l'usine */
    while (fgets(line, sizeof(line), fp)) {
        parse_csv_line(line, col1, col2, col3, col4, col5);
        
        /* SOURCE → USINE */
        if (strcmp(col1, "-") == 0 && strcmp(col3, facility_id) == 0 && strcmp(col4, "-") != 0) {
            facility_found = 1;
            double source_volume = atof(col4);
            double leak_percent = atof(col5);
            double treated_volume = source_volume * (100.0 - leak_percent) / 100.0;
            total_treated_volume += treated_volume;
        }
    }
    
    /* Si l'usine n'est pas trouvée, retourner -1 */
    if (!facility_found) {
        fclose(fp);
        FILE *out = fopen(output_file, "w");
        if (out) {
            fprintf(out, "identifier;Leak volume (M.m3.year-1)\n");
            fprintf(out, "%s;-1\n", facility_id);
            fclose(out);
        }
        return 0;
    }
    
    /* Phase 2: Construction de l'arbre de distribution */
    rewind(fp);
    
    while (fgets(line, sizeof(line), fp)) {
        parse_csv_line(line, col1, col2, col3, col4, col5);
        
        /* USINE → STOCKAGE */
        if (strcmp(col1, "-") == 0 && strcmp(col2, facility_id) == 0 && strcmp(col3, "-") != 0 && strcmp(col5, "-") != 0) {
            double leak_percent = atof(col5);
            Node *storage_node = create_node(col3, leak_percent);
            
            if (storage_node) {
                /* Ajouter à la liste des racines */
                if (num_storages >= capacity_storages) {
                    int new_capacity = capacity_storages == 0 ? 4 : capacity_storages * 2;
                    Node **new_roots = (Node **)realloc(storage_roots, new_capacity * sizeof(Node *));
                    if (new_roots) {
                        storage_roots = new_roots;
                        capacity_storages = new_capacity;
                    }
                }
                storage_roots[num_storages++] = storage_node;
                
                /* Ajouter à l'AVL */
                avl_root = avl_insert(avl_root, col3, storage_node);
            }
        }
        /* STOCKAGE → JONCTION, JONCTION → RACCORDEMENT, RACCORDEMENT → USAGER */
        else if (strcmp(col1, facility_id) == 0 && strcmp(col2, "-") != 0 && strcmp(col3, "-") != 0 && strcmp(col5, "-") != 0) {
            double leak_percent = atof(col5);
            
            /* Rechercher le parent dans l'AVL */
            Node *parent = avl_search(avl_root, col2);
            if (parent) {
                /* Créer le nœud enfant */
                Node *child = create_node(col3, leak_percent);
                if (child) {
                    add_child(parent, child);
                    /* Ajouter à l'AVL pour recherches futures */
                    avl_root = avl_insert(avl_root, col3, child);
                }
            }
        }
    }
    
    fclose(fp);
    
    /* Phase 3: Calcul des volumes dans l'arbre */
    for (int i = 0; i < num_storages; i++) {
        calculate_volumes(storage_roots[i], total_treated_volume, num_storages);
    }
    
    /* Phase 4: Calcul du total des fuites */
    double total_leaks = 0.0;
    for (int i = 0; i < num_storages; i++) {
        total_leaks += calculate_total_leaks(storage_roots[i]);
    }
    
    /* Conversion en M.m³ (millions de m³) */
    double total_leaks_Mm3 = total_leaks / 1000.0;
    
    /* Phase 5: Écriture du résultat */
    FILE *out = fopen(output_file, "w");
    if (!out) {
        fprintf(stderr, "Erreur: Impossible de créer le fichier %s\n", output_file);
        /* Libération de la mémoire */
        for (int i = 0; i < num_storages; i++) {
            free_tree(storage_roots[i]);
        }
        free(storage_roots);
        free_avl(avl_root);
        return 3;
    }
    
    fprintf(out, "identifier;Leak volume (M.m3.year-1)\n");
    fprintf(out, "%s;%.6f\n", facility_id, total_leaks_Mm3);
    fclose(out);
    
    /* Libération de la mémoire */
    for (int i = 0; i < num_storages; i++) {
        free_tree(storage_roots[i]);
    }
    free(storage_roots);
    free_avl(avl_root);
    
    return 0;
}
