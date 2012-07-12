//
//  Image.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Image.h"
#include <map>
#include <SDL/SDL.h>
#include <SDL_image/SDL_image.h>
#include <cmath>
#include <cstring>
#include <vector>

#define TAMPON_SOMMETS 200000

namespace ImagesBase {
	std::map<std::string, ImageBase *> *_images = 0;
	std::list<ImageBase *> _imagesSansFichier;
	
	void initialiser();
	void preReinitialiser();
	void reinitialiser();
	
	void nettoyer();
	
	struct SauveImage {
		int _largeur, _hauteur;
		unsigned char *_pixels;
	};
	
	std::list<SauveImage> _sauveImages;
	
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

void ImagesBase::preReinitialiser() {
	for(std::list<ImageBase *>::iterator i = ImagesBase::_imagesSansFichier.begin(); i != ImagesBase::_imagesSansFichier.end(); ++i) {
		SauveImage sauve;
		sauve._largeur = (*i)->_dimensions.x;
		sauve._hauteur = (*i)->_dimensions.y;
		sauve._pixels = new unsigned char[sauve._largeur * sauve._hauteur * 4];
		glBindTexture(GL_TEXTURE_2D, (*i)->_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, sauve._pixels);
		_sauveImages.push_back(sauve);

		glDeleteTextures(1, &((*i)->_tex));
	}
		
	for(std::map<std::string, ImageBase *>::iterator i = ImagesBase::_images->begin(); i != ImagesBase::_images->end(); ++i) {
		glDeleteTextures(1, &i->second->_tex);
	}
}

void ImagesBase::reinitialiser() {
	for(std::map<std::string, ImageBase *>::iterator i = ImagesBase::_images->begin(); i != ImagesBase::_images->end(); ++i) {
		i->second->charger(i->second->_fichier);
	}
	
	for(std::list<ImageBase *>::iterator i = ImagesBase::_imagesSansFichier.begin(); i != ImagesBase::_imagesSansFichier.end(); ++i) {
		SauveImage sauve = _sauveImages.front();

		(*i)->charger(sauve._pixels, sauve._largeur, sauve._hauteur, 4, false);
		delete[] sauve._pixels;
		_sauveImages.pop_front();
	}
}

void ImagesBase::ajouterSommet(Coordonnees const &pos, Coordonnees const &posTex, Couleur const &couleur) {
	if(_vertCoords.size() / 2 <= _nbSommets) {
		_vertCoords.resize(_vertCoords.size() + _vertCoords.size() / 10);
		_texCoords.resize(_texCoords.size() + _texCoords.size() / 10);
		_couleurs.resize(_couleurs.size() + _couleurs.size() / 10);
		
		std::cout << "Dimensionnement des tableaux de sommets sous-évalués. Nouvelle taille : " << _vertCoords.size() / 2 << std::endl;
	}
	
	_vertCoords[_nbSommets * 2] = pos.x / Ecran::largeur() * 2 - 1;
	_vertCoords[_nbSommets * 2 + 1] = -(pos.y / Ecran::hauteur() * 2 - 1);

	_texCoords[_nbSommets * 2] = posTex.x;
	_texCoords[_nbSommets * 2 + 1] = posTex.y;

	_couleurs[_nbSommets * 4] = couleur.r;
	_couleurs[_nbSommets * 4 + 1] = couleur.v;
	_couleurs[_nbSommets * 4 + 2] = couleur.b;
	_couleurs[_nbSommets * 4 + 3] = couleur.a;

	++_nbSommets;
	/*static int nb = 0;
	static horloge_t h = horloge();
	nb += _nbSommets;
	float H = horloge();
	if(H - h >= 1.0f) {
		std::cout << nb / (H - h) << std::endl;
		
		h = H;
		nb = 0;
	}*/
}

void ImagesBase::changerTexture(GLint tex) {
	if(tex != _tex) {
		if(_nbSommets) {			
			glVertexAttribPointer(Shader::shaderActuel().vertCoord(), 2, GL_FLOAT, GL_FALSE, 0, &_vertCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().texCoord(), 2, GL_FLOAT, GL_FALSE, 0, &_texCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().coul(), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &_couleurs[0]);
			
			glEnableVertexAttribArray(Shader::shaderActuel().vertCoord());
			glEnableVertexAttribArray(Shader::shaderActuel().texCoord());
			glEnableVertexAttribArray(Shader::shaderActuel().coul());
			
			glDrawArrays(GL_TRIANGLES, 0, _nbSommets);
			
			glDisableVertexAttribArray(Shader::shaderActuel().vertCoord());
			glDisableVertexAttribArray(Shader::shaderActuel().texCoord());
			glDisableVertexAttribArray(Shader::shaderActuel().coul());

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
	ImageBase *image = new ImageBase(img, largeur, hauteur, profondeur, retourner);
	ImagesBase::_imagesSansFichier.push_back(image);
	
	return image;
}

void ImageBase::detruire() {
	--_nombreReferences;
	if(_nombreReferences <= 0) {
		if(!_fichier.empty()) {
			ImagesBase::_images->erase(_fichier);
		}
		else {
			ImagesBase::_imagesSansFichier.remove(this);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

Image::Image(std::string const &fichier) : _base(0), _facteur(Coordonnees::un), _angle(0.0f) {
	if(fichier.size())
		_base = ImageBase::imageBase(fichier);
}

Image::Image(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner) : _base(0), _facteur(Coordonnees::un), _angle(0.0f) {
	_base = ImageBase::imageBase(pixels, largeur, hauteur, profondeur, retourner);

}

Image::Image(Image const &img) : _base(0), _facteur(Coordonnees::un), _angle(0.0f) {
	if(img.valide()) {
		_base = img._base;
		++_base->_nombreReferences;
	}
}

Image::Image() : _base(0), _facteur(Coordonnees::un), _angle(0.0f) {
	
}

Image &Image::operator=(Image const &img) {
	_base = img._base;
	if(_base)
		++_base->_nombreReferences;
		
	_facteur = img._facteur;

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
	return _base->_dimensions.etirer(_facteur);
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

Image const &Image::redimensionner(coordonnee_t facteur) const {
	return this->redimensionner(Coordonnees::un * facteur);
}

Image const &Image::redimensionner(Coordonnees const &facteur) const {
	_facteur = facteur;
	
	return *this;
}

void Image::afficher(Coordonnees const &position, Rectangle const &filtre) const {
	Rectangle vert(position, Coordonnees(filtre.largeur * _facteur.x, filtre.hauteur * _facteur.y));
	
	if(!vert.superposition(Ecran::ecran()))
		return;
	
	Shader::shaderActuel().definirParametre(Shader::pos, position.x, position.y);
	Shader::shaderActuel().definirParametre(Shader::dim, vert.largeur, vert.hauteur);
		
	ImagesBase::changerTexture(_base->_tex);
		
	Couleur c(255 - Image::_teinte.a * (255 - Image::_teinte.r) / 255, 255 - Image::_teinte.a * (255 - Image::_teinte.v) / 255, 255 - Image::_teinte.a * (255 - Image::_teinte.b) / 255, Image::_opacite);

	ImagesBase::ajouterSommet(Coordonnees(vert.gauche, vert.haut), Coordonnees(filtre.gauche / _base->_dimensions.x, filtre.haut / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche + vert.largeur, vert.haut), Coordonnees((filtre.gauche + filtre.largeur) / _base->_dimensions.x, filtre.haut / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche, vert.haut + vert.hauteur), Coordonnees(filtre.gauche / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y), c);

	ImagesBase::ajouterSommet(Coordonnees(vert.gauche, vert.haut + vert.hauteur), Coordonnees(filtre.gauche / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche + vert.largeur, vert.haut), Coordonnees((filtre.gauche + filtre.largeur) / _base->_dimensions.x, filtre.haut / _base->_dimensions.y), c);
	ImagesBase::ajouterSommet(Coordonnees(vert.gauche + vert.largeur, vert.haut + vert.hauteur), Coordonnees((filtre.gauche + filtre.largeur) / _base->_dimensions.x, (filtre.haut + filtre.hauteur) / _base->_dimensions.y), c);
}
