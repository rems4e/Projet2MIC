//
//  Partie.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 07/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Partie_h
#define Projet2MIC_Partie_h

#include <string>

class Niveau;
class Joueur;

class Partie {
public:
	Partie();
	Partie(std::string const &sauvegarde);
	Partie(int numeroSecteur);
	virtual ~Partie();
	
	void commencer();
	
	void reinitialiser();
	
protected:
	void afficher();
	void animer();
	void gestionEvenements();
	
	Niveau *_niveau;
	Joueur *_joueur;
};

#endif
