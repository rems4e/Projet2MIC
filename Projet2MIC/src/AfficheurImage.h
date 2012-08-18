/*
 *  AfficheurImage.h
 *  Jeu C++
 *
 *  Created by Rémi on 18/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef AFFICHEUR_IMAGE_H
#define AFFICHEUR_IMAGE_H

#include "VueInterface.h"
#include "Image.h"

typedef VueInterface AfficheurCouleur;

class AfficheurImage : public VueInterface {
public:
	inline AfficheurImage(Rectangle const &cadre, Image const &img = Image(), Couleur const &couleurFond = Couleur::gris, Couleur const &teinte = Couleur::blanc) : VueInterface(cadre, couleurFond), _image(img), _teinte(teinte), _filtre() {
		
	}
		
	inline void definirImage(Image const &img) {
		_image = img;
	}
	inline Image const &image() const {
		return _image;
	}
	
	inline void definirTeinte(Couleur const &t) {
		_teinte = t;
	}
	inline Couleur const &teinte() const {
		return _teinte;
	}
		
	inline void definirFiltre(Rectangle const &c) {
		_filtre = c;
	}
	inline Rectangle const &filtre() const {
		return _filtre;
	}

protected:
	Image _image;
	Couleur _teinte;
	Rectangle _filtre;
	void dessiner();
};

#endif
