CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
BINDIR = bin
SRCDIR = .

# Créer le répertoire bin s'il n'existe pas
$(BINDIR):
	mkdir -p $(BINDIR)

# Cible par défaut
all: histo

# Compilation de histo
histo: $(SRCDIR)/histo.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/histo $(SRCDIR)/histo.c

# Compilation de leaks (si vous avez ce fichier)
leaks: $(SRCDIR)/fuites.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/leaks $(SRCDIR)/leaks.c

# Nettoyage
clean:
	rm -rf $(BINDIR)

.PHONY: all clean histo leaks
