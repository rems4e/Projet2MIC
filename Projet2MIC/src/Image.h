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
#include "Texture.h"
#include <string>

namespace Session {
	void nettoyer();
}

class Image final {
	friend void Session::nettoyer();
	friend void modifierResolution(unsigned int largeur, unsigned int hauteur, bool pleinEcran);
public:
	Image(std::string const &fichier) throw(int);
	// On créé un image à partir d'un tableau 2D de pixels, rangés un rang après l'autre, avec 'profondeur' octet pour chaque pixel.
	Image(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner = false);
	Image(Image const &img);
	Image();
	Image &operator=(Image const &img);
	~Image();
	
	// Dimensions de l'image multipliées par le facteur de zoom
	glm::vec2 dimensions() const;
	// Dimensions de l'image zommée à 100 %
	glm::ivec2 &dimensionsReelles() const;
	
	// Pixels de l'image. Coûteux à obtenir. Le pointeur doit être passé à delete[] après utilisation.
	GLubyte const *pixels() const;
	
	// Transformations de l'image
	Image const &tourner(float angle) const;
	Image const &redimensionner(coordonnee_t facteur) const;
	Image const &redimensionner(glm::vec2 const &facteur) const;
	
	// Affichage de l'image à une position donnée
	inline void afficher(glm::vec2 const &position) const {
		this->afficher(position, Rectangle(glm::vec2(0.0f), glm::vec2(this->dimensionsReelles())));
	}
	// Affichage de la portion de l'image définie par le rectangle
	void afficher(glm::vec2 const &position, Rectangle const &filtre) const;
	
	bool chargee() const;
private:
	Texture *_tex = nullptr;
	
	mutable glm::vec2 _facteur;
	mutable float _angle;
};

#endif
