#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct arbre_usine{
	int v;//volume capté en litres de l'usine provenant des sources (m³)
	char* id;  //identifiant de l'usine	
}Usine;

typedef Usine * pUsine

typedef struct{
	int eq;
	pUsine fg;
	pUsine fd;
}AVL;

pUsine creerUsine(char *id, int volume){
pUsine new = malloc(sizeof(Usine));
if(!new){pUsine insertUsine(pUsine 
	exit(1);
}
new->v = volume;
strcpy(new->id,id);
new->fg = new->fd = NULL;
new->eq = 0;
return new;
}

pUsine insertUsine(AVL * avl ,pUsine u){
	if(!avl){
		return u;
	}
	
	
}






