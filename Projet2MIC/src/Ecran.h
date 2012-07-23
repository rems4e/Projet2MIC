//
//  Ecran.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef EN_TETE_ECRAN
#define EN_TETE_ECRAN

#include "Geometrie.h"
#include <string>
#include <list>
#include "nombre.h"
#include <exception>

#define LARGEUR_ECRAN 800
#define HAUTEUR_ECRAN 600
#define PLEIN_ECRAN false
#define FREQUENCE_RAFRAICHISSEMENT (Parametres::limiteIPS() ? 100 : 10000)

class Texte;
class Image;

struct Couleur {
	unsigned char r, v, b, a;

	static Couleur const noir;
	static Couleur const gris;
	static Couleur const grisClair;
	static Couleur const grisFonce;
	static Couleur const blanc;
	static Couleur const rouge;
	static Couleur const vert;
	static Couleur const bleu;
	static Couleur const jaune;
	static Couleur const transparent;
	
	inline Couleur() : r(255), v(255), b(255), a(255) { }
	inline Couleur(unsigned char _r, unsigned char _v, unsigned char _b, unsigned char _a = 255) : r(_r), v(_v), b(_b), a(_a) { }
	inline Couleur(unsigned char gris, unsigned char _a = 255) : r(gris), v(gris), b(gris), a(_a) { }
	inline Couleur(Couleur const &c, unsigned char _a) : r(c.r), v(c.v), b(c.b), a(_a) { }
};

namespace Ecran {
	class Exc_InitialisationImpossible : public std::exception {
	public:
		Exc_InitialisationImpossible() throw() : std::exception() {
			std::cerr << this->what() << std::endl;
		}
		virtual ~Exc_InitialisationImpossible() throw() { }
		virtual const char* what() const throw() { return "Impossible de définir la résolution de l'écran."; }
	};
	
	void modifierResolution(unsigned int largeur, unsigned int hauteur, bool pleinEcran) throw(Exc_InitialisationImpossible);
	
	Coordonnees dimensions();
	int largeur();
	int hauteur();
	
	Rectangle ecran();
	
	bool pleinEcran();
		
	float frequence();
	float frequenceInstantanee();
	
	Image *apercu();
	
	// Met à jour l'affichage en fonction des éléments affichés depuis le dernier appel à la fonction
	void maj();
	void finaliser();
	// Remplis l'écran avec une couleur unie
	void effacer();
	
	// Affiche une couleur unie dans un rectangle. La couleur peut être transparente.
	void afficherRectangle(Rectangle const &r, Couleur const &c);
	void afficherLigne(Coordonnees const &depart, Coordonnees const &arrivee, Couleur const &c, dimension_t epaisseur = 1.0);
	void afficherQuadrilatere(Coordonnees const &p1, Coordonnees const &p2, Coordonnees const &p3, Coordonnees const &p4, Couleur const &c);
	void afficherTriangle(Coordonnees const &p1, Coordonnees const &p2, Coordonnees const &p3, Couleur const &c);
	
	// Pointeur
	bool pointeurAffiche();
	void definirPointeurAffiche(bool af);
	
	Image const *pointeur();
	// Si image vaut 0, le pointeur par défaut est utilisé. La valeur decalage représente les coordonnées du point cliquable de l'image du pointeur.
	void definirPointeur(Image const *image, Coordonnees const &decalage = Coordonnees());
	
	Coordonnees const &echelle();
	coordonnee_t echelleMin();
	
	// Les résolutions disponibles pour la fenêtre/le plein écran.
	std::list<std::pair<int, int> > resolutionsDisponibles(bool pleinEcran);
}

#endif
