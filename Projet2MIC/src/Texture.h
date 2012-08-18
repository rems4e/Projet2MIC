//
//  Texture.h
//  Projet2MIC
//
//  Created by Rémi on 17/08/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef EN_TETE_TEXTURE
#define EN_TETE_TEXTURE

#include "Constantes.h"

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#elif defined(__WIN32__)
#include <GL/glew.h>
#else
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif

#include <string>
#include "Geometrie.h"
#include <unordered_map>
#include <list>

namespace Session {
	void nettoyer();
}

class Texture final {
	//friend void Session::nettoyer();
	//friend void modifierResolution(unsigned int largeur, unsigned int hauteur, bool pleinEcran);
public:
	Texture(std::string const &fichier) throw(int);
	// On créé un image à partir d'un tableau 2D de pixels, rangés un rang après l'autre, avec 'profondeur' octet pour chaque pixel.
	Texture(GLubyte const *pixels, size_t largeur, size_t hauteur, int profondeur, bool retourner = false);
	Texture(Texture const &img);
	Texture &operator=(Texture const &img);
	~Texture();
		
	glm::ivec2 &dimensions() const;
	
	// Pixels de l'image. Coûteux à obtenir. Le pointeur doit être passé à delete[] après utilisation.
	GLubyte const *pixels() const;
	
	// Fichier de l'image ou chaîne vide si l'image a été générée à partir d'une matrice de pixels
	std::string const &fichier() const;
	
	GLuint tex() const;
public:
	struct TextureBase;
private:
	TextureBase *_base = 0;
};

struct Bitmap final {
public:
	Bitmap(std::string const &fichier) throw(int);
	
	inline ~Bitmap() {
		delete[] _pixels;
	}
	
	inline size_t largeur() const {
		return _largeur;
	}
	
	inline size_t hauteur() const {
		return _hauteur;
	}
	
	inline int profondeur() const {
		return _profondeur;
	}
	
	inline GLubyte const *pixels() const {
		return _pixels;
	}
	
	inline GLubyte *piquerPixels() {
		GLubyte *pix = _pixels;
		_pixels = nullptr;
		return pix;
	}
	
private:
	GLubyte * _pixels;
	size_t _largeur;
	size_t _hauteur;
	int _profondeur;
	
	Bitmap(Bitmap const &) = delete;
	Bitmap &operator=(Bitmap const &) = delete;
};

#endif
