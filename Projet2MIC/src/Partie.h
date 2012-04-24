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
#include "Geometrie.h"
#include <exception>


class Niveau;
class Joueur;
class TableauDeBord;
class Marchand;

class Partie {
public:
	class Exc_PartieDejaCreee : public std::exception {
	public:
		Exc_PartieDejaCreee() throw() : std::exception() { }
		virtual ~Exc_PartieDejaCreee() throw() { }
		virtual const char* what() const throw() { return "Partie déjà créée !"; }
	};
	
	static Partie *creerPartie() throw(Exc_PartieDejaCreee);
	static Partie *creerPartie(std::string const &sauvegarde) throw(Exc_PartieDejaCreee);
	static Partie *creerPartie(int numeroSecteur) throw(Exc_PartieDejaCreee);
	virtual ~Partie();
	
	void commencer();
	void reinitialiser();
	
	Joueur *joueur();
	Marchand *marchand();
	void definirMarchand(Marchand *m);
	
	Rectangle zoneJeu() const;
	
	static Partie *partie();
	
private:
	void afficher();
	void animer();
	void gestionEvenements();
	void mortJoueur();
	
	Partie();
	Partie(std::string const &sauvegarde);
	Partie(int numeroSecteur);

	Niveau *_niveau;
	Joueur *_joueur;
	TableauDeBord *_tableauDeBord;
	Marchand *_marchand;
	
	Partie(Partie const &);
	Partie &operator=(Partie const &);
	
	static Partie *_partie;
};

#endif
