/*
 *  NavigateurFichiers.h
 *  Jeu C++
 *
 *  Created by Rémi on 08/05/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef NAVIGATEUR_FICHIERS_H
#define NAVIGATEUR_FICHIERS_H

#include <vector>
#include <string>

namespace NavigateurFichiers {
	std::vector<std::string> const &extensionsImage();
	
	std::vector<std::string> listeDossiers(std::string const &chemin);
	std::vector<std::string> listeFichiers(std::string const &chemin, std::vector<std::string> const &extensions = std::vector<std::string>());
	
	std::vector<std::string> listeFichiers(std::vector<std::string> const &chemins, std::vector<std::string> const &extensions = std::vector<std::string>());
}

#endif
