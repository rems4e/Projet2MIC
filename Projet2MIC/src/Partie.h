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
#include "Shader.h"

class Niveau;
class Joueur;
class TableauDeBord;
class Marchand;
class TiXmlElement;
class Image;

class Partie {
public:
	class Exc_PartieDejaCreee : public std::exception {
	public:
		Exc_PartieDejaCreee() throw() : std::exception() { }
		virtual ~Exc_PartieDejaCreee() throw() { }
		virtual const char* what() const throw() { return "Partie déjà créée !"; }
	};
	
	static TiXmlElement *charger(Image *fond, Shader const &s = Shader::aucun());
	static Partie *creerPartie(TiXmlElement *sauve = 0) throw(Exc_PartieDejaCreee);
	virtual ~Partie();
	
	TiXmlElement *commencer();
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
	Partie(TiXmlElement *sauve);
	
	void sauvegarder(Image *fond);
	TiXmlElement *sauvegarde();

	Niveau *_niveau;
	Joueur *_joueur;
	TableauDeBord *_tableauDeBord;
	Marchand *_marchand;
	
	int _numeroNiveau;
	
	Partie(Partie const &);
	Partie &operator=(Partie const &);
	
	static Partie *_partie;
};

#endif
