#--------------------------------------------------
# MAKEFILE
#
# Formation ITI soir, 2ème année.
# Cours : Programmation système.
# Réalisation d'un mini serveur web en C.
#
# Début : 11 mars 2014
# Fin   : 20 mai 2014
#
# Auteurs : Felipe Paul Martins et Joachim Schmidt
#--------------------------------------------------

CC=gcc
INCLUDES_FOLDERS=-I./include/
DEFINE_OPT=-DOPTIONS_LONGUES 
DEFINE=-D_GNU_SOURCES
CFLAGS=-W -Wall -ansi -pedantic -g -ggdb -pg -std=gnu99 $(INCLUDES_FOLDERS)
LDFLAGS=
EXEC=webserver

all: $(EXEC)

webserver: main.o options.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c ./include/options.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

options.o: options.c ./include/options.h 
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE_OPT) $(DEFINE)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)