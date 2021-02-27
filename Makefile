SDIR = src
IDIR = src
ODIR = obj
BDIR = bin

CC = gcc
CFLAGS = -Wextra -Wall -O3

LIBS = -lpcre2-8 -lcurl -lpthread

_DEPS = argparser.h csv.h help.h regexcheck.h verbose.h webrequest.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o argparser.o csv.o help.o regexcheck.o verbose.o webrequest.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BDIR)/csherlock: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean

clean:
	rm -rf $(BDIR)/csherlock
	rm -rf $(ODIR)
