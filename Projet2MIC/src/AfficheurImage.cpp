/*
 *  AfficheurImage.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 01/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "AfficheurImage.h"
#include "Affichage.h"

void AfficheurImage::dessiner() {
	VueInterface::dessiner();
	
	Couleur aT = Affichage::teinte();
	Affichage::definirTeinte(_teinte);
	if(_filtre.largeur == 0)
		_filtre = Rectangle(glm::vec2(0), glm::vec2(_image.dimensionsReelles()));
	_image.redimensionner(static_cast<float>(std::min(this->cadre().largeur / _filtre.largeur, this->cadre().hauteur / _filtre.hauteur)));
	_image.afficher(glm::vec2(0), _filtre);
	Affichage::definirTeinte(aT);
}
