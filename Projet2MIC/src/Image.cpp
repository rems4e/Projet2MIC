/*
 *  Image.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 10/04/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#include "Image.h"
#include <map>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <cmath>
#include <cstring>

#define TAMPON_SOMMETS 200000

namespace ImagesBase {
	std::map<std::string, ImageBase *> *_images = 0;
	void initialiser();
	void nettoyer();
	GLint _tex = -1;
	std::vector<float> _vertCoords;
	std::vector<float> _texCoords;
	std::vector<unsigned char> _couleurs;
	size_t _nbSommets = 0;
		
	void changerTexture(GLint tex);
	
	void ajouterSommet(Coordonnees const &pos, Coordonnees const &posTex, Couleur const &couleur);
}

struct ImageBase {
	inline ImageBase(std::string const &fichier) : _tex(0), _dimensions(), _fichier(), _nombreReferences(1) { this->charger(fichier); }
	inline ImageBase(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner) : _tex(0), _dimensions(), _fichier(), _nombreReferences(1) { this->charger(pixels, largeur, hauteur, profondeur, retourner); }
	static ImageBase *imageBase(std::string const &fichier);
	static ImageBase *imageBase(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner);

	virtual ~ImageBase();
	void detruire();
	
	ImageBase *charger(std::string const &fichier);
	ImageBase *charger(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner);
	
	unsigned char const *pixels() const;
		
	GLuint _tex;
	std::string _fichier;
	Coordonnees _dimensions;
	int _nombreReferences;
};

void ImagesBase::nettoyer() {
	for(std::map<std::string, ImageBase *>::iterator i = ImagesBase::_images->begin(); i != ImagesBase::_images->end(); ++i) {
		delete i->second;
	}
	
	delete ImagesBase::_images;
	ImagesBase::_images = 0;
}

void ImagesBase::initialiser() {
	_vertCoords.resize(2 * TAMPON_SOMMETS);
	_couleurs.resize(4 * TAMPON_SOMMETS);
	_texCoords.resize(2 * TAMPON_SOMMETS);
}

void ImagesBase::ajouterSommet(Coordonnees const &pos, Coordonnees const &posTex, Couleur const &couleur) {
	if(_vertCoords.size() / 2 <= _nbSommets) {
		_vertCoords.resize(_vertCoords.size() + _vertCoords.size() / 10);
		_texCoords.resize(_texCoords.size() + _texCoords.size() / 10);
		_couleurs.resize(_couleurs.size() + _couleurs.size() / 10);
		
		std::cout << "Dimensionnement des tableaux de sommets sous-évalués. Nouvelle taille : " << _vertCoords.size() / 2 << std::endl;
	}
	
/*	if(vertBuf == -1) {
		glGenBuffers(1, &vertBuf);

		glBindBuffer(GL_ARRAY_BUFFER, vertBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * TAMPON_SOMMETS * 2, 0, GL_DYNAMIC_DRAW);

	}*/

	_vertCoords[_nbSommets * 2] = pos.x / Ecran::largeur() * 2 - 1;
	_vertCoords[_nbSommets * 2 + 1] = -(pos.y / Ecran::hauteur() * 2 - 1);

	_texCoords[_nbSommets * 2] = posTex.x;
	_texCoords[_nbSommets * 2 + 1] = posTex.y;

	_couleurs[_nbSommets * 4] = couleur.r;
	_couleurs[_nbSommets * 4 + 1] = couleur.v;
	_couleurs[_nbSommets * 4 + 2] = couleur.b;
	_couleurs[_nbSommets * 4 + 3] = couleur.a;

	++_nbSommets;
}

void ImagesBase::changerTexture(GLint tex) {
	if(tex != _tex) {
		if(_nbSommets) {
			//glBindBuffer(GL_ARRAY_BUFFER, vertBuf);
			//glBufferSubData(GL_ARRAY_BUFFER, 0, _nbSommets * 2 * sizeof(_vertCoords[0]), &_vertCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().vertCoord(), 2, GL_FLOAT, GL_FALSE, 0, &_vertCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().texCoord(), 2, GL_FLOAT, GL_FALSE, 0, &_texCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().coul(), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &_couleurs[0]);
			
			glEnableVertexAttribArray(Shader::shaderActuel().vertCoord());
			glEnableVertexAttribArray(Shader::shaderActuel().texCoord());
			glEnableVertexAttribArray(Shader::shaderActuel().coul());
			glDrawArrays(GL_TRIANGLES, 0, _nbSommets);

			_nbSommets = 0;
		}
		

		if(tex != -1) {
			glBindTexture(GL_TEXTURE_2D, tex);
		}
		_tex = tex;
	}
}

Couleur Image::_teinte = Couleur::blanc;
unsigned char Image::_opacite = 255;

ImageBase *ImageBase::imageBase(std::string const &fichier) {
	if(ImagesBase::_images == 0)
		ImagesBase::_images = new std::map<std::string, ImageBase *>;
	
	ImageBase *&img = ImagesBase::_images->operator[](fichier);

	if(img == 0) {
		img = new ImageBase(fichier);
	}
	else
		++(img->_nombreReferences);
	
	return img;
}

ImageBase *ImageBase::imageBase(unsigned char *img, int largeur, int hauteur, int profondeur, bool retourner) {
	return new ImageBase(img, largeur, hauteur, profondeur, retourner);
}

void ImageBase::detruire() {
	--_nombreReferences;
	if(_nombreReferences <= 0) {
		if(!_fichier.empty()) {
			ImagesBase::_images->erase(_fichier);
		}
		
		delete this;
	}
}

ImageBase::~ImageBase() {
	glDeleteTextures(1, &_tex);
}

ImageBase *ImageBase::charger(std::string const &fichier) {
	_fichier = fichier;

	SDL_Surface *surf = IMG_Load(_fichier.c_str());
	if(!surf) {
		std::cerr << "L'image \"" << fichier << "\" n'a pu être chargée : " << IMG_GetError() << std::endl;
		throw 0;
	}
	
	SDL_PixelFormat format = *(surf->format);
	format.BitsPerPixel = 32;
	format.BytesPerPixel = 4;
	
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	format.Rmask = 0xff000000;
	format.Gmask = 0x00ff0000;
	format.Bmask = 0x0000ff00;
	format.Amask = 0x000000ff;
#else
	format.Rmask = 0x000000ff;
	format.Gmask = 0x0000ff00;
	format.Bmask = 0x00ff0000;
	format.Amask = 0xff000000;
#endif
	
	SDL_Surface *conv = SDL_CreateRGBSurface(SDL_SWSURFACE, surf->w, surf->h, surf->format->BitsPerPixel, format.Rmask, format.Gmask, format.Bmask, format.Amask);
	
	SDL_Surface *conv1 = SDL_ConvertSurface(conv, &format, SDL_SWSURFACE);
	SDL_FreeSurface(conv);
	conv = conv1;
	SDL_SetAlpha(surf, 0, 0);
	SDL_BlitSurface(surf, 0, conv, 0);
	SDL_FreeSurface(surf);
	surf = conv;
	
	ImageBase *img = this->charger(static_cast<unsigned char *>(surf->pixels), surf->w, surf->h, surf->format->BytesPerPixel, false);
	SDL_FreeSurface(surf);
	
	return img;
}

ImageBase *ImageBase::charger(unsigned char *img, int largeur, int hauteur, int profondeur, bool retourner) {	
	glGenTextures(1, &_tex);
	
	if(retourner) {
		unsigned char *imageRetournee = new unsigned char[largeur * hauteur * profondeur];
		for(int ligne = 0; ligne < hauteur; ++ligne) {
			std::memcpy(imageRetournee + ligne * largeur * profondeur, img + (hauteur - ligne - 1) * largeur * profondeur, largeur * profondeur);
		}
		
		img = imageRetournee;
	}
		
	_dimensions.x = largeur;
	_dimensions.y = hauteur;
	
	glBindTexture(GL_TEXTURE_2D , _tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, largeur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	
	if(retourner) {
		delete[] img;
	}

	return this;
}

unsigned char const *ImageBase::pixels() const {
	unsigned char *pix = new unsigned char[size_t(_dimensions.x * _dimensions.y * 4)];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);

	return pix;
}

Image::Image(std::string const &fichier) : _base(0), _facteurX(1.0f), _facteurY(1.0f), _angle(0.0f) {
	if(fichier.size())
		_base = ImageBase::imageBase(fichier);
}

Image::Image(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner) : _base(0), _facteurX(1.0f), _facteurY(1.0f), _angle(0.0f) {
	_base = ImageBase::imageBase(pixels, largeur, hauteur, profondeur, retourner);

}

Image::Image(Image const &img) : _base(0), _facteurX(1.0f), _facteurY(1.0f), _angle(0.0f) {
	if(img.valide()) {
		_base = img._base;
		++_base->_nombreReferences;
	}
}

Image::Image() : _base(0), _facteurX(1.0f), _facteurY(1.0f), _angle(0.0f) {
	
}

Image &Image::operator=(Image const &img) {
	_base = img._base;
	if(_base)
		++_base->_nombreReferences;
		
	_facteurX = img._facteurX;
	_facteurY = img._facteurY;

	_angle = img._angle;
	
	return *this;
}

Image::~Image() {
	if(this->valide()) {
		_base->detruire();
	}
}

unsigned char Image::opacite() {
	return _opacite;
}

Couleur Image::teinte() {
	return _teinte;
}

void Image::definirTeinte(const Couleur &c) {
	_teinte = c;
}

void Image::definirOpacite(unsigned char o) {
	_opacite = o;
}

Coordonnees Image::dimensions() const {
	return Coordonnees(_base->_dimensions.x * _facteurX, _base->_dimensions.y * _facteurY);
}

Coordonnees Image::dimensionsReelles() const {
	return _base->_dimensions;
}

unsigned char const *Image::pixels() const {
	return _base ? _base->pixels() : 0;
}

std::string const &Image::fichier() const {
	return _base->_fichier;
}

GLint Image::tex() {
	return _base->_tex;
}

Image const &Image::tourner(float angle) const {
	_angle = std::fmod(_angle + angle, static_cast<float>(2 * M_PI));
	
	return *this;
}

Image const &Image::redimensionner(facteur_t facteur) const {
	return this->redimensionner(facteur, facteur);
}

Image const &Image::redimensionner(facteur_t facteurX, facteur_t facteurY) const {
	_facteurX = facteurX;
	_facteurY = facteurY;
	
	return *this;
}

void Image::afficher(Coordonnees const &position, Rectangle const &filtre) const {
	Rectangle vert(position, Coordonnees(filtre.largeur * _facteurX, filtre.hauteur * _facteurY));
	
	if(!vert.superposition(Ecran::ecran()))
		return;
	
	Shader::shaderActuel().definirParametre(Shader::dim, vert.largeur, vert.hauteur);
	Shader::shaderActuel().definirParametre(Shader::pos, vert.gauche, vert.haut);
		
	ImagesBase::changerTexture(_base->_tex);
		
	Couleur c(255 - Image::_teinte.a * (255 - Image::_teinte.r) / 255, 255 - Image::_teinte.a * (255 - Image::_teinte.v) / 255, 255 - Image::_teinte.a * (255 - Image::_teinte.b) / 255, Image::_opacite);

	ImagesBase::ajouterSommet(Coordonnees(vert.gauche, vert.haut), Coordonnees(filtre.gauche / _base->_dimensions.x, filtre.haut / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche + vert.largeur, vert.haut), Coordonnees((filtre.gauche + filtre.largeur) / _base->_dimensions.x, filtre.haut / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche, vert.haut + vert.hauteur), Coordonnees(filtre.gauche / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y), c);

	ImagesBase::ajouterSommet(Coordonnees(vert.gauche, vert.haut + vert.hauteur), Coordonnees(filtre.gauche / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche + vert.largeur, vert.haut), Coordonnees((filtre.gauche + filtre.largeur) / _base->_dimensions.x, filtre.haut / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche + vert.largeur, vert.haut + vert.hauteur), Coordonnees((filtre.gauche + filtre.largeur) / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y), c);
}
