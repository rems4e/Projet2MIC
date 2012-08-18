//
//  Image.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Ecran.h"
#include "Affichage.h"
#include "Image.h"
#include <map>
#include <SDL/SDL.h>
#include <SDL_image/SDL_image.h>
#include <cmath>
#include <cstring>
#include <vector>

Image::Image(std::string const &fichier) throw(int) : _tex(new Texture(fichier)), _facteur(1, 1), _angle(0.0f) {

}

Image::Image(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner) : _tex(new Texture(pixels, largeur, hauteur, profondeur, retourner)), _facteur(1, 1), _angle(0.0f) {

}

Image::Image(Image const &img) : _tex(img._tex), _facteur(1, 1), _angle(0.0f) {

}

Image::Image() : _tex(0), _facteur(1, 1), _angle(0.0f) {
	
}

Image &Image::operator=(Image const &img) {
	_tex = img._tex;
	_facteur = img._facteur;
	_angle = img._angle;
	
	return *this;
}

Image::~Image() {

}

bool Image::chargee() const {
	return _tex;
}

glm::vec2 Image::dimensions() const {
	return glm::vec2(this->dimensionsReelles()) * _facteur;
}

glm::ivec2 &Image::dimensionsReelles() const {
	return _tex->dimensions();
}

GLubyte const *Image::pixels() const {
	return _tex->pixels();
}

Image const &Image::tourner(float angle) const {
	_angle = std::fmod(_angle + angle, static_cast<float>(2 * M_PI));
	
	return *this;
}

Image const &Image::redimensionner(coordonnee_t facteur) const {
	return this->redimensionner(glm::vec2(facteur));
}

Image const &Image::redimensionner(glm::vec2 const &facteur) const {
	_facteur = facteur;
	
	return *this;
}

void Image::afficher(glm::vec2 const &position, Rectangle const &filtre) const {
	Rectangle vert(position, glm::vec2(filtre.largeur * _facteur.x, filtre.hauteur * _facteur.y));
	
	if(!vert.superposition(Ecran::ecran()))
		return;
		
	Affichage::changerTexture(_tex->tex());
	
	Couleur const &teinte = Affichage::teinte();
	Couleur c(255 - teinte.a * (255 - teinte.r) / 255, 255 - teinte.a * (255 - teinte.v) / 255, 255 - teinte.a * (255 - teinte.b) / 255, Affichage::opacite());
	glm::ivec2 const &dim = this->dimensionsReelles();
	
	Affichage::ajouterSommet(glm::vec2(vert.gauche, vert.haut), glm::vec2(filtre.gauche / dim.x, filtre.haut / dim.y), c, true);
	Affichage::ajouterSommet(glm::vec2(vert.gauche, vert.haut + vert.hauteur), glm::vec2(filtre.gauche / dim.x, (filtre.haut + filtre.hauteur) / dim.y), c, true);
	Affichage::ajouterSommet(glm::vec2(vert.gauche + vert.largeur, vert.haut), glm::vec2((filtre.gauche + filtre.largeur) / dim.x, filtre.haut / dim.y), c, true);
	
	Affichage::ajouterSommet(glm::vec2(vert.gauche, vert.haut + vert.hauteur), glm::vec2(filtre.gauche / dim.x, (filtre.haut + filtre.hauteur) / dim.y), c, true);
	Affichage::ajouterSommet(glm::vec2(vert.gauche + vert.largeur, vert.haut + vert.hauteur), glm::vec2((filtre.gauche + filtre.largeur) / dim.x, (filtre.haut + filtre.hauteur) / dim.y), c, true);
	Affichage::ajouterSommet(glm::vec2(vert.gauche + vert.largeur, vert.haut), glm::vec2((filtre.gauche + filtre.largeur) / dim.x, filtre.haut / dim.y), c, true);
}
