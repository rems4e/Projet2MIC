//
//  Shader.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Shader.h"
#include <iostream>
#include <fstream>
#include "Session.h"
#include <cstdlib>
#include "Ecran.h"

namespace ImagesBase {
	void changerTexture(GLint tex);
}

char const * const Shader::rayonFlou = "rayon";
char const * const Shader::dim = "_dim";

std::map<std::pair<std::string, std::string>, std::pair<std::pair<GLint, std::pair<GLint, GLint> >, size_t> > *Shader::_programmes;
std::map<std::string, std::pair<GLint, size_t> > *Shader::_vertexShaders;
std::map<std::string, std::pair<GLint, size_t> > *Shader::_fragmentShaders;

Shader *Shader::_flou = 0;
Shader *Shader::_aucun = 0;
Shader const *Shader::_shaderActuel = 0;

void Shader::initialiser() {
	_programmes = new std::map<std::pair<std::string, std::string>, std::pair<std::pair<GLint, std::pair<GLint, GLint> >, size_t> >;
	_vertexShaders = new std::map<std::string, std::pair<GLint, size_t> >;
	_fragmentShaders = new std::map<std::string, std::pair<GLint, size_t> >;
	Shader::aucun().activer();
}

Shader const &Shader::shaderActuel() {
	return *_shaderActuel;
}

Shader::Shader(std::string const &vert, std::string const &frag) throw(Shader::Exc_CreationImpossible) : _frag(0), _vert(0), _prog(0) {	
	std::pair<std::pair<GLint, std::pair<GLint, GLint> >, size_t> &idProgramme((*_programmes)[std::make_pair(vert, frag)]);
	std::pair<GLint, size_t> &idVert((*_vertexShaders)[vert]);
	std::pair<GLint, size_t> &idFrag((*_fragmentShaders)[frag]);

	// Le programme n'a pas été trouvé.
	if(idProgramme.second == 0) {
		// On charge le vertex shader
		if(idVert.second == 0) {
			_vert = glCreateShader(GL_VERTEX_SHADER);
			idVert.first = _vert;
			
			std::ifstream fVert(vert.c_str(), std::ios::in);
			if(!fVert) {
				std::cerr << "Vertex shader introuvable : " << vert << std::endl;
				throw Exc_CreationImpossible();
			}
			
			// Chargement du contenu
			fVert.seekg(0, std::ios::end);
			int dimVert = fVert.tellg();
			GLchar *vertSource = new GLchar[dimVert];
			fVert.seekg(0, std::ios::beg);
			fVert.read(vertSource, dimVert);
			fVert.close();
			
			glShaderSource(_vert, 1, const_cast<GLchar const **>(&vertSource), &dimVert);
			delete[] vertSource;
			
			try {
				Shader::compiler(_vert);
			}
			catch(Exc_CreationImpossible &e) {
				std::cerr << "Compilation du vertex shader impossible : " + vert << std::endl;
				throw;
			}
		}
		else {
			//std::cout << vert << std::endl;
			_vert = idVert.first;
		}
		
		// On charge le fragment shader
		if(idFrag.second == 0) {
			_frag = glCreateShader(GL_FRAGMENT_SHADER);
			idFrag.first = _frag;

			std::ifstream fFrag(frag.c_str(), std::ios::in);
			if(!fFrag) {
				std::cerr << "Fragment shader introuvable : " << frag << std::endl;
				throw Exc_CreationImpossible();
			}
			
			fFrag.seekg(0, std::ios::end);
			int dimFrag = fFrag.tellg();
			GLchar *fragSource = new GLchar[dimFrag];
			fFrag.seekg(0, std::ios::beg);
			fFrag.read(fragSource, dimFrag);
			fFrag.close();
						
			glShaderSource(_frag, 1, const_cast<GLchar const **>(&fragSource), &dimFrag);
			
			delete [] fragSource;
			
			try {
				Shader::compiler(_frag);
			}
			catch(Exc_CreationImpossible &e) {
				std::cerr << "Compilation du fragment shader impossible : " + frag << std::endl;
				throw;
			}
		}
		else {
			_frag = idFrag.first;
		}
		
		_prog = glCreateProgram();
		idProgramme.first.first = _prog;
		idProgramme.first.second.first = _vert;
		idProgramme.first.second.second = _frag;
		
		glAttachShader(_prog, _vert);
		glAttachShader(_prog, _frag);
		
		try {
			Shader::lier(_prog);
		}
		catch(Exc_CreationImpossible &e) {
			std::cerr << "Édition des liens du programme impossible : " + vert + " " + frag << std::endl;
			throw;
		}
		
		_vertCoord =  glGetAttribLocation(_prog, "vertCoord");
		_texCoord = glGetAttribLocation(_prog, "texCoord");
		_coul = glGetAttribLocation(_prog, "color");
	}
	else {
		_prog = idProgramme.first.first;
		_vert = idProgramme.first.second.first;
		_frag = idProgramme.first.second.second;
	}
	++idProgramme.second;
	++idVert.second;
	++idFrag.second;
}

Shader::~Shader() {
	if(_shaderActuel == this) {
		Shader::desactiver();
	}

	std::map<std::pair<std::string, std::string>, std::pair<std::pair<GLint, std::pair<GLint, GLint> >, size_t> >::iterator prog = _programmes->begin();
	std::map<std::string, std::pair<GLint, size_t> >::iterator vert = _vertexShaders->begin();
	std::map<std::string, std::pair<GLint, size_t> >::iterator frag = _fragmentShaders->begin();

	for(; prog != _programmes->end(); ++prog) {
		if(prog->second.first.first == _prog)
			break;
	}
	for(; vert != _vertexShaders->end(); ++vert) {
		if(vert->second.first == _vert)
			break;
	}
	for(; frag != _fragmentShaders->end(); ++frag) {
		if(frag->second.first == _frag)
			break;
	}
		
	--prog->second.second;
	--vert->second.second;
	--frag->second.second;
		
	if(prog->second.second == 0) {		
		glDetachShader(_prog, _vert);
		glDetachShader(_prog, _frag);
		glDeleteProgram(_prog);
		
		_programmes->erase(prog);
	}
	
	if(vert->second.second == 0) {
		glDeleteShader(_vert);
		_vertexShaders->erase(vert);
	}
	if(frag->second.second == 0) {
		glDeleteShader(_frag);
		_fragmentShaders->erase(frag);
	}
}

Shader const &Shader::aucun() {
	if(!_aucun) {
		_aucun = new Shader(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "aucun.frag");
	}

	return *_aucun;
}

Shader const &Shader::flou(float rayon) {
	if(!_flou) {
		_flou = new Shader(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "flou.frag");
	}
	_flou->definirParametre(rayonFlou, rayon);
	
	return *_flou;
}

void Shader::nettoyer() {
	delete _flou;
	delete _aucun;
	delete _programmes;
	delete _vertexShaders;
	delete _fragmentShaders;
}

void Shader::definirParametre(char const *param, float v) const {
	if(_shaderActuel != this)
		glUseProgram(_prog);
	else
		ImagesBase::changerTexture(-1);

	glUniform1f(this->locParam(param), v);
}

void Shader::definirParametre(char const *param, float v1, float v2) const {
	if(_shaderActuel != this)
		glUseProgram(_prog);
	else
		ImagesBase::changerTexture(-1);

	glUniform2f(this->locParam(param), v1, v2);
}

void Shader::definirParametre(char const *param, float v1, float v2, float v3) const {
	if(_shaderActuel != this)
		glUseProgram(_prog);
	else
		ImagesBase::changerTexture(-1);

	glUniform3f(this->locParam(param), v1, v2, v3);
}

void Shader::definirParametre(char const *param, float v1, float v2, float v3, float v4) const {
	if(_shaderActuel != this)
		glUseProgram(_prog);
	else
		ImagesBase::changerTexture(-1);

	glUniform4f(this->locParam(param), v1, v2, v3, v4);
}

GLint Shader::locParam(char const *param) const {
	GLint &loc = _parametres[param];
	if(loc == 0) {
		loc = glGetUniformLocation(_prog, param);
	}

	return loc;
}

void Shader::activer() const {
	if(_shaderActuel != this) {
		ImagesBase::changerTexture(-1);
		_shaderActuel = this;
		glUseProgram(_prog);
		
		this->definirParametre("_ecran", Ecran::largeur(), Ecran::hauteur());
	}
}

void Shader::desactiver() {
	Shader const &aucun = Shader::aucun();
	if(_shaderActuel != &aucun) {
		aucun.activer();
	}
}

Coordonnees Shader::versShader(Coordonnees const &c) {
	return Coordonnees(c.x / Ecran::largeur() * 2 - 1, -(c.y / Ecran::hauteur() * 2 - 1));
}

Rectangle Shader::versShader(Rectangle const &r) {
	return Rectangle(versShader(r.origine()), Coordonnees(r.largeur / Ecran::largeur(), r.hauteur / Ecran::hauteur()));

}

void Shader::compiler(GLuint shader) throw(Shader::Exc_CreationImpossible) {
	glCompileShader(shader);

	GLint etat = 0;
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &etat);
	if(etat != GL_TRUE) {
		GLint tailleLog = 0;
		GLchar *log = 0;
		
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &tailleLog);
		
		std::string str;
		if(tailleLog) {
			log = new GLchar[tailleLog];
			glGetShaderInfoLog(shader, tailleLog, &tailleLog, log);
			log[tailleLog - 1] = '\0';
			
			str = log;
			delete[] log;
		}
		
		std::cerr << "Erreur de compilation : " << str << std::endl;
		throw Exc_CreationImpossible();
	}
}

void Shader::lier(GLint prog) throw(Exc_CreationImpossible) {
	glLinkProgram(prog);
	
	GLint etat = 0;
	
	glGetProgramiv(prog , GL_LINK_STATUS  , &etat);
	if(etat != GL_TRUE) {
		GLint tailleLog = 0;
		GLchar *log = 0;
		
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &tailleLog);
		
		std::string str;
		if(tailleLog) {
			log = new GLchar[tailleLog];
		
			glGetProgramInfoLog(prog, tailleLog, &tailleLog, log);
			log[tailleLog - 1] = '\0';
			
			str = log;
			delete[] log;
		}
		
		std::cerr << "Erreur lors de l'édition des liens du programme de shaders : " << str << std::endl;;
		throw Exc_CreationImpossible();
	}
}
