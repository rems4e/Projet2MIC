//
//  Shader.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Shader_h
#define Projet2MIC_Shader_h

#include "Constantes.h"
#include "Session.h"
#include <exception>
#include <map>
#include <string>
#include GL_H

class Shader {
	friend void Session::nettoyer();
	friend void Session::initialiser();
public:
	static char const * const rayonFlou;
	static char const * const dim;
	static char const * const pos;
		
	class Exc_CreationImpossible : public std::exception {
	public:
		Exc_CreationImpossible() throw() : std::exception() { }
		virtual ~Exc_CreationImpossible() throw() { }
		virtual const char* what() const throw() { return "Création shader impossible"; }
	};

	Shader(std::string const &vert, std::string const &frag) throw(Exc_CreationImpossible);
	~Shader();
		
	static Shader const &aucun();
	
	static Shader const &flou(float rayon);
	
	void definirParametre(char const *param, float v) const;
	void definirParametre(char const *param, float v1, float v2) const;
	void definirParametre(char const *param, float v1, float v2, float v3) const;
	void definirParametre(char const *param, float v1, float v2, float v3, float v4) const;

	void activer() const;
	
	GLint vertCoord() const { return _vertCoord; }
	GLint texCoord() const { return _texCoord; }
	GLint coul() const { return _coul; }
	
	static void desactiver();
	
	static Shader const &shaderActuel();
		
protected:
	static void compiler(GLuint shader) throw(Exc_CreationImpossible);
	static void lier(GLint prog) throw(Exc_CreationImpossible);
	
	GLint locParam(char const *param) const;

private:
	GLint _vert;
	GLint _frag;
	GLint _prog;
	
	GLint _vertCoord, _texCoord, _coul;
			
	mutable std::map<char const *, GLint> _parametres;

	static std::map<std::pair<std::string, std::string>, std::pair<std::pair<GLint, std::pair<GLint, GLint> >, size_t> > *_programmes;
	static std::map<std::string, std::pair<GLint, size_t> > *_vertexShaders;
	static std::map<std::string, std::pair<GLint, size_t> > *_fragmentShaders;
	static Shader *_flou;
	static Shader *_aucun;
	
	static Shader const *_shaderActuel;
	
	static void initialiser();
	static void nettoyer();

	Shader &operator=(Shader const &);
	Shader(Shader const &);
};

#endif
