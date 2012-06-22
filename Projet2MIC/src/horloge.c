//
//  horloge.c
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "horloge.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <SDL/SDL.h>

horloge_t horloge() {
#ifdef __WIN32__
	return SDL_GetTicks() / 1000.0;
#else
	static struct timeval refheure = {.tv_sec = -1, .tv_usec = 0};
	if(refheure.tv_sec == -1)
		gettimeofday(&refheure, 0);
	struct timeval temp;
	gettimeofday(&temp, 0);
	
	return (horloge_t)(temp.tv_sec - refheure.tv_sec + (temp.tv_usec - refheure.tv_usec) / 1.0E6);
#endif
}

void attendre(horloge_t sec) {
	if(sec < 0)
		return;
#ifdef __WIN32__
	SDL_Delay(sec * 1000);
#else
	struct timespec a = {.tv_sec = (long)sec, .tv_nsec = (long)(((double)sec - (long)sec) * 1E9)};
	nanosleep(&a, 0);
#endif
}
