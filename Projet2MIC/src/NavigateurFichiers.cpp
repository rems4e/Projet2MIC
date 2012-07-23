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
#include <sys/stat.h>

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

		DIR *dossier = opendir(chemin.c_str());
		struct dirent *el = 0;
		while((el = readdir(dossier))) {
			struct stat st; 
			stat(el->d_name, &st); 
			if(S_ISDIR(st.st_mode)) {
				if(el->d_name[0] != '.')
					dossiers.push_back(el->d_name);
			}
		}
		closedir(dossier);
		
		std::sort(dossiers.begin(), dossiers.end());
				
		return dossiers;
	}
	
	std::vector<std::string> listeFichiers(std::string const &chemin, std::vector<std::string> const &extensions) {
		std::vector<std::string> fichiers;
		
		DIR *dossier = opendir(chemin.c_str());
		struct dirent *el = 0;
		while((el = readdir(dossier))) {
			struct stat st; 
			stat(el->d_name, &st); 
			if(S_ISREG(st.st_mode)) {
				if(el->d_name[0] != '.')
					fichiers.push_back(el->d_name);
			}
		}
		closedir(dossier);
		
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
		
		std::sort(fichiers.begin(), fichiers.end());

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
