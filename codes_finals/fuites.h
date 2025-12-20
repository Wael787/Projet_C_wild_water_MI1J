#ifndef FUITES_H
#define FUITES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_IDENTIFIER_LENGTH 256

/* Structure pour un nœud de l'arbre de distribution */
typedef struct Node {
    char *identifier;              
    double volume;                 
    double leak_percentage;        
    struct Node **children;        
    int num_children;              
    int capacity_children;         
} Node;

/* Structure pour un nœud AVL */
typedef struct AVLNode {
    char *identifier;
    Node *data_node;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
} AVLNode;

/* Gestion de l'arbre de distribution */
Node *create_node(const char *identifier, double leak_percentage);
void add_child(Node *parent, Node *child);
void free_tree(Node *root);

/* Gestion de l'AVL */
AVLNode *create_avl_node(const char *identifier, Node *data_node);
int avl_height(AVLNode *node);
int avl_max(int a, int b);
int avl_balance_factor(AVLNode *node);
AVLNode *avl_rotate_right(AVLNode *y);
AVLNode *avl_rotate_left(AVLNode *x);
AVLNode *avl_insert(AVLNode *root, const char *identifier, Node *data_node);
Node *avl_search(AVLNode *root, const char *identifier);
void free_avl(AVLNode *root);

/* Calcul des volumes et fuites */
void calculate_volumes(Node *node, double parent_volume, int num_siblings);
double calculate_total_leaks(Node *node);

/* Fonctions utilitaires */
int parse_csv_line(char *line, char *col1, char *col2, char *col3, char *col4, char *col5);
char *trim_whitespace(char *str);
char *duplicate_string(const char *str);

#endif
