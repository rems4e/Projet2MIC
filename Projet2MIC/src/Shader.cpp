//
//  Shader.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Shader.h"
#include <iostream>
#include <fstream>
#include "Session.h"
#include <cstdlib>
#include "Affichage.h"

char const * const Shader::rayonFlou = "rayon";
char const * const Shader::dim = "_dim";
char const * const Shader::pos = "_pos";
char const * const Shader::temps = "_temps";
char const * const Shader::tempsAbsolu = "_tempsAbsolu";

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
	for(std::map<std::string, SousProgramme>::iterator i = _vertexShaders->begin(); i != _vertexShaders->end(); ++i) {
		SousProgramme &v = i->second;
		
		v._id = Shader::chargerShader(v._fichier, GL_VERTEX_SHADER);
		Shader::compiler(v._id, v._fichier);
	}

	for(std::map<std::string, SousProgramme>::iterator i = _fragmentShaders->begin(); i != _fragmentShaders->end(); ++i) {
		SousProgramme &f = i->second;
		
		f._id = Shader::chargerShader(f._fichier, GL_FRAGMENT_SHADER);
		Shader::compiler(f._id, f._fichier);
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
		p._norm = glGetAttribLocation(p._id, "norm");
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
			
			Shader::compiler(idVert._id, vert);
		}
		
		// On charge le fragment shader
		if(idFrag._ref == 0) {
			idFrag._fichier = frag;
			idFrag._id = Shader::chargerShader(frag, GL_FRAGMENT_SHADER);
			
			Shader::compiler(idFrag._id, frag);
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
		idProgramme._norm = glGetAttribLocation(idProgramme._id, "norm");
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
	
	SousProgramme *vert = _prog->_vert, *frag = _prog->_frag;
	
	if(_prog->_ref == 0) {		
		glDetachShader(_prog->_id, _prog->_vert->_id);
		glDetachShader(_prog->_id, _prog->_frag->_id);
		glDeleteProgram(_prog->_id);
		
		_programmes->erase(std::make_pair(_prog->_vert->_fichier, _prog->_frag->_fichier));
	}

	if(vert->_ref == 0) {
		glDeleteShader(vert->_id);
		_vertexShaders->erase(vert->_fichier);
	}
	if(frag->_ref == 0) {
		glDeleteShader(frag->_id);
		_fragmentShaders->erase(frag->_fichier);
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
	GLint loc = this->locParam(param);
	if(loc == -1)
		return;

	if(_shaderActuel != this) {
		this->activer();
	}
	else
		Affichage::changerTexture(-1);

	glUniform1f(loc, v);
}

void Shader::definirParametre(char const *param, glm::vec2 const &vec) const {
	GLint loc = this->locParam(param);
	if(loc == -1)
		return;

	if(_shaderActuel != this) {
		this->activer();
	}
	else
		Affichage::changerTexture(-1);

	glUniform2fv(loc, 1, &vec[0]);
}

void Shader::definirParametre(char const *param, glm::vec3 const &vec) const {
	GLint loc = this->locParam(param);
	if(loc == -1)
		return;

	if(_shaderActuel != this) {
		this->activer();
	}
	else
		Affichage::changerTexture(-1);

	glUniform3fv(loc, 1, &vec[0]);
}

void Shader::definirParametre(char const *param, glm::vec4 const &vec) const {
	GLint loc = this->locParam(param);
	if(loc == -1)
		return;

	if(_shaderActuel != this) {
		this->activer();
	}
	else
		Affichage::changerTexture(-1);

	glUniform4fv(loc, 1, &vec[0]);
}

void Shader::definirParametre(char const *param, glm::mat3 const &mat) const {
	GLint loc = this->locParam(param);
	if(loc == -1)
		return;
	
	if(_shaderActuel != this) {
		this->activer();
	}
	else
		Affichage::changerTexture(-1);
	
	glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
}

void Shader::definirParametre(char const *param, glm::mat4 const &mat) const {
	GLint loc = this->locParam(param);
	if(loc == -1)
		return;
	
	if(_shaderActuel != this) {
		this->activer();
	}
	else
		Affichage::changerTexture(-1);
	
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
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

void Shader::majModele() {
	Shader const &s = Shader::shaderActuel();
	s.definirParametre("_matModeleVue", Ecran::modeleVue());
}

void Shader::majProjection() {
	Shader const &s = Shader::shaderActuel();
	s.definirParametre("_matProjection", Ecran::projection());
}

void Shader::activer() const {
	if(_shaderActuel != this) {
		Affichage::changerTexture(-1);
		_shaderActuel = this;
		glUseProgram(_prog->_id);

		this->definirParametre("_ecran", glm::vec2(Ecran::largeur(), Ecran::hauteur()));
		Shader::majModele();
		Shader::majProjection();
	}
}

void Shader::desactiver() {
	Shader::aucun().activer();
}

glm::vec2 Shader::versShader(glm::vec2 const &c) {
	return glm::vec2(c.x / Ecran::largeur() * 2 - 1, -(c.y / Ecran::hauteur() * 2 - 1));
}

Rectangle Shader::versShader(Rectangle const &r) {
	return Rectangle(versShader(r.origine()), glm::vec2(r.largeur / Ecran::largeur(), r.hauteur / Ecran::hauteur()));

}

GLint Shader::chargerShader(std::string const &chemin, GLint type) throw(Shader::Exc_CreationImpossible) {
	GLint shad = glCreateShader(type);
	
	std::ifstream fShad(chemin.c_str(), std::ios::in);
	if(!fShad) {
		std::cerr << "Shader introuvable : " << chemin << std::endl;
		throw Exc_CreationImpossible();
	}
	
	fShad.seekg(0, std::ios::end);
	int dimShad = static_cast<int>(fShad.tellg());
	GLchar *shadSource = new GLchar[dimShad];
	fShad.seekg(0, std::ios::beg);
	fShad.read(shadSource, dimShad);
	fShad.close();
	
	glShaderSource(shad, 1, const_cast<GLchar const **>(&shadSource), &dimShad);
	
	delete[] shadSource;
	
	return shad;
}

void Shader::compiler(GLuint shader, std::string const &fichier) throw(Shader::Exc_CreationImpossible) {
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
		
		std::cerr << "Erreur de compilation du shader (" << fichier << ") : " << str << std::endl;
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
