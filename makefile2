# Makefile pour le projet C-Wildwater

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = -lm

# Répertoires
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Fichiers sources
SOURCES = main.c histo.c fuites.c acteur.c avl_index.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)

# Exécutables
EXEC_HISTO = $(BINDIR)/histo
EXEC_LEAKS = $(BINDIR)/leaks
EXEC_MAIN = $(BINDIR)/wildwater

# Cible par défaut
all: directories $(EXEC_MAIN) liens

# Créer les répertoires nécessaires
directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p csv
	@mkdir -p dat

# Compilation de l'exécutable principal
$(EXEC_MAIN): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Compilation réussie : $(EXEC_MAIN)"

# Création des liens symboliques pour histo et leaks
liens: $(EXEC_MAIN)
	@ln -sf wildwater $(EXEC_HISTO)
	@ln -sf wildwater $(EXEC_LEAKS)
	@echo "Liens symboliques créés"

# Règle générique pour compiler les fichiers .c en .o
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dépendances spécifiques
$(OBJDIR)/main.o: main.c codes_erreur.h histo.h fuites.h
$(OBJDIR)/histo.o: histo.c histo.h codes_erreur.h
$(OBJDIR)/fuites.o: fuites.c fuites.h acteur.h avl_index.h codes_erreur.h
$(OBJDIR)/acteur.o: acteur.c acteur.h codes_erreur.h
$(OBJDIR)/avl_index.o: avl_index.c avl_index.h acteur.h codes_erreur.h

# Nettoyage
clean:
	rm -rf $(OBJDIR)
	rm -f $(EXEC_MAIN) $(EXEC_HISTO) $(EXEC_LEAKS)
	@echo "Nettoyage effectué"

# Nettoyage complet (avec fichiers de sortie)
mrproper: clean
	rm -rf csv/*.csv
	rm -rf dat/*.dat
	@echo "Nettoyage complet effectué"

# Aide
help:
	@echo "Makefile pour le projet C-Wildwater"
	@echo ""
	@echo "Cibles disponibles :"
	@echo "  all       : Compile le projet (par défaut)"
	@echo "  clean     : Supprime les fichiers objets et exécutables"
	@echo "  mrproper  : Nettoyage complet (clean + fichiers de sortie)"
	@echo "  help      : Affiche cette aide"

.PHONY: all directories clean mrproper help liens
