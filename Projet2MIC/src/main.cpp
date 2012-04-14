//
//  main.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 31/01/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Session.h"

#ifdef __cplusplus
extern "C"
#endif


int main(int, char **) {	
	// Initialise et lance le jeu
	Session::initialiser();
	Session::menu();
	Session::nettoyer();
	
	return 0;
}
