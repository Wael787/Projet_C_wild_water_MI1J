# version finale
# date : 21/12/2025

CC = gcc
CFLAGS = -Wall -Wextra -O2
BINDIR = bin
SRCDIR = src

$(BINDIR):
	mkdir -p $(BINDIR)

all: histo leaks

histo: $(SRCDIR)/histo.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/histo $(SRCDIR)/histo.c

leaks: $(SRCDIR)/leaks.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/leaks $(SRCDIR)/leaks.c

clean:
	rm -rf $(BINDIR)

.PHONY: all clean histo leaks
