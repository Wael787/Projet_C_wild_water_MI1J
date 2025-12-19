CC = gcc
CFLAGS = -Wall -Wextra -O2

BIN = bin
SRC = src

all: $(BIN)/histo $(BIN)/leaks

$(BIN)/histo: $(SRC)/histo.c
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN)/leaks: $(SRC)/leaks.c
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BIN)
