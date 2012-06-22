//
//  Partie.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
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
	static TiXmlElement *charger(Image &fond, Shader const &s = Shader::aucun());
	
	void restaurer(TiXmlElement *sauve);
	
	// Retourne la partie en cours. Créé une partie s'il n'en existe pas déjà une. Elle doit être effacée avec delete après son utilisation.
	static Partie *partie();
	virtual ~Partie();
	
	TiXmlElement *commencer();
	void terminerNiveau();
	bool niveauTermine() const;
	void reinitialiser();
	
	Joueur *joueur();
	Marchand *marchand();
	void definirMarchand(Marchand *m);
	
	Rectangle zoneJeu() const;
	
	
private:
	Partie();

	void afficher();
	void animer();
	TiXmlElement *mortJoueur(bool &continuer);
		
	void sauvegarder(Image *fond);
	TiXmlElement *sauvegarde();

	Niveau *_niveau;
	Joueur *_joueur;
	TableauDeBord *_tableauDeBord;
	Marchand *_marchand;
	
	int _numeroNiveau;
	bool _niveauTermine;
	TiXmlElement *_derniereSauvegarde;
	
	Partie(Partie const &);
	Partie &operator=(Partie const &);
	
	static Partie *_partie;
};

#endif
