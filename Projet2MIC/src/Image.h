//
//  Image.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_IMAGE
#define EN_TETE_IMAGE

#include "Constantes.h"
#include <SDL/SDL.h>

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif

#include <string>
#include "Ecran.h"
#include "Geometrie.h"
#include <stack>
#include "Shader.h"

#define REUTILISATION_ID_TEXTURE 1

struct ImageBase;

class Image {
	friend void quitter(int code);
public:
	Image(std::string const &fichier);
	// On créé un image à partir d'un tableau 2D de pixels, rangés un rang après l'autre, avec 'profondeur' octet pour chaque pixel.
	Image(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner = false);
	Image(Image const &img);
	Image();
	Image &operator=(Image const &img);
	virtual ~Image();
		
	// L'image a été chargée correctement
	inline bool valide() const { return _base; }
	
	// Dimensions de l'image multipliées par le facteur de zoom
	Coordonnees dimensions() const;
	// Dimensions de l'image zommée à 100 %
	Coordonnees dimensionsReelles() const;
	
	// Pixels de l'image. Coûteux à obtenir. Le pointeur doit être passé à delete[] après utilisation.
	unsigned char const *pixels() const;
	
	// Transformations de l'image
	Image const &tourner(float angle) const;
	Image const &redimensionner(coordonnee_t facteur) const;
	Image const &redimensionner(Coordonnees const &facteur) const;
	
	// Affichage de l'image à une position donnée
	inline void afficher(Coordonnees const &position) const { this->afficher(position, Rectangle(Coordonnees(), this->dimensionsReelles())); }
	// Affichage de la portion de l'image définie par le rectangle
	void afficher(Coordonnees const &position, Rectangle const &filtre) const;
	
	// Fichier de l'image ou chaîne vide si l'image a été générée à partir d'une matrice de pixels
	inline std::string const &fichier() const;
	
	// Opacité de l'affichage de 0 = transparent à 255 = opaque. Valable pour toutes les images.
	static unsigned char opacite();
	static void definirOpacite(unsigned char o);
	
	// Teinte de l'image : Couleur::blanc -> image non teintée. Valable pour toutes les images.
	static Couleur teinte();
	static void definirTeinte(Couleur const &c);
	
	GLint tex();
	
private:
	ImageBase *_base;
	
	mutable Coordonnees _facteur;
	mutable float _angle;
	static Couleur _teinte;
	static unsigned char _opacite;
	
	Image *charger(std::string const &fichier);
	Image *charger(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner);
};

#endif
