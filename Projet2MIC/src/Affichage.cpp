//
//  Affichage.cpp
//  Projet2MIC
//
//  Created by Rémi on 12/08/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

//#define VBO
//#define STAT

#include "Affichage.h"
#include "Constantes.h"
#include <stack>
#include "Shader.h"

#ifdef STAT
#include "horloge.h"
#endif

#define TAMPON_SOMMETS 200000

namespace Affichage {	
	void initialiser();
	
	size_t const aucunePosition = static_cast<size_t>(-1);
			
	GLint _tex = -1;
	std::vector<GLfloat> _vertCoords;
	std::vector<GLfloat> _texCoords;
	std::vector<GLubyte> _couleurs;
#ifdef VBO
	GLuint _buf;
#endif
	size_t _nbSommets = 0;
	std::stack<Couleur::composante_t> _opacite;
	Couleur _teinte = Couleur::blanc;
	
	// Vérifie que les tableaux d'attributs de sommets sont assez grands pour nbSup nouveaux sommets. Redimensionne les tableaux le cas échéant et retourne faux, sinon, retoure vrai.
	bool verifierDimensions(size_t nbSup);
}

void Affichage::initialiser() {
	_vertCoords.resize(3 * TAMPON_SOMMETS);
	_couleurs.resize(4 * TAMPON_SOMMETS);
	_texCoords.resize(2 * TAMPON_SOMMETS);
	
	_opacite.push(255);
}

#ifdef STAT
static int nbVert = 0;
#endif

bool Affichage::verifierDimensions(size_t nbSup) {
	bool redim = false;
	while(_nbSommets + nbSup >= _vertCoords.size() / 3) {
		redim = true;
		
		_vertCoords.resize(_vertCoords.size() + _vertCoords.size() / 10);
		_texCoords.resize(_texCoords.size() + _texCoords.size() / 10);
		_couleurs.resize(_couleurs.size() + _couleurs.size() / 10);
		
#ifdef VBO
		glBindBuffer(GL_ARRAY_BUFFER, _buf);
		glBufferData(GL_ARRAY_BUFFER, _vertCoords.size() / 3 * (7 * sizeof(GLfloat) + 4 * sizeof(GLubyte)), 0, GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
		
		std::cout << "Dimensionnement des tableaux de sommets sous-évalués. Nouvelle taille : " << _vertCoords.size() / 3 << std::endl;
	}
	
	return !redim;
}

void Affichage::ajouterSommet(glm::vec2 const &pos, glm::vec2 const &posTex, Couleur const &couleur, bool depuisEcran) {
	if(depuisEcran) {
		_vertCoords[_nbSommets * 3] = pos.x / Ecran::cadreAffichage().largeur * 2 - 1;
		_vertCoords[_nbSommets * 3 + 1] = -(pos.y / Ecran::cadreAffichage().hauteur * 2 - 1);
		_vertCoords[_nbSommets * 3 + 2] = 0;
	}
	else {
		_vertCoords[_nbSommets * 3] = pos.x;
		_vertCoords[_nbSommets * 3 + 1] = pos.y;
		_vertCoords[_nbSommets * 3 + 2] = 0;
	}
	
	_texCoords[_nbSommets * 2] = posTex.x;
	_texCoords[_nbSommets * 2 + 1] = posTex.y;
	
	_couleurs[_nbSommets * 4] = couleur.r;
	_couleurs[_nbSommets * 4 + 1] = couleur.v;
	_couleurs[_nbSommets * 4 + 2] = couleur.b;
	_couleurs[_nbSommets * 4 + 3] = couleur.a;
	
	++_nbSommets;
#ifdef STAT
	++nbVert;
#endif
}

void Affichage::ajouterSommets(size_t sommets, size_t composantesSommet, coordonnee_t const *pos, coordonnee_t const *couleurs, GLushort const *index) {
	Affichage::changerTexture(Ecran::vide().tex());
	Affichage::verifierDimensions(sommets);
	static glm::vec2 tex[3] = {glm::vec2(0.0, 0.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0)};
	for(index_t i = 0; i < sommets; ++i) {
		index_t indx = i;
		if(index) {
			indx = index[i];
		}
		Affichage::ajouterSommet(glm::vec2(pos[composantesSommet * indx], pos[composantesSommet * indx + 1]), tex[i % 3], Couleur(couleurs[4 * indx] * 255, couleurs[4 * indx + 1] * 255, couleurs[4 * indx + 2] * 255, couleurs[4 * indx + 3] * 255), false);
	}
}

void Affichage::ajouterSommets(size_t sommets, size_t composantesSommet, coordonnee_t const *pos, coordonnee_t const *posTex, coordonnee_t const *couleurs) {
	Affichage::verifierDimensions(sommets);
	for(index_t i = 0; i < sommets; ++i) {
		Affichage::ajouterSommet(glm::vec2(pos[composantesSommet * i], pos[composantesSommet * i + 1]), glm::vec2(posTex[2 * i], posTex[2 * i + 1]), Couleur(couleurs[4 * i] * 255, couleurs[4 * i + 1] * 255, couleurs[4 * i + 2] * 255, couleurs[4 * i + 3] * 255), false);
	}
}

void Affichage::afficherSommets(size_t sommets, GLfloat const *pos, GLubyte const *couleurs, GLfloat const *normales, GLuint const *index) {
	Affichage::changerTexture(Ecran::vide().tex());

	
	glEnableVertexAttribArray(Shader::shaderActuel().vertCoord());
	glVertexAttribPointer(Shader::shaderActuel().vertCoord(), 3, GL_FLOAT, GL_FALSE, 0, pos);
	if(couleurs) {
		glEnableVertexAttribArray(Shader::shaderActuel().coul());
		glVertexAttribPointer(Shader::shaderActuel().coul(), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, couleurs);
	}
	if(normales) {
		glEnableVertexAttribArray(Shader::shaderActuel().normales());
		glVertexAttribPointer(Shader::shaderActuel().normales(), 3, GL_FLOAT, GL_TRUE, 0, normales);
	}
	
	if(!index) {
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(sommets));
	}
	else {
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sommets), GL_UNSIGNED_INT, index);
	}

	glDisableVertexAttribArray(Shader::shaderActuel().vertCoord());
	glDisableVertexAttribArray(Shader::shaderActuel().coul());
	
#ifdef STAT
	nbVert += sommets;
#endif
}

void Affichage::afficherSommetsVBO(size_t sommets, GLuint vbo, size_t posVert, size_t posCouleurs, size_t posNormales, GLuint const *index) {
	Affichage::changerTexture(Ecran::vide().tex());
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(Shader::shaderActuel().coul());
	glVertexAttribPointer(Shader::shaderActuel().vertCoord(), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(posVert));

	if(posCouleurs != aucunePosition) {
		glEnableVertexAttribArray(Shader::shaderActuel().vertCoord());
		glVertexAttribPointer(Shader::shaderActuel().coul(), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, reinterpret_cast<void *>(posCouleurs));
	}
	if(posNormales != aucunePosition) {
		glEnableVertexAttribArray(Shader::shaderActuel().normales());
		glVertexAttribPointer(Shader::shaderActuel().normales(), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(posNormales));
	}
	
	if(!index) {
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(sommets));
	}
	else {
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sommets), GL_UNSIGNED_INT, index);
	}

	glDisableVertexAttribArray(Shader::shaderActuel().vertCoord());
	glDisableVertexAttribArray(Shader::shaderActuel().coul());
	glDisableVertexAttribArray(Shader::shaderActuel().normales());
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef STAT
	nbVert += sommets;
#endif
}

void Affichage::changerTexture(GLuint tex) {
	if(tex != _tex || tex == -1) {
		if(_nbSommets) {
#ifdef VBO
			if(_buf == 0) {
				glGenBuffers(1, &_buf);
				
				glBindBuffer(GL_ARRAY_BUFFER, _buf);
				glBufferData(GL_ARRAY_BUFFER, TAMPON_SOMMETS * (7 * sizeof(GLfloat) + 4 * sizeof(GLubyte)), 0, GL_STREAM_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				//GLuint vao;
				//glGenVertexArrays(1, &vao);
				//glBindVertexArray(vao);
			}
#endif
			glEnableVertexAttribArray(Shader::shaderActuel().vertCoord());
			glEnableVertexAttribArray(Shader::shaderActuel().texCoord());
			glEnableVertexAttribArray(Shader::shaderActuel().coul());
			
#ifdef VBO
			glBindBuffer(GL_ARRAY_BUFFER, _buf);
			glBufferSubData(GL_ARRAY_BUFFER, 0, _nbSommets * 3 * sizeof(GLfloat), &_vertCoords[0]);
			glBufferSubData(GL_ARRAY_BUFFER, _nbSommets * 3 * sizeof(GLfloat), _nbSommets * 4 * sizeof(GLfloat), &_texCoords[0]);
			glBufferSubData(GL_ARRAY_BUFFER, _nbSommets * 7 * sizeof(GLfloat), _nbSommets * 4 * sizeof(GLubyte), &_couleurs[0]);
			
			glVertexAttribPointer(Shader::shaderActuel().vertCoord(), 3, GL_FLOAT, GL_FALSE, 0, 0);
			glVertexAttribPointer(Shader::shaderActuel().texCoord(), 2, GL_FLOAT, GL_FALSE, 0, (void *)(_nbSommets * 3 * sizeof(GLfloat)));
			glVertexAttribPointer(Shader::shaderActuel().coul(), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)(_nbSommets * 7 * sizeof(GLfloat)));
			
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_nbSommets));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
			glVertexAttribPointer(Shader::shaderActuel().vertCoord(), 3, GL_FLOAT, GL_FALSE, 0, &_vertCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().texCoord(), 2, GL_FLOAT, GL_FALSE, 0, &_texCoords[0]);
			glVertexAttribPointer(Shader::shaderActuel().coul(), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &_couleurs[0]);
			
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_nbSommets));
#endif
			glDisableVertexAttribArray(Shader::shaderActuel().vertCoord());
			glDisableVertexAttribArray(Shader::shaderActuel().texCoord());
			glDisableVertexAttribArray(Shader::shaderActuel().coul());
			
#ifdef STAT
			static int nb1 = 0;
			static float nb2 = 0;
			static horloge_t h = horloge();
			++nb1;
			nb2 += _nbSommets;
			float H = horloge();
			if(H - h >= 1.0f) {
				std::cout << nbVert / (H - h) << " vtx/s" << std::endl;
				std::cout << nb1 / (H - h) << " ch/s (" << nb2 / nb1 << " vtx/ch, " << nb1 / Ecran::frequence() << " ch/img)" << std::endl;
				
				nbVert = 0;
				
				h = H;
				nb1 = 0;
				nb2 = 0;
			}
#endif
			
			_nbSommets = 0;
		}
		
		if(tex != -1) {
			glBindTexture(GL_TEXTURE_2D, tex);
		}
		_tex = tex;
	}
}

Couleur::composante_t const &Affichage::opacite() {
	return _opacite.top();
}

Couleur const &Affichage::teinte() {
	return _teinte;
}

void Affichage::definirTeinte(const Couleur &c) {
	_teinte = c;
}

void Affichage::ajouterOpacite(Couleur::composante_t o) {
	_opacite.push(o * Affichage::opacite() / 255);
}

void Affichage::supprimerOpacite() {
	_opacite.pop();
}

void Affichage::afficherRectangle(Rectangle const &r, Couleur const &c) {
	Affichage::afficherQuadrilatere(glm::vec2(r.gauche, r.haut), glm::vec2(r.gauche, r.haut + r.hauteur), glm::vec2(r.gauche + r.largeur, r.haut + r.hauteur), glm::vec2(r.gauche + r.largeur, r.haut), c);
}

void Affichage::afficherLigne(glm::vec2 const &depart, glm::vec2 const &arrivee, Couleur const &c, dimension_t epaisseur) {
	if(vecteurNul(depart - arrivee))
		return;
	glm::vec2 normale = glm::normalize(depart - arrivee) * epaisseur;
	std::swap(normale.x, normale.y);
	normale.y *= -1;
	
	Affichage::afficherQuadrilatere(depart - normale, depart + normale, arrivee + normale, arrivee - normale, c);
}

void Affichage::afficherQuadrilatere(glm::vec2 const &p1, glm::vec2 const &p2, glm::vec2 const &p3, glm::vec2 const &p4, Couleur const &c) {
	glm::vec2 pos(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
	pos.x = std::min(p3.x, pos.x);
	pos.x = std::min(p4.x, pos.x);
	pos.y = std::min(p3.y, pos.y);
	pos.y = std::min(p4.y, pos.y);
	
	Affichage::changerTexture(Ecran::vide().tex());
	
	Affichage::ajouterSommet(p1, glm::vec2(0, 0), c, true);
	Affichage::ajouterSommet(p2, glm::vec2(1, 0), c, true);
	Affichage::ajouterSommet(p4, glm::vec2(0, 1), c, true);
	
	Affichage::ajouterSommet(p4, glm::vec2(0, 1), c, true);
	Affichage::ajouterSommet(p2, glm::vec2(1, 0), c, true);
	Affichage::ajouterSommet(p3, glm::vec2(1, 1), c, true);
}

void Affichage::afficherTriangle(glm::vec2 const &p1, glm::vec2 const &p2, glm::vec2 const &p3, Couleur const &c) {
	glm::vec2 pos = glm::vec2(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
	pos.x = std::min(p3.x, pos.x);
	pos.y = std::min(p3.y, pos.y);
	
	Affichage::changerTexture(Ecran::vide().tex());
	
	Affichage::ajouterSommet(p1, glm::vec2(0, 0), c, true);
	Affichage::ajouterSommet(p2, glm::vec2(1, 0), c, true);
	Affichage::ajouterSommet(p3, glm::vec2(1, 1), c, true);
}
