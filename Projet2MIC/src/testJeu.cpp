//
//  test.cpp
//  Projet2MIC
//
//  Created by Rémi on 31/07/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "testJeu.h"
#include <cmath>
#include "Affichage.h"

#define ARRAY_COUNT(X) (sizeof(X) / sizeof((X)[0]))

void test() {
	VueTest vue;
	Session::ajouterVueFenetre(&vue);
}

VueTest::VueTest() : VueInterface(Ecran::ecran()), _img1(Session::cheminRessources() + "hm.png"), _shader(Session::cheminRessources() + "test.vert", Session::cheminRessources() + "test.frag") {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);
	
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _img1.largeur() * _img1.hauteur() * (3 * 6 * sizeof(GLfloat) + 4 * 6 * sizeof(GLubyte) + 3 * 6 * sizeof(GLfloat)), nullptr, GL_STATIC_DRAW);

	std::vector<GLuint>indx((_img1.largeur() - 1) * (_img1.hauteur() - 1) * 3 * 2);
	std::vector<GLfloat> vert(_img1.largeur() * _img1.hauteur() * 3);
	std::vector<GLfloat> norm(_img1.largeur() * _img1.hauteur() * 3);
	std::vector<GLubyte> coul(_img1.largeur() * _img1.hauteur() * 4);

	int i = 0, numIndx = 0;
	for(index_t xx = 0; xx < _img1.largeur(); ++xx) {
		for(index_t yy = 0; yy < _img1.hauteur(); ++yy) {
			vert[3 * i] = xx * 0.05;
			vert[3 * i + 1] = _img1.pixels()[(yy * _img1.largeur() + xx) * 4] / 255.0 * 2;
			vert[3 * i + 2] = yy * 0.05;
			
			coul[4 * i] = 255;
			coul[4 * i + 1] = ((xx / 100) * 100) * 255 / _img1.largeur();
			coul[4 * i + 2] = 0;
			coul[4 * i + 3] = 255;
			
			if(xx < _img1.largeur() - 1 && yy < _img1.hauteur() - 1) {
				indx[numIndx++] = i;
				indx[numIndx++] = i + 1;
				indx[numIndx++] = i + _img1.hauteur();

				indx[numIndx++] = i + _img1.hauteur() + 1;
				indx[numIndx++] = i + _img1.hauteur();
				indx[numIndx++] = i + 1;
			}
			++i;
		}
	}
	
	for(index_t i = 0; i < indx.size() / 3; ++i) {
		glm::vec3 o(vert[3 * indx[3 * i + 0]], vert[3 * indx[3 * i + 0] + 1], vert[3 * indx[3 * i + 0] + 2]);
		glm::vec3 p1(vert[3 * indx[3 * i + 1]], vert[3 * indx[3 * i + 1] + 1], vert[3 * indx[3 * i + 1] + 2]);
		glm::vec3 p2(vert[3 * indx[3 * i + 2]], vert[3 * indx[3 * i + 2] + 1], vert[3 * indx[3 * i + 2] + 2]);
		glm::vec3 v1(p1 - o), v2(p2 - o);
		glm::vec3 pV = glm::cross(v1, v2);
		
		norm[3 * indx[3 * i + 0] + 0] += pV.x;
		norm[3 * indx[3 * i + 1] + 0] += pV.x;
		norm[3 * indx[3 * i + 2] + 0] += pV.x;
		
		norm[3 * indx[3 * i + 0] + 1] += pV.y;
		norm[3 * indx[3 * i + 1] + 1] += pV.y;
		norm[3 * indx[3 * i + 2] + 1] += pV.y;
		
		norm[3 * indx[3 * i + 0] + 2] += pV.z;
		norm[3 * indx[3 * i + 1] + 2] += pV.z;
		norm[3 * indx[3 * i + 2] + 2] += pV.z;
	}
	
	_nbSommets = indx.size();
	
	_vert.resize(_nbSommets * 3);
	_coul.resize(_nbSommets * 4);
	_norm.resize(_nbSommets * 3);
	for(index_t i = 0; i < indx.size(); ++i) {
		_vert[3 * i + 0] = vert[3 * indx[i] + 0];
		_vert[3 * i + 1] = vert[3 * indx[i] + 1];
		_vert[3 * i + 2] = vert[3 * indx[i] + 2];

		_coul[4 * i + 0] = coul[4 * indx[i] + 0];
		_coul[4 * i + 1] = coul[4 * indx[i] + 1];
		_coul[4 * i + 2] = coul[4 * indx[i] + 2];
		_coul[4 * i + 3] = coul[4 * indx[i] + 3];

		_norm[3 * i + 0] = norm[3 * indx[i] + 0];
		_norm[3 * i + 1] = norm[3 * indx[i] + 1];
		_norm[3 * i + 2] = norm[3 * indx[i] + 2];
	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, _nbSommets * 3 * sizeof(GLfloat), &_vert[0]);
	glBufferSubData(GL_ARRAY_BUFFER, _nbSommets * 3 * sizeof(GLfloat), _nbSommets * 3 * sizeof(GLfloat), &_norm[0]);
	glBufferSubData(GL_ARRAY_BUFFER, _nbSommets * (3 + 3) * sizeof(GLfloat), _nbSommets * 4 * sizeof(GLubyte), &_coul[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
	_vert.clear();
	_coul.clear();
	_norm.clear();
	_vert.shrink_to_fit();
	_coul.shrink_to_fit();
	_norm.shrink_to_fit();
}

VueTest::~VueTest() {
	glDeleteBuffers(1, &_vbo);
}


void VueTest::preparationDessin() {
	
}

glm::vec3 sphCam(40, -45, 0);
glm::vec3 posLumiere(10, 10, 10);

glm::vec3 cibleCamera(10, 0, 10);

void VueTest::dessiner() {
	glEnable(GL_DEPTH_TEST);
	
	posLumiere.x = std::cos(horloge()) * 10 + 10;
	posLumiere.z = std::sin(horloge()) * 10 + 10;
	
	_shader.activer();
	Ecran::perspective(45, 1.0, 60);
	
	Ecran::ajouterTransformation(cameraSphere(sphCam, cibleCamera, glm::vec3(0, 1, 0)));
	
	glm::vec3 lum = glm::vec3(Ecran::modeleVue() * glm::vec4(posLumiere, 1.0));
	glm::mat4 inv = glm::inverse(Ecran::projection());
	
	Shader::shaderActuel().definirParametre("invProjection", inv);
	Shader::shaderActuel().definirParametre("lightAttenuation", 0.02);
	Shader::shaderActuel().definirParametre("positionLumiere", lum);
	Shader::shaderActuel().definirParametre("lumiere", 0.8f * glm::vec3(1.0, 1.0, 1.0));
	Shader::shaderActuel().definirParametre("lumiereAmbiante", glm::vec3(0.2, 0.2, 0.2));

	Affichage::afficherSommetsVBO(_nbSommets, _vbo, 0, _nbSommets * 6 * sizeof(GLfloat), _nbSommets * 3 * sizeof(GLfloat));

	Shader::desactiver();
}

void VueTest::gestionClavier() {
	bool continuer = true;
	static bool clamp = true;
	if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER)) {
		continuer = false;
	}
	float decCible = 0.05;
	float dec = 1;
	float dec1 = 1;
	if(Session::evenement(Session::T_GAUCHE)) {
		sphCam.z -= dec;
	}
	if(Session::evenement(Session::T_HAUT)) {
		sphCam.y += dec;
	}
	if(Session::evenement(Session::T_BAS)) {
		sphCam.y -= dec;
	}
	if(Session::evenement(Session::T_DROITE)) {
		sphCam.z += dec;
	}
	if(Session::evenement(Session::T_c)) {
		sphCam.x += dec1;
	}
	if(Session::evenement(Session::T_x)) {
		sphCam.x -= dec1;
	}
	
	if(Session::evenement(Session::T_z)) {
		cibleCamera.x += decCible;
	}
	if(Session::evenement(Session::T_s)) {
		cibleCamera.x -= decCible;
	}
	if(Session::evenement(Session::T_q)) {
		cibleCamera.z -= decCible;
	}
	if(Session::evenement(Session::T_d)) {
		cibleCamera.z += decCible;
	}
	if(Session::evenement(Session::T_o)) {
		posLumiere.y += 0.2;
	}
	if(Session::evenement(Session::T_l)) {
		posLumiere.y -= 0.2;
	}
	if(Session::evenement(Session::T_ESPACE)) {
		if(clamp)
			glDisable(GL_DEPTH_CLAMP);
		else
			glEnable(GL_DEPTH_CLAMP);
		clamp = !clamp;
		
		Session::reinitialiser(Session::T_ESPACE);
	}
	
	sphCam.y = ::clamp(sphCam.y, -85, 0);
	sphCam.z = std::fmod(sphCam.z, 360);
	if(!continuer) {
		Session::supprimerVueFenetre();
	}
}
