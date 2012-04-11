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

std::string const Shader::rayonFlou("rayon");

Shader::Shader(std::string const &vert, std::string const &frag) throw(Shader::Exc_CreationImpossible) : _frag(0), _vert(0), _prog(0) {
	// On ouvre les fichiers
	std::ifstream fVert(vert.c_str(), std::ios::in);
	if(!fVert) {
		std::cerr << "Vertex shader introuvable : " << vert << std::endl;
		throw Exc_CreationImpossible();
	}
	
	std::ifstream fFrag(frag.c_str(), std::ios::in);
	if(!fFrag) {
		std::cerr << "Fragment shader introuvable : " << frag << std::endl;
		throw Exc_CreationImpossible();
	}
	
	// Chargement du contenu
	fVert.seekg(0, std::ios::end);
	int dimVert = fVert.tellg();
	GLchar *vertSource = new GLchar[dimVert];
	fVert.seekg(0, std::ios::beg);
	fVert.read(vertSource, dimVert);
	fVert.close();
	
	fFrag.seekg(0, std::ios::end);
	int dimFrag = fFrag.tellg();
	GLchar *fragSource = new GLchar[dimFrag];
	fFrag.seekg(0, std::ios::beg);
	fFrag.read(fragSource, dimFrag);
	fFrag.close();
	
	// Création du programme
	_vert = glCreateShader(GL_VERTEX_SHADER);
	_frag = glCreateShader(GL_FRAGMENT_SHADER);
		
	glShaderSource(_vert, 1, const_cast<GLchar const **>(&vertSource), &dimVert);
	glShaderSource(_frag, 1, const_cast<GLchar const **>(&fragSource), &dimFrag);
	
	delete[] vertSource;
	delete [] fragSource;
		
	try {
		Shader::compiler(_vert);
	}
	catch(Exc_CreationImpossible &e) {
		std::cerr << "Compilation du vertex shader impossible : " + vert << std::endl;
		throw;
	}

	try {
		Shader::compiler(_frag);
	}
	catch(Exc_CreationImpossible &e) {
		std::cerr << "Compilation du fragment shader impossible : " + frag << std::endl;
		throw;
	}
	
	_prog = glCreateProgram();
	
	glAttachShader(_prog, _vert);
	glAttachShader(_prog, _frag);
	
	Shader::lier(_prog);
}

Shader::~Shader() {
	if(glIsProgram(_prog)) {
		glUseProgram(0);
	}
	
	glDetachShader(_prog, _vert);
	glDetachShader(_prog, _frag);
	glDeleteProgram(_prog);
	glDeleteShader(_vert);
	glDeleteShader(_frag);
}

GLint Shader::programme() const {
	return _prog;
}

Shader const &Shader::aucun() {
	static Shader const s(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "aucun.frag");
	return s;
}

Shader const &Shader::flou(float rayon) {
	static Shader const s(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "flou.frag");

	s.definirParametre(rayonFlou, rayon);
	
	return s;
	
}

std::map<std::string, float>::const_iterator Shader::premierParametre() const {
	return _parametres.begin();
}

std::map<std::string, float>::const_iterator Shader::dernierParametre() const {
	return _parametres.end();
}

void Shader::definirParametre(std::string const &param, float valeur) const {
	_parametres[param] = valeur;
}

void Shader::activer() const {
	glUseProgram(_prog);
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