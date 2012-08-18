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
#include "Unichar.h"
#include <glm/glm.hpp>

#define LARGEUR_ECRAN 800
#define HAUTEUR_ECRAN 600
#define PLEIN_ECRAN false

class Texte;
class Texture;
class Image;

struct Couleur {
	typedef unsigned char composante_t;
	composante_t r, v, b, a;

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
	inline Couleur(composante_t _r, composante_t _v, composante_t _b, composante_t _a = 255) : r(_r), v(_v), b(_b), a(_a) { }
	inline Couleur(composante_t gris, composante_t _a = 255) : r(gris), v(gris), b(gris), a(_a) { }
	inline Couleur(Couleur const &c, composante_t _a) : r(c.r), v(c.v), b(c.b), a(_a) { }
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
	
	glm::ivec2 dimensions();
	int largeur();
	int hauteur();
	
	Rectangle ecran();
	
	bool pleinEcran();
		
	float frequence();
	float frequenceInstantanee();
	
	Image *apercu();
	
	void maj();
	
	// Pointeur
	bool pointeurAffiche();
	void definirPointeurAffiche(bool af);
	
	Texture const &vide();

	Image const *pointeur();
	// Si image vaut 0, le pointeur par défaut est utilisé. La valeur decalage représente les coordonnées du point cliquable de l'image du pointeur.
	void definirPointeur(Image const *image, glm::vec2 const &decalage = glm::vec2(0.0f));
	
	glm::vec2 const &echelle();
	coordonnee_t echelleMin();
	
	// Les résolutions disponibles pour la fenêtre/le plein écran.
	std::list<glm::ivec2> resolutionsDisponibles(bool pleinEcran);

	Rectangle const &cadreAffichage();
	void ajouterCadreAffichage(Rectangle const &);
	void supprimerCadreAffichage();

	Unichar const &bulleAide();
	void definirBulleAide(Unichar const &txt);
	
	void ajouterTransformation(glm::mat4 const &transfo);
	// Si on supprime plus que ce que l'on doit, exception lancée !
	void supprimerTransformation() throw(int);
	glm::vec4 transformerVecteur(glm::vec4 const &vec);
	
	void perspective(float angle, float near, float far);

	glm::mat4 const &projection();
	glm::mat4 const &modeleVue();
}

#endif
