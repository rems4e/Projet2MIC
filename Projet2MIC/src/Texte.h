//
//  Texte.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_TEXTE
#define EN_TETE_TEXTE

#include "Ecran.h"
#include <string>
#include "Unichar.h"
#include "Session.h"

typedef float taillePolice_t;
enum police_t {premierePolice, POLICE_NORMALE = premierePolice, POLICE_DECO, POLICE_GRANDE, nbPolices};
police_t &operator++(police_t &);

class Texte {
	friend void Session::nettoyer();
public:
	Texte(Unichar const &txt = Unichar(), police_t police = POLICE_NORMALE, taillePolice_t taille = 12, Couleur const &coul = Couleur::noir);
	Texte(Texte const &);
	Texte &operator=(Texte const &);
	
	virtual ~Texte();
	
	static Coordonnees dimensions(Unichar const &texte, police_t police, taillePolice_t taille);
	static dimension_t hauteur(police_t police, taillePolice_t taille);
	
	Unichar const &texte() const { return _texte; }
	police_t police() const { return _police; }
	taillePolice_t taille() const { return _taille; }
	Couleur couleur() const { return _couleur; }

	inline Coordonnees dimensions() const { return Texte::dimensions(_texte, _police, _taille); }
	
	Texte &definir(Unichar const &txt);
	Texte &definir(police_t police, taillePolice_t taille);
	Texte &definir(taillePolice_t taille);
	Texte &definir(Unichar const &txt, police_t police, taillePolice_t taille);
	Texte &definir(Couleur const &coul);
	
	void afficher(Coordonnees const &pos) const;
	static void afficher(Unichar const &texte, police_t police, taillePolice_t taille, Couleur const &couleur, Coordonnees pos);
	
private:
	Unichar _texte;
	police_t _police;
	taillePolice_t _taille;
	Couleur _couleur;
	
	Coordonnees _dimensions;

	static bool _init;
	static void nettoyer();
};

#endif
