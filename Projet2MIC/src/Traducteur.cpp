/*
 *  Traduction.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 26/06/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "Traducteur.h"
#include <fstream>
#include <list>

Traducteur::Traducteur(std::string const &fichier) throw(Traducteur::Exc_FichierLangue) : _traductions(), _fichier(fichier) {
	std::ifstream f(fichier.c_str());
	
	std::string ff;
	char c;
	while(f.get(c))
		ff.push_back(c);
	
	if(ff.empty()) {
		f.close();
		return;
	}
	
	std::list<std::string> lignes;
	std::string::iterator pos = ff.begin();
	for(std::string::iterator i = ff.begin(); i != ff.end(); ++i) {
		if(*i == '\n') {
			lignes.push_back(std::string(pos, i));
			pos = i + 1;
		}
	}
	if(pos < ff.end())
		lignes.push_back(std::string(pos, ff.end()));
	
	for(std::list<std::string>::iterator i = lignes.begin(); i != lignes.end(); ++i) {
		if(i->empty() || (*i)[0] != '"')
			continue;
		std::string cle, valeur;
		std::string *p = &cle;
		for(Unichar::uindex_t j = 1; j != i->size(); ++j) {
			if((*i)[j] == '\\') {
				if(j + 1 == i->size())
					throw Exc_FichierLangue("Fichier de traduction malformé : '\\' trouvé (" + fichier + ":l." + nombreVersTexte(std::distance(lignes.begin(), i)) + ").");
				
				switch((*i)[j + 1]) {
					case '\\':
					case '"':
						p->push_back((*i)[j + 1]);
						break;
					case 't':
						p->push_back('\t');
						break;
					case 'n':
						p->push_back('\n');
						break;
					default:
						throw Exc_FichierLangue(std::string("Fichier de traduction malformé : '\\") + (*i)[j + 1] + "' trouvé (" + fichier + ":l." + nombreVersTexte(std::distance(lignes.begin(), i)) + ").");
				}
				++j;
				continue;
			}
			else if((*i)[j] == '"') {
				if(p == &cle) {
					do {
						++j;
					} while((*i)[j] == ' ' || (*i)[j] == '\t');
					if((*i)[j] != '=') {
						throw Exc_FichierLangue("Fichier de traduction malformé : '=' attendu (" + fichier + ":l." + nombreVersTexte(std::distance(lignes.begin(), i)) + ").");
					}
					
					do {
						++j;
					} while((*i)[j] == ' ' || (*i)[j] == '\t');
					if((*i)[j] != '"') {
						throw Exc_FichierLangue("Fichier de traduction malformé : '\"' attendu (" + fichier + ":l." + nombreVersTexte(std::distance(lignes.begin(), i)) + ").");
					}
					
					p = &valeur;
				}
				else if(p == &valeur) {
					do {
						++j;
					} while((*i)[j] == ' ' || (*i)[j] == '\t');
					if((*i)[j] != ';') {
						throw Exc_FichierLangue("Fichier de traduction malformé : ';' attendu (" + fichier + ":l." + nombreVersTexte(std::distance(lignes.begin(), i)) + ").");
					}
				}
			}
			else {
				p->push_back((*i)[j]);
			}
		}
		
		std::pair<Conteneur::iterator, bool> pos = _traductions.insert(std::make_pair(cle, valeur));
		if(!pos.second) {
			std::cerr << "Le fichier de traduction " << fichier << " contient la clé " << cle << " plus d'une fois !" << std::endl;
		}
	}
	f.close();
}

Traducteur::~Traducteur() {
	
}

Unichar const &Traducteur::valeurPourCle(Unichar const &cle) const {
	Conteneur::const_iterator pos = _traductions.find(cle);
	if(pos == _traductions.end()) {
		std::cout << "Aucune traduction pour \"" << cle << "\" = \"" << cle << "\";" << std::endl;
		return cle;
	}
	else {
		return pos->second;
	}
}
