//
//  Texture.cpp
//  Projet2MIC
//
//  Created by Rémi on 17/08/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Texture.h"
#include <SDL_image/SDL_image.h>

namespace TexturePrive {
	struct SauveImage {
		glm::ivec2 _dimensions;
		GLubyte *_pixels;
	};
	
	void nettoyer();
	void preReinitialiser();
	void reinitialiser();
	
	std::unordered_map<std::string, Texture::TextureBase *> _images;
	std::list<Texture::TextureBase *> _imagesSansFichier;
	std::list<SauveImage> _sauveImages;
}

struct Texture::TextureBase final {
	inline TextureBase(std::string const &fichier): _fichier() {
		this->charger(fichier);
	}
	inline TextureBase(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner) : _fichier() {
		this->charger(pixels, largeur, hauteur, profondeur, retourner);
	}
	
	static TextureBase *imageBase(std::string const &fichier);
	static TextureBase *imageBase(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner);
	
	~TextureBase();
	void detruire();
	
	TextureBase *charger(std::string const &fichier);
	TextureBase *charger(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner);
	
	GLubyte const *pixels() const;
	
	GLuint _tex = 0;
	std::string _fichier;
	glm::ivec2 _dimensions;
	int _nombreReferences = 1;
};

void TexturePrive::nettoyer() {
	for(std::unordered_map<std::string, Texture::TextureBase *>::iterator i = TexturePrive::_images.begin(); i != TexturePrive::_images.end(); ++i) {
		delete i->second;
	}
}

void TexturePrive::preReinitialiser() {
	for(std::list<Texture::TextureBase *>::iterator i = TexturePrive::_imagesSansFichier.begin(); i != TexturePrive::_imagesSansFichier.end(); ++i) {
		SauveImage sauve;
		sauve._dimensions = (*i)->_dimensions;
		sauve._pixels = new GLubyte[sauve._dimensions.x * sauve._dimensions.y * 4];
		glBindTexture(GL_TEXTURE_2D, (*i)->_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, sauve._pixels);
		_sauveImages.push_back(sauve);
		
		glDeleteTextures(1, &((*i)->_tex));
	}
	
	for(std::unordered_map<std::string, Texture::TextureBase *>::iterator i = TexturePrive::_images.begin(); i != TexturePrive::_images.end(); ++i) {
		glDeleteTextures(1, &i->second->_tex);
	}
}

void TexturePrive::reinitialiser() {
	for(std::unordered_map<std::string, Texture::TextureBase *>::iterator i = TexturePrive::_images.begin(); i != TexturePrive::_images.end(); ++i) {
		i->second->charger(i->second->_fichier);
	}
	
	for(std::list<Texture::TextureBase *>::iterator i = TexturePrive::_imagesSansFichier.begin(); i != TexturePrive::_imagesSansFichier.end(); ++i) {
		SauveImage sauve = _sauveImages.front();
		
		(*i)->charger(sauve._pixels, sauve._dimensions.x, sauve._dimensions.y, 4, false);
		delete[] sauve._pixels;
		_sauveImages.pop_front();
	}
}

Texture::TextureBase *Texture::TextureBase::imageBase(std::string const &fichier) {
	TextureBase *&img = TexturePrive::_images[fichier];
	
	if(img == 0) {
		img = new TextureBase(fichier);
	}
	else
		++(img->_nombreReferences);
	
	return img;
}

Texture::TextureBase *Texture::TextureBase::imageBase(GLubyte const *img, size_t largeur, size_t hauteur, int profondeur, bool retourner) {
	TextureBase *image = new TextureBase(img, largeur, hauteur, profondeur, retourner);
	TexturePrive::_imagesSansFichier.push_back(image);
	
	return image;
}

void Texture::TextureBase::detruire() {
	--_nombreReferences;
	if(_nombreReferences <= 0) {
		if(!_fichier.empty()) {
			TexturePrive::_images.erase(_fichier);
		}
		else {
			TexturePrive::_imagesSansFichier.remove(this);
		}
		
		delete this;
	}
}

Texture::TextureBase::~TextureBase() {
	glDeleteTextures(1, &_tex);
}

Texture::TextureBase *Texture::TextureBase::charger(std::string const &fichier) {
	_fichier = fichier;
	
	Bitmap const b(fichier);
	TextureBase *img = this->charger(b.pixels(), b.largeur(), b.hauteur(), b.profondeur(), false);
	
	return img;
}

Texture::TextureBase *Texture::TextureBase::charger(GLubyte const *img, size_t largeur, size_t hauteur, int profondeur, bool retourner) {
	glGenTextures(1, &_tex);
	
	if(retourner) {
		GLubyte *imageRetournee = new GLubyte[largeur * hauteur * profondeur];
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<int>(largeur));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<int>(largeur), static_cast<int>(hauteur), 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	
	if(retourner) {
		delete[] img;
	}
	
	return this;
}

GLubyte const *Texture::TextureBase::pixels() const {
	GLubyte *pix = new GLubyte[_dimensions.x * _dimensions.y * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	
	return pix;
}

Texture::Texture(std::string const &fichier) throw(int) {
	_base = TextureBase::imageBase(fichier);
}

Texture::Texture(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner) {
	_base = TextureBase::imageBase(pixels, largeur, hauteur, profondeur, retourner);
}

Texture::Texture(Texture const &img) {
	_base = img._base;
	++_base->_nombreReferences;
}

Texture &Texture::operator=(Texture const &img) {
	_base = img._base;
	if(_base)
		++_base->_nombreReferences;
		
	return *this;
}

Texture::~Texture() {
	_base->detruire();
}

glm::ivec2 &Texture::dimensions() const {
	return _base->_dimensions;
}

GLubyte const *Texture::pixels() const {
	return _base->pixels();
}

std::string const &Texture::fichier() const {
	return _base->_fichier;
}

GLuint Texture::tex() const {
	return _base->_tex;
}

Bitmap::Bitmap(std::string const &fichier) throw(int) {
	SDL_Surface *surf = IMG_Load(fichier.c_str());
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
	
	_pixels = static_cast<GLubyte *>(surf->pixels);
	_largeur = surf->w;
	_hauteur = surf->h;
	_profondeur = surf->format->BytesPerPixel;
	surf->pixels = nullptr;
	SDL_FreeSurface(surf);
}
