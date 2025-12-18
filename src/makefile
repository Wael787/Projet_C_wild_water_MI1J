# Configuration 
CC = gcc
# -Iinclude permet de trouver histo.h dans le dossier include
CFLAGS = -Wall -Wextra -Iinclude
# -lm pour la bibliothèque mathématique (round)
LIBS = -lm
BIN_DIR = bin
SRC = histo.c
EXEC = $(BIN_DIR)/histo

# RÈGLES

# La règle par défaut (tapée via 'make')
all: $(EXEC)

# Création de l'exécutable
$(EXEC): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

# Nettoyage des fichiers générés
clean:
	rm -rf $(BIN_DIR) csv/*.csv csv/*.png

# Règle pour éviter les conflits avec des fichiers du même nom
.PHONY: all clean
