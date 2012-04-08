/*
 *  Texte.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 12/04/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#include "Texte.h"
#include "Image.h"
#include "Constantes.h"
#include <string.h>

static Unichar::unichar *caracteres = 0;
static size_t nbCaracteres = 0;
static void initialiserCaracteres();

static Image *policeDeco = 0;
static unsigned int const hauteurDeco = 80;
static int const largeursDeco[][3] = {{0, 32, 0}, {33, 31, 0}, {65, 32, 0}, {98, 31, 0}, {130, 32, 0}, {163, 32, 0}, {196, 31, 0}, {228, 32, 0}, {261, 31, 0}, {293, 31, 0}, {325, 55, 0}, {381, 51, 0}, {433, 51, 0}, {485, 55, 0}, {541, 47, 0}, {589, 43, 0}, {633, 55, 0}, {689, 54, 0}, {744, 27, 0}, {772, 31, 0}, {804, 56, 0}, {861, 47, 0}, {909, 76, 0}, {0, 50, 80}, {51, 56, 80}, {108, 43, 80}, {152, 56, 80}, {209, 51, 80}, {261, 47, 80}, {309, 47, 80}, {357, 56, 80}, {414, 56, 80}, {471, 76, 80}, {548, 55, 80}, {604, 56, 80}, {661, 47, 80}, {709, 40, 80}, {750, 37, 80}, {788, 37, 80}, {826, 40, 80}, {867, 34, 80}, {902, 31, 80}, {934, 39, 80}, {0, 40, 160}, {41, 20, 160}, {62, 23, 160}, {86, 40, 160}, {127, 34, 160}, {162, 53, 160}, {216, 36, 160}, {253, 40, 160}, {294, 31, 160}, {326, 40, 160}, {367, 37, 160}, {405, 34, 160}, {440, 34, 160}, {475, 40, 160}, {516, 40, 160}, {557, 53, 160}, {611, 40, 160}, {652, 41, 160}, {694, 34, 160}, {729, 31, 160}, {761, 40, 160}, {802, 40, 160}, {843, 34, 160}, {878, 34, 160}, {913, 34, 160}, {948, 34, 160}, {0, 20, 240}, {21, 20, 240}, {42, 40, 240}, {83, 40, 240}, {124, 40, 240}, {165, 37, 240}, {203, 47, 240}, {251, 47, 240}, {299, 55, 240}, {355, 47, 240}, {403, 55, 240}, {459, 47, 240}, {507, 27, 240}, {535, 28, 240}, {564, 56, 240}, {621, 56, 240}, {678, 56, 240}, {735, 51, 240}, {787, 27, 240}, {815, 27, 240}, {843, 27, 240}, {871, 39, 240}, {911, 27, 240}, {939, 43, 240}, {0, 27, 320}, {28, 27, 320}, {56, 27, 320}, {84, 31, 320}, {116, 30, 320}, {147, 30, 320}, {178, 30, 320}, {209, 28, 320}, {238, 28, 320}, {267, 61, 320}, {329, 28, 320}, {358, 48, 320}, {407, 26, 320}, {434, 32, 320}, {467, 31, 320}, {499, 55, 320}, {555, 42, 320}, {598, 41, 320}, {640, 31, 320}, {672, -1, 320}, {712, 31, 320}, {744, 44, 320}, {789, 36, 320}, {826, 37, 320}, {864, 38, 320}, {903, 27, 320}, {931, 28, 320}, {0, 38, 400}, {39, 48, 400}, {88, 48, 400}, {137, -1, 400}, {177, 40, 400}, {218, 56, 400}, {275, -1, 400}, {315, -1, 400}, {355, 39, 400}, {395, -1, 400}, {435, 31, 400}, {467, -1, 400}, {507, 30, 400}, {538, 39, 400}, {578, -1, 400}, {618, -1, 400}, {658, 56, 400}, {715, 46, 400}, {762, 56, 400}, {819, 68, 400}, {888, 56, 400}, {0, 55, 480}, {56, 67, 480}, {124, 56, 480}, {181, 39, 480}, {221, 56, 480}, {278, 55, 480}, {334, 56, 480}};
static float const tailleRenduDeco = 72;

static Image *policeNormale = 0;
static unsigned int const hauteurNormal = 20;
static int const largeursNormal[][3] = {{0, 12, 0}, {13, 12, 0}, {26, 12, 0}, {39, 12, 0}, {52, 12, 0}, {65, 12, 0}, {78, 12, 0}, {91, 12, 0}, {104, 12, 0}, {117, 12, 0}, {130, 13, 0}, {144, 12, 0}, {157, 13, 0}, {171, 14, 0}, {186, 11, 0}, {198, 11, 0}, {210, 14, 0}, {225, 14, 0}, {240, 7, 0}, {248, 9, 0}, {258, 12, 0}, {271, 11, 0}, {283, 17, 0}, {301, 14, 0}, {316, 15, 0}, {332, 12, 0}, {345, 15, 0}, {361, 12, 0}, {374, 11, 0}, {386, 11, 0}, {398, 14, 0}, {413, 12, 0}, {426, 18, 0}, {445, 13, 0}, {459, 12, 0}, {472, 13, 0}, {486, 11, 0}, {498, 13, 0}, {512, 11, 0}, {524, 13, 0}, {538, 12, 0}, {551, 8, 0}, {560, 13, 0}, {574, 13, 0}, {588, 7, 0}, {596, 7, 0}, {604, 11, 0}, {616, 7, 0}, {624, 18, 0}, {643, 13, 0}, {657, 12, 0}, {670, 13, 0}, {684, 13, 0}, {698, 8, 0}, {707, 10, 0}, {718, 9, 0}, {728, 12, 0}, {741, 11, 0}, {753, 16, 0}, {770, 11, 0}, {782, 11, 0}, {794, 10, 0}, {805, 7, 0}, {813, 11, 0}, {825, 11, 0}, {837, 12, 0}, {850, 12, 0}, {863, 12, 0}, {876, 12, 0}, {889, 7, 0}, {897, 7, 0}, {905, 12, 0}, {918, 12, 0}, {931, 12, 0}, {944, 11, 0}, {956, 11, 0}, {968, 11, 0}, {980, 13, 0}, {0, 11, 20}, {12, 13, 20}, {26, 11, 20}, {38, 7, 20}, {46, 7, 20}, {54, 15, 20}, {70, 15, 20}, {86, 14, 20}, {101, 13, 20}, {115, 8, 20}, {124, 8, 20}, {133, 6, 20}, {140, 9, 20}, {150, 6, 20}, {157, 10, 20}, {168, 6, 20}, {175, 6, 20}, {182, 6, 20}, {189, 9, 20}, {199, 14, 20}, {214, 14, 20}, {229, 8, 20}, {238, 14, 20}, {253, 14, 20}, {268, 16, 20}, {285, 11, 20}, {297, 13, 20}, {311, 7, 20}, {319, 19, 20}, {339, 9, 20}, {349, 19, 20}, {369, 14, 20}, {384, 10, 20}, {395, 12, 20}, {408, 12, 20}, {421, 14, 20}, {436, 12, 20}, {449, 16, 20}, {466, 8, 20}, {475, 8, 20}, {484, 8, 20}, {493, 8, 20}, {502, 19, 20}, {522, 16, 20}, {539, 18, 20}, {558, 19, 20}, {578, 12, 20}, {591, 15, 20}, {607, -1, 20}, {611, -1, 20}, {615, 12, 20}, {628, 19, 20}, {648, 19, 20}, {668, 19, 20}, {688, 19, 20}, {708, 8, 20}, {717, -1, 20}, {721, 19, 20}, {741, 15, 20}, {757, 19, 20}, {777, 12, 20}, {790, 18, 20}, {809, 12, 20}, {822, 13, 20}, {836, 16, 20}, {853, 12, 20}, {866, 11, 20}, {878, 14, 20}, {893, 13, 20}, {907, 15, 20}};
static float const tailleRenduNormal = 16;

static Image *policeGrande = 0;
static unsigned int const hauteurGrande = 49;
static int const largeursGrande[][3] = {{0, 25, 0}, {26, 25, 0}, {52, 25, 0}, {78, 25, 0}, {104, 25, 0}, {130, 25, 0}, {156, 25, 0}, {182, 25, 0}, {208, 25, 0}, {234, 25, 0}, {260, 29, 0}, {290, 26, 0}, {317, 28, 0}, {346, 32, 0}, {379, 24, 0}, {404, 24, 0}, {429, 31, 0}, {461, 31, 0}, {493, 13, 0}, {507, 18, 0}, {526, 26, 0}, {553, 23, 0}, {577, 38, 0}, {616, 31, 0}, {648, 33, 0}, {682, 25, 0}, {708, 33, 0}, {742, 26, 0}, {769, 24, 0}, {794, 24, 0}, {819, 31, 0}, {851, 26, 0}, {878, 40, 0}, {919, 27, 0}, {947, 26, 0}, {0, 27, 49}, {28, 23, 49}, {52, 27, 49}, {80, 23, 49}, {104, 27, 49}, {132, 25, 49}, {158, 15, 49}, {174, 27, 49}, {202, 27, 49}, {230, 12, 49}, {243, 13, 49}, {257, 22, 49}, {280, 12, 49}, {293, 39, 49}, {333, 27, 49}, {361, 27, 49}, {389, 27, 49}, {417, 27, 49}, {445, 16, 49}, {462, 20, 49}, {483, 17, 49}, {501, 27, 49}, {529, 23, 49}, {553, 35, 49}, {589, 22, 49}, {612, 23, 49}, {636, 21, 49}, {658, 13, 49}, {672, 23, 49}, {696, 23, 49}, {720, 25, 49}, {746, 25, 49}, {772, 25, 49}, {798, 25, 49}, {824, 12, 49}, {837, 12, 49}, {850, 27, 49}, {878, 27, 49}, {906, 27, 49}, {934, 23, 49}, {958, 24, 49}, {0, 24, 98}, {25, 29, 98}, {55, 24, 98}, {80, 29, 98}, {110, 24, 98}, {135, 13, 98}, {149, 13, 98}, {163, 33, 98}, {197, 33, 98}, {231, 31, 98}, {263, 28, 98}, {292, 15, 98}, {308, 15, 98}, {324, 11, 98}, {336, 17, 98}, {354, 11, 98}, {366, 20, 98}, {387, 11, 98}, {399, 11, 98}, {411, 11, 98}, {423, 19, 98}, {443, 30, 98}, {474, 30, 98}, {505, 16, 98}, {522, 30, 98}, {553, 30, 98}, {584, 35, 98}, {620, 24, 98}, {645, 28, 98}, {674, 12, 98}, {687, 43, 98}, {731, 18, 98}, {750, 43, 98}, {794, 30, 98}, {825, 21, 98}, {847, 25, 98}, {873, 25, 98}, {899, 30, 98}, {930, 26, 98}, {957, 36, 98}, {0, 15, 147}, {16, 15, 147}, {32, 15, 147}, {48, 15, 147}, {64, 43, 147}, {108, 36, 147}, {145, 41, 147}, {187, 43, 147}, {231, 27, 147}, {259, 33, 147}, {293, -1, 147}, {297, -1, 147}, {301, 26, 147}, {328, 43, 147}, {372, 43, 147}, {416, 43, 147}, {460, 43, 147}, {504, 14, 147}, {519, -1, 147}, {523, 43, 147}, {567, 33, 147}, {601, 43, 147}, {645, 25, 147}, {671, 41, 147}, {713, 26, 147}, {740, 29, 147}, {770, 36, 147}, {807, 26, 147}, {834, 23, 147}, {858, 31, 147}, {890, 29, 147}, {920, 33, 147}};
static float const tailleRenduGrande = 40;

Texte::Texte(Unichar const &txt, police_t police, taillePolice_t taille, Couleur const &coul) : _texte(txt), _police(police), _taille(taille), _couleur(coul), _dimensions() {
	_dimensions = Texte::dimensions(_texte, _police, _taille);
}

Texte::Texte(Texte const &t) : _texte(t._texte), _police(t._police), _taille(t._taille), _couleur(t._couleur), _dimensions(t._dimensions) { }

Texte &Texte::operator=(Texte const &t) {
	_texte = t._texte;
	_police = t._police;
	_taille = t._taille;
	_dimensions = t._dimensions;
	_couleur = t._couleur;
	
	return *this;
}

Texte::~Texte() {}

Texte &Texte::definir(Unichar const &txt) {
	_texte = txt;
	_dimensions = Texte::dimensions(_texte, _police, _taille);
	
	return *this;
}

Texte &Texte::definir(police_t police, taillePolice_t taille) {
	_police = police;
	_taille = taille;
	_dimensions = Texte::dimensions(_texte, _police, _taille);
	
	return 
	*this;
}

Texte &Texte::definir(Unichar const &txt, police_t police, taillePolice_t taille) {
	_texte = txt;
	_police = police;
	_taille = taille;
	_dimensions = Texte::dimensions(_texte, _police, _taille);
	
	return *this;
}

Texte &Texte::definir(Couleur const &coul) {
	_couleur = coul;
	
	return *this;
}

void Texte::afficher(Coordonnees const &pos) const {
	Texte::afficher(_texte, _police, _taille, _couleur, pos);
}

void Texte::afficher(Unichar const &txt, police_t police, taillePolice_t taille, Couleur const &coul, Coordonnees pos) {
	Couleur teinte = Image::teinte();
	Image::definirTeinte(coul);

	int hauteur;
	float tailleRendu;
	Image *image;
	switch(police) {
		case POLICE_DECO:
			hauteur = hauteurDeco;
			tailleRendu = tailleRenduDeco;
			if(policeDeco == 0) {
				policeDeco = new Image(Session::cheminRessources() + "policeDeco.png");
			}
			image = policeDeco;
			break;
		case POLICE_NORMALE:
			hauteur = hauteurNormal;
			tailleRendu = tailleRenduNormal;
			if(policeNormale == 0) {
				policeNormale = new Image(Session::cheminRessources() + "policeNormale.png");
			}
			image = policeNormale;
			break;
		case POLICE_GRANDE:
			hauteur = hauteurGrande;
			tailleRendu = tailleRenduGrande;
			if(policeGrande == 0) {
				policeGrande = new Image(Session::cheminRessources() + "policeGrande.png");
			}
			image = policeGrande;
			break;
	}

	float rapportTaille = taille / tailleRendu;
	image->redimensionner(rapportTaille);

	initialiserCaracteres();
	Coordonnees posInitiale = pos;
	
	long longueur = txt.size();
	for(long i = 0; i < longueur; ++i) {
		Unichar::unichar c = *(txt.unitxt() + i);

		if(c == '\n') {
			pos.x = posInitiale.x;
			pos.y += 4.0 / 4.0 * hauteur * rapportTaille;
		}
		else {
			uindex_t p = std::find(caracteres, caracteres + nbCaracteres, c) - caracteres;
			if(p == nbCaracteres)
				std::cerr << "Le caractère " << c << " n'est pas géré par le système de texte." << std::endl;

			int l;
			switch(police) {
				case POLICE_DECO:
					l = largeursDeco[p][1];
					break;
				case POLICE_NORMALE:
					l = largeursNormal[p][1];
					break;
				case POLICE_GRANDE:
					l = largeursGrande[p][1];
					break;
			}
			if(l == -1)
				std::cerr << "Le caractère " << c << " n'est pas contenu dans la police " << police << "." << std::endl;
			else {
				Rectangle filtre;
				switch(police) {
					case POLICE_DECO:
						filtre = Rectangle(largeursDeco[p][0], largeursDeco[p][2], l, hauteur);
						break;
					case POLICE_NORMALE:
						filtre = Rectangle(largeursNormal[p][0], largeursNormal[p][2], l, hauteur);
						break;
					case POLICE_GRANDE:
						filtre = Rectangle(largeursGrande[p][0], largeursGrande[p][2], l, hauteur);
						break;
				}
				image->afficher(pos, filtre);
				pos.x += filtre.largeur * rapportTaille;
			}
		}

	}
	
	Image::definirTeinte(teinte);
}

static void initialiserCaracteres() {
	if(caracteres)
		return;
	char const *t = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz âàêéèëîïôöùçÉÈÀÊÂËÎÏÔÖÙÇ()'\",?;.:/+=-<>@#&!°\\…~*$€^_%{}[]§æœπôÒ≤≈ß∞÷≠±•≥ΩÓ‰¥ŒŸÅÆŸåÛÁØ";
	Unichar texte(t);
	nbCaracteres = texte.size() - 1;
	caracteres = new Unichar::unichar[nbCaracteres + 1];
	std::copy(texte.unitxt(), texte.unitxt() + nbCaracteres + 1, caracteres);
}

Coordonnees Texte::dimensions(Unichar const &texte, police_t police, taillePolice_t taille) {
	return Texte::dimensions(texte, 0, texte.size(), police, taille);
}

Coordonnees Texte::dimensions(Unichar const &texte, uindex_t p, size_t n, police_t police, taillePolice_t taille) {
	Coordonnees retour;
	if(!n)
		return retour;
	assert(p + n <= texte.size());
	
	int const (*largeurs)[3] = 0;
	unsigned int hauteur = 0;
	float tailleRendu = 1;
	switch(police) {
		case POLICE_DECO:
			hauteur = hauteurDeco;
			tailleRendu = tailleRenduDeco;
			largeurs = largeursDeco;
			break;
		case POLICE_NORMALE:
			hauteur = hauteurNormal;
			tailleRendu = tailleRenduNormal;
			largeurs = largeursNormal;
			break;
		case POLICE_GRANDE:
			hauteur = hauteurGrande;
			tailleRendu = tailleRenduGrande;
			largeurs = largeursGrande;
			break;
	}
	initialiserCaracteres();
	
	double abscisse = 0;
	
	retour.y = hauteur * taille / tailleRendu;
	
	for(uindex_t i = p; i < n + p; ++i) {
		Unichar::unichar c = *(texte.unitxt() + i);
		
		if(c == '\n') {
			retour.x = std::max(abscisse, retour.x);
			retour.y += 5.0 / 4 * hauteur * taille / tailleRendu;
			abscisse = 0;
		}
		else {
			uindex_t p = std::find(caracteres, caracteres + nbCaracteres, c) - caracteres;
			if(p == nbCaracteres)
				std::cerr << "Le caractère " << c << " n'est pas géré par le système de texte." << std::endl;

			int l = largeurs[p][1];
			if(l == -1)
				std::cerr << "Le caractère " << c << " n'est pas contenu dans la police " << police << "." << std::endl;
			else
				abscisse += l;
		}
	}
	
	retour.x = (std::max(abscisse, retour.x)) * taille / tailleRendu;
	
	return retour;	
}

dimension_t Texte::hauteur(police_t police, taillePolice_t taille) {
	unsigned int h;
	float tailleRendu;
	if(police == POLICE_DECO) {
		h = hauteurDeco;
		tailleRendu = tailleRenduDeco;
	}
	else if(police == POLICE_NORMALE) {
		h = hauteurNormal;
		tailleRendu = tailleRenduNormal;
	}
	else if(police == POLICE_GRANDE) {
		h = hauteurGrande;
		tailleRendu = tailleRenduGrande;
	}
	return h * taille / tailleRendu;
}

void Texte::nettoyer() {
	delete policeDeco;
	delete policeNormale;
	delete policeGrande;
	delete[] caracteres;
}
