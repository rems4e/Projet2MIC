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
char const * const Shader::pos = "_pos";
char const * const Shader::temps = "_temps";

std::map<std::pair<std::string, std::string>, Shader::Programme> *Shader::_programmes = 0;
std::map<std::string, Shader::SousProgramme> *Shader::_vertexShaders = 0;
std::map<std::string, Shader::SousProgramme> *Shader::_fragmentShaders = 0;

Shader *Shader::_flou = 0;
Shader *Shader::_aucun = 0;
Shader const *Shader::_shaderActuel = 0;

bool Shader::init() {
	return _programmes;
}

void Shader::initialiser() {
	_programmes = new std::map<std::pair<std::string, std::string>, Programme>;
	_vertexShaders = new std::map<std::string, SousProgramme>;
	_fragmentShaders = new std::map<std::string, SousProgramme>;
}

void Shader::preReinitialiser() {
	for(std::map<std::pair<std::string, std::string>, Programme>::iterator i = _programmes->begin(); i != _programmes->end(); ++i) {
		Programme &p = i->second;
		SousProgramme &v = *p._vert, &f = *p._frag;
		
		glUseProgram(0);
		
		glDetachShader(p._id, v._id);
		 glDetachShader(p._id, f._id);
		 glDeleteProgram(p._id);
	}
	
	for(std::map<std::string, SousProgramme>::iterator i = _vertexShaders->begin(); i != _vertexShaders->end(); ++i) {
		SousProgramme &v = i->second;
		glDeleteShader(v._id);
	}
	
	for(std::map<std::string, SousProgramme>::iterator i = _fragmentShaders->begin(); i != _fragmentShaders->end(); ++i) {
		SousProgramme &f = i->second;
		glDeleteShader(f._id);
	}
}

void Shader::reinitialiser() throw(Shader::Exc_CreationImpossible) {
	for(std::map<std::pair<std::string, std::string>, Programme>::iterator i = _programmes->begin(); i != _programmes->end(); ++i) {
		Programme &p = i->second;
		SousProgramme &v = *p._vert, &f = *p._frag;
	}
	
	for(std::map<std::string, SousProgramme>::iterator i = _vertexShaders->begin(); i != _vertexShaders->end(); ++i) {
		SousProgramme &v = i->second;
		
		v._id = Shader::chargerShader(v._fichier, GL_VERTEX_SHADER);
		Shader::compiler(v._id);
	}

	for(std::map<std::string, SousProgramme>::iterator i = _fragmentShaders->begin(); i != _fragmentShaders->end(); ++i) {
		SousProgramme &f = i->second;
		
		f._id = Shader::chargerShader(f._fichier, GL_FRAGMENT_SHADER);
		Shader::compiler(f._id);
	}
		
	for(std::map<std::pair<std::string, std::string>, Programme>::iterator i = _programmes->begin(); i != _programmes->end(); ++i) {
		Programme &p = i->second;
		SousProgramme &v = *p._vert, &f = *p._frag;

		p._id = glCreateProgram();
		
		glAttachShader(p._id, v._id);
		glAttachShader(p._id, f._id);
		
		try {
			Shader::lier(p._id);
		}
		catch(Exc_CreationImpossible &e) {
			std::cerr << "Édition des liens du programme impossible : " + v._fichier + " " + f._fichier << std::endl;
			throw;
		}
		
		p._vertCoord =  glGetAttribLocation(p._id, "vertCoord");
		p._texCoord = glGetAttribLocation(p._id, "texCoord");
		p._coul = glGetAttribLocation(p._id, "color");
	}
	
	Shader const &a = Shader::shaderActuel();
	_shaderActuel = 0;
	a.activer();
}

Shader const &Shader::shaderActuel() {
	return *_shaderActuel;
}

Shader::Shader(std::string const &vert, std::string const &frag) throw(Shader::Exc_CreationImpossible) : _prog(0) {	
	Programme &idProgramme((*_programmes)[std::make_pair(vert, frag)]);
	SousProgramme &idVert((*_vertexShaders)[vert]);
	SousProgramme &idFrag((*_fragmentShaders)[frag]);

	// Le programme n'a pas été trouvé.
	if(idProgramme._ref == 0) {
		// On charge le vertex shader
		if(idVert._ref == 0) {
			idVert._fichier = vert;
			idVert._id = Shader::chargerShader(vert, GL_VERTEX_SHADER);
			
			Shader::compiler(idVert._id);
		}
		
		// On charge le fragment shader
		if(idFrag._ref == 0) {
			idFrag._fichier = frag;
			idFrag._id = Shader::chargerShader(frag, GL_FRAGMENT_SHADER);
			
			Shader::compiler(idFrag._id);
		}
		
		idProgramme._id = glCreateProgram();
		idProgramme._vert = &idVert;
		idProgramme._frag = &idFrag;
		
		glAttachShader(idProgramme._id, idVert._id);
		glAttachShader(idProgramme._id, idFrag._id);
		
		try {
			Shader::lier(idProgramme._id);
		}
		catch(Exc_CreationImpossible &e) {
			std::cerr << "Édition des liens du programme impossible : " + vert + " " + frag << std::endl;
			throw;
		}
		
		idProgramme._vertCoord =  glGetAttribLocation(idProgramme._id, "vertCoord");
		idProgramme._texCoord = glGetAttribLocation(idProgramme._id, "texCoord");
		idProgramme._coul = glGetAttribLocation(idProgramme._id, "color");
	}

	++idProgramme._ref;
	++idVert._ref;
	++idFrag._ref;
	
	_prog = &idProgramme;
}

Shader::~Shader() {
	if(_shaderActuel == this) {
		Shader::desactiver();
	}
		
	--_prog->_ref;
	--_prog->_vert->_ref;
	--_prog->_frag->_ref;
		
	if(_prog->_ref == 0) {		
		glDetachShader(_prog->_id, _prog->_vert->_id);
		glDetachShader(_prog->_id, _prog->_frag->_id);
		glDeleteProgram(_prog->_id);
		
		_programmes->erase(std::make_pair(_prog->_vert->_fichier, _prog->_frag->_fichier));
	}
	
	if(_prog->_vert->_ref == 0) {
		glDeleteShader(_prog->_vert->_id);
		_vertexShaders->erase(_prog->_vert->_fichier);
	}
	if(_prog->_frag->_ref == 0) {
		glDeleteShader(_prog->_frag->_id);
		_fragmentShaders->erase(_prog->_frag->_fichier);
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
	if(_shaderActuel != this) {
		this->activer();
	}
	else
		ImagesBase::changerTexture(-1);

	glUniform1f(this->locParam(param), v);
}

void Shader::definirParametre(char const *param, float v1, float v2) const {
	if(_shaderActuel != this) {
		this->activer();
	}
	else
		ImagesBase::changerTexture(-1);

	glUniform2f(this->locParam(param), v1, v2);
}

void Shader::definirParametre(char const *param, float v1, float v2, float v3) const {
	if(_shaderActuel != this) {
		this->activer();
	}
	else
		ImagesBase::changerTexture(-1);

	glUniform3f(this->locParam(param), v1, v2, v3);
}

void Shader::definirParametre(char const *param, float v1, float v2, float v3, float v4) const {
	if(_shaderActuel != this) {
		this->activer();
	}
	else
		ImagesBase::changerTexture(-1);

	glUniform4f(this->locParam(param), v1, v2, v3, v4);
}

GLint Shader::locParam(char const *param) const {
	GLint loc = -1;
	std::map<char const *, GLint>::iterator i = _parametres.find(param);
	if(i == _parametres.end()) {
		_parametres[param] = loc = glGetUniformLocation(_prog->_id, param);
	}
	else
		loc = i->second;
		
	return loc;
}

void Shader::activer() const {
	if(_shaderActuel != this) {
		ImagesBase::changerTexture(-1);
		_shaderActuel = this;
		glUseProgram(_prog->_id);

		this->definirParametre("_ecran", Ecran::largeur(), Ecran::hauteur());
	}
}

void Shader::desactiver() {
	Shader::aucun().activer();
}

Coordonnees Shader::versShader(Coordonnees const &c) {
	return Coordonnees(c.x / Ecran::largeur() * 2 - 1, -(c.y / Ecran::hauteur() * 2 - 1));
}

Rectangle Shader::versShader(Rectangle const &r) {
	return Rectangle(versShader(r.origine()), Coordonnees(r.largeur / Ecran::largeur(), r.hauteur / Ecran::hauteur()));

}

GLint Shader::chargerShader(std::string const &chemin, GLint type) throw(Shader::Exc_CreationImpossible) {
	GLint shad = glCreateShader(type);
	
	std::ifstream fShad(chemin.c_str(), std::ios::in);
	if(!fShad) {
		std::cerr << "Shader introuvable : " << chemin << std::endl;
		throw Exc_CreationImpossible();
	}
	
	fShad.seekg(0, std::ios::end);
	int dimShad = fShad.tellg();
	GLchar *shadSource = new GLchar[dimShad];
	fShad.seekg(0, std::ios::beg);
	fShad.read(shadSource, dimShad);
	fShad.close();
	
	glShaderSource(shad, 1, const_cast<GLchar const **>(&shadSource), &dimShad);
	
	delete[] shadSource;
	
	return shad;
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
		
		std::cerr << "Erreur de compilation du shader : " << str << std::endl;
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
