/**
 * WebServer, a small and light web server.
 * 
 * Copyright (C) 2014 Felipe Paul Martins and Joachim Schmidt
 * 
 * This file is part of WebServer.
 * 
 * WebServer is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * WebServer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with WebServer. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Authors: Felipe Paul Martins, Joachim Schmidt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "include/local_time.h"

/*
 * La fonction get_local_time crée et renvoi un buffer contenant la date et l'heure locale.
 * Le paramètre mode permet de modifier le formatage de la date.
 */

int get_local_time(char **buffer, int mode)
{
	char buf_tmp[50];
	int size;
	time_t temps;
	struct tm *tm;
	
	// Remplit la variable temps avec le nombre de seconde écoulée depuis le
	// 1er janvier 1970.
	if((time(&temps)) == (time_t)-1)
	{
		perror("time");
		return -1;
	}

	// Remplit les champs de la structure tm en fonction du temps en seconde écoulé
	// depuis le 1er janvier 1970.
	if((tm = localtime(&temps)) == NULL)
	{
		perror("localtime");
		return -1;
	}

	// Chois du mode pour de formatage.
	if(mode == 0)
		sprintf(buf_tmp, "[localtime = %02d/%02d/%02d - %02d:%02d:%02d - (%s)]",
				tm->tm_mday, tm->tm_mon + 1, tm->tm_year % 100,
				tm->tm_hour, tm->tm_min, tm->tm_sec,
				(tm->tm_isdst > 0) ? "Eté" : (tm->tm_isdst == 0) ? "Hiver" : "Undef");
	else if(mode == 1)
		sprintf(buf_tmp, "%02d%02d%02d_%02d%02d%02d",
				tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec);

	// Permet de s'assurer que buffer n'est pas déjà initialisé. Si c'est le
	// cas l'assertion n'est pas respecté et le programme s'arrête en laissant
	// un message d'erreur.
	assert(*buffer == NULL);

	size = strlen(buf_tmp);

	// Création du buffer contenant la date.
	if((*buffer = calloc(size + 1, sizeof(char))) == NULL)
	{
		perror("calloc");
		return -1;
	}

	strcpy(*buffer, buf_tmp);

	// On retroune la taille du buffer.
	return size;
}
