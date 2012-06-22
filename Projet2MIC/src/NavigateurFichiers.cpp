/*
 *  NavigateurFichiers.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 08/05/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "NavigateurFichiers.h"
#include <dirent.h>
#include <algorithm>

namespace NavigateurFichiers {
	std::vector<std::string> const &extensionsImage() {
		static std::vector<std::string> imgs;
		
		if(!imgs.size()) {
			imgs.push_back("jpg");
			imgs.push_back("jp2");
			imgs.push_back("png");
			imgs.push_back("tiff");
			imgs.push_back("tif");
			imgs.push_back("gif");
			imgs.push_back("bmp");
		}
		
		return imgs;
	}

	std::vector<std::string> listeDossiers(std::string const &chemin) {
		std::vector<std::string> dossiers;
		char const *emplacement = chemin.c_str();
		struct dirent **liste;
		int nombreElements;
		nombreElements = scandir(emplacement, &liste, 0, alphasort);
		for(int j = 0; j < nombreElements; ++j) {
			if(liste[j]->d_type == DT_DIR) {
				if(liste[j]->d_name[0] != '.')
					dossiers.push_back(liste[j]->d_name);
			}
			free(liste[j]);
		}
		if(nombreElements != -1)
			free(liste);
		
		return dossiers;
	}
	
	std::vector<std::string> listeFichiers(std::string const &chemin, std::vector<std::string> const &extensions) {
		std::vector<std::string> fichiers;
		char const *emplacement = chemin.c_str();
		struct dirent **liste;
		int nombreElements;
		nombreElements = scandir(emplacement, &liste, 0, alphasort);
		for(int j = 0; j < nombreElements; ++j) {
			if(liste[j]->d_type == DT_REG) {
				if(liste[j]->d_name[0] != '.')
					fichiers.push_back(liste[j]->d_name);
			}
			free(liste[j]);
		}
		if(nombreElements != -1)
			free(liste);
		
		if(!extensions.empty()) {
			for(unsigned int i = 0; i < fichiers.size(); ++i) {
				size_t pos = fichiers[i].rfind(".");
				if(pos != std::string::npos && pos != fichiers[i].size() - 1) {
					std::string ext = fichiers[i].substr(pos + 1);
					if(std::find(extensions.begin(), extensions.end(), ext) == extensions.end()) {
						fichiers.erase(fichiers.begin() + i);
						--i;
						continue;
					}
				}
				else {
					fichiers.erase(fichiers.begin() + i);
					--i;
					continue;
				}
			}
		}
		
		return fichiers;
	}
	
	std::vector<std::string> listeFichiers(std::vector<std::string> const &chemins, std::vector<std::string> const &extensions) {
		std::vector<std::string> fichiers;
		for(std::vector<std::string>::const_iterator i = chemins.begin(); i != chemins.end(); ++i) {
			std::vector<std::string> retour = listeFichiers(*i, extensions);
			fichiers.insert(fichiers.end(), retour.begin(), retour.end());
		}
		
		return fichiers;
	}

}
