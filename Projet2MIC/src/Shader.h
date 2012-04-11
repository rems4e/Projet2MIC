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
#include <exception>
#include <map>
#include <string>
#include GL_H

class Shader {
public:
	static std::string const rayonFlou;
	
	typedef std::map<std::string, float>::const_iterator const_iterator;
	
	class Exc_CreationImpossible : public std::exception {
	public:
		Exc_CreationImpossible() throw() : std::exception() { }
		virtual ~Exc_CreationImpossible() throw() { }
		virtual const char* what() const throw() { return "Création shader impossible"; }
	};

	Shader(std::string const &vert, std::string const &frag) throw(Exc_CreationImpossible);
	~Shader();
	
	GLint programme() const;
	
	static Shader const &aucun();
	
	static Shader const &flou(float rayon);
	
	const_iterator premierParametre() const;
	const_iterator dernierParametre() const;
	
	void definirParametre(std::string const &param, float valeur) const;
	void activer() const;
		
protected:
	static void compiler(GLuint shader) throw(Exc_CreationImpossible);
	static void lier(GLint prog) throw(Exc_CreationImpossible);

private:
	GLint _vert;
	GLint _frag;
	GLint _prog;
	
	mutable std::map<std::string, float> _parametres;
};

#endif
