#
# WebServer, a small and light web server.
# 
# Copyright (C) 2014 Felipe Paul Martins and Joachim Schmidt
# 
# This file is part of WebServer.
# 
# WebServer is free software: you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# 
# WebServer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# 
# See the GNU General Public License for more details. You should have received a copy of the GNU
# General Public License along with WebServer. If not, see <http://www.gnu.org/licenses/>.
# 
# Authors: Felipe Paul Martins, Joachim Schmidt
#

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
CFLAGS=-W -Wall -ansi -pedantic -g -ggdb -pg -std=gnu99 -Wno-unused-but-set-variable -Wno-pointer-to-int-cast $(INCLUDES_FOLDERS)
LDFLAGS=
EXEC=webserver

all: $(EXEC)

webserver: main.o options.o socket.o sig_handler.o process_management.o http.o local_time.o histo.o log_process.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c ./include/options.h ./include/server_const.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

options.o: options.c ./include/options.h ./include/server_const.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE_OPT) $(DEFINE)

socket.o: socket.c ./include/socket.h ./include/process_management.h ./include/server_const.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

process_management.o: process_management.c ./include/process_management.h ./include/server_const.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

sig_handler.o: sig_handler.c ./include/sig_handler.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

http.o: http.c ./include/http.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

local_time.o: local_time.c ./include/local_time.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

histo.o: histo.c ./include/histo.h ./include/local_time.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

log_process.o: log_process.c ./include/log_process.h ./include/histo.h ./include/process_management.h
	$(CC) -o $@ -c $< $(CFLAGS) $(DEFINE)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)