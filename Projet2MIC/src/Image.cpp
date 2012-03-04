/*
 *  Image.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 10/04/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#include "Image.h"
#include <list>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <cmath>
#include <cstring>

struct ImageBase {
	GLuint _tex;
	std::string _fichier;
	Coordonnees _dimensions;
	int _nombreReferences;
	
	static ImageBase *imageBase(std::string const &fichier);
	static ImageBase *imageBase(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner);
	
	inline ImageBase(std::string const &fichier) : _tex(0), _dimensions(), _fichier(), _nombreReferences(1) { this->charger(fichier); }
	inline ImageBase(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner) : _tex(0), _dimensions(), _fichier(), _nombreReferences(1) { this->charger(pixels, largeur, hauteur, profondeur, retourner); }
	
	ImageBase *charger(std::string const &fichier);
	ImageBase *charger(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner);
	
	virtual ~ImageBase();
	
	void detruire();
	
	inline bool operator==(ImageBase const &i) const {
		return _fichier == i._fichier;
	}
	inline bool operator==(std::string const &f) const {
		return _fichier == f;
	}
};

static std::list<ImageBase *> *images = 0;
#if REUTILISATION_ID_TEXTURE
GLuint const Image::aucuneTexture = GLuint(-1);
GLuint Image::_derniereTexture = Image::aucuneTexture;

#endif

Couleur Image::_teinte = Couleur::blanc;
unsigned char Image::_opacite = 255;

struct egImg {
	inline egImg(std::string const &s) : _s(s) { }
	
	inline bool operator()(ImageBase const *i) const {
		return *i == _s;
	}
	
	std::string const &_s;
};

ImageBase *ImageBase::imageBase(std::string const &fichier) {
	if(images == 0)
		images = new std::list<ImageBase *>;
	
	std::list<ImageBase *>::iterator it = std::find_if(images->begin(), images->end(), egImg(fichier));

	if(it == images->end()) {
		ImageBase *i = new ImageBase(fichier);
		images->push_back(i);
		return i;
	}
	
	++(*it)->_nombreReferences;
	
	return *it;
}

ImageBase *ImageBase::imageBase(unsigned char *img, int largeur, int hauteur, int profondeur, bool retourner) {
	return new ImageBase(img, largeur, hauteur, profondeur, retourner);
}

void ImageBase::detruire() {
	if(!images)
		return;

	--_nombreReferences;
	if(_nombreReferences <= 0) {
		if(_fichier.size()) {
			std::list<ImageBase *>::iterator it = std::find(images->begin(), images->end(), this);
			images->erase(it);
		}
		delete this;
	}
}

ImageBase::~ImageBase() {
#if REUTILISATION_ID_TEXTURE
	Image::changerTexture(Image::aucuneTexture);
#endif
	glDeleteTextures(1, &_tex);
}

ImageBase *ImageBase::charger(std::string const &fichier) {
	_fichier = fichier;

	SDL_Surface *surf = IMG_Load(_fichier.c_str());
	if(!surf) {
		std::cout << "L'image \"" << fichier << "\" n'a pu être chargée : " << IMG_GetError() << std::endl;
	}
	
	SDL_PixelFormat format = *(surf->format);
	format.BitsPerPixel = 32;
	format.BytesPerPixel = 4;
	
	if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		format.Rmask = 0xff000000;
		format.Gmask = 0x00ff0000;
		format.Bmask = 0x0000ff00;
		format.Amask = 0x000000ff;
	}
	else {
		format.Rmask = 0x000000ff;
		format.Gmask = 0x0000ff00;
		format.Bmask = 0x00ff0000;
		format.Amask = 0xff000000;
	}
	
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

ImageBase *ImageBase::charger(unsigned char *img, int largeur, int hauteur, int profondeur, bool retourner) {
#if REUTILISATION_ID_TEXTURE
	Image::changerTexture(Image::aucuneTexture);
#endif
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

void Image::nettoyer() {
	for(std::list<ImageBase *>::iterator i = images->begin(); i != images->end(); ++i) {
		delete *i;
	}
	delete images;
	images = 0;
}

Coordonnees Image::dimensions() const {
	return Coordonnees(_base->_dimensions.x * _facteurX, _base->_dimensions.y * _facteurY);
}

Coordonnees Image::dimensionsReelles() const {
	return _base->_dimensions;
}

std::string const &Image::fichier() const {
	return _base->_fichier;
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


#if REUTILISATION_ID_TEXTURE
void Image::changerTexture(GLuint tex) {
	if(tex != _derniereTexture) {
		/*if(!_vert.empty() && _derniereTexture != Image::aucuneTexture) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, _derniereTexture);
			
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			
			glColor4ub(255 - Ecran::teinte().a * (255 - Ecran::teinte().r) / 255, 255 - Ecran::teinte().a * (255 - Ecran::teinte().v) / 255, 255 - Ecran::teinte().a * (255 - Ecran::teinte().b) / 255, Ecran::opacite());
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, &_vert[0]);
			
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, &_texCoord[0]);
			
			glDrawArrays(GL_TRIANGLES, 0, _vert.size() / 2);
			//std::cout << _vert.size() << std::endl;
			//std::cout << _texCoord.size() << std::endl;
			_vert.clear();
			_texCoord.clear();
		}*/
		
		_derniereTexture = tex;
		glEnd();
		
		if(_derniereTexture != Image::aucuneTexture) {
			glBindTexture(GL_TEXTURE_2D, _derniereTexture);
		}
	}
}

#endif

void Image::afficher(Coordonnees const &position, Rectangle const &filtre) const {
	Rectangle vert(position, Coordonnees(filtre.largeur * _facteurX, filtre.hauteur * _facteurY));
		
	#if REUTILISATION_ID_TEXTURE
	GLuint tex = _derniereTexture;
	this->changerTexture(_base->_tex);
	if(_base->_tex != tex)
		glBegin(GL_QUADS);
	#else
		glBindTexture(GL_TEXTURE_2D, _base->_tex);
		glBegin(GL_QUADS);
	#endif

	glColor4ub(255 - Image::_teinte.a * (255 - Image::_teinte.r) / 255, 255 - Image::_teinte.a * (255 - Image::_teinte.v) / 255, 255 - Image::_teinte.a * (255 - Image::_teinte.b) / 255, Image::_opacite);
	
	glTexCoord2d(filtre.gauche / _base->_dimensions.x, filtre.haut / _base->_dimensions.y);
	glVertex2d(vert.gauche, vert.haut);
	
	glTexCoord2d((filtre.gauche + filtre.largeur) / _base->_dimensions.x, filtre.haut / _base->_dimensions.y);
	glVertex2d(vert.largeur + vert.gauche, vert.haut);
	
	glTexCoord2d((filtre.gauche + filtre.largeur) / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y);
	glVertex2d(vert.largeur + vert.gauche, vert.hauteur + vert.haut);
	
	glTexCoord2d(filtre.gauche / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y);
	glVertex2d(vert.gauche, vert.hauteur + vert.haut);
	
#if !REUTILISATION_ID_TEXTURE
	glEnd();
#endif
}
