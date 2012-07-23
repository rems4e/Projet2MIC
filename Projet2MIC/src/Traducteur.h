/*
 *  Traducteur.h
 *  Jeu C++
 *
 *  Created by Rémi on 26/06/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef TRADUCTEUR_H
#define TRADUCTEUR_H

#include <string>
#include "Unichar.h"
#include <unordered_map>
#include <functional>
#include <exception>

class Traducteur {
public:
	class Exc_FichierLangue : public std::exception {
	public:
		Exc_FichierLangue(std::string const &v) throw() : std::exception(), _valeur(v) {
			std::cerr << this->what() << std::endl;
		}
		virtual ~Exc_FichierLangue() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};

	Traducteur(std::string const &fichier) throw(Exc_FichierLangue);
	~Traducteur();
	
	template<typename... Args>
	Unichar traduction(Unichar const &u, Args... args) const {
		Unichar const &format = this->valeurPourCle(u);
		return Unichar(format, args...);
	}
	
protected:
	typedef std::unordered_map<Unichar const, Unichar, std::hash<std::string>> Conteneur;
	
	Unichar const &valeurPourCle(Unichar const &u) const;

	Conteneur _traductions;
	std::string const _fichier;
};

#endif
