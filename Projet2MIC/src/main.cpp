//
//  main.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 31/01/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Constantes.h"
#include <string>
#include <iostream>
#include "SDL/SDL.h"
#include "fonctions.h"
#include "Session.h"
#include <cstdlib>
#include <ctime>

#ifdef __cplusplus
extern "C"
#endif


int main(int, char **) {
	std::srand(static_cast<unsigned int>(std::time(0)));
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Impossible d'initialiser la SDL : " << SDL_GetError() << std::endl;
		quitter(1);
	}
	SDL_EnableUNICODE(1);
	
	/* Initialise et lance le jeu */
	Session::initialiser();
	Session::menu();
	
	quitter(0);
	
	return 0;
}
