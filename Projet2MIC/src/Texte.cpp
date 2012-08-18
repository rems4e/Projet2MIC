//
//  Texte.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Texte.h"
#include "Image.h"
#include "Affichage.h"
#include "Constantes.h"
#include <cstring>
#include <cstdlib>

static Unichar::unichar *caracteres = 0;
static size_t nbCaracteres = 0;
static void initialiserCaracteres();

char const caracteresSupportes[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz âàêéèëîïôöùçÉÈÀÊÂËÎÏÔÖÙÇ()'\",?;.:/+=-<>@#&!°\\…~*$€^_%{}[]§æœπôÒ≤≈ß∞÷≠±•≥ΩÓ‰¥ŒŸÅÆŸåÛÁØ";

static Image *polices[nbPolices] = {0};
static char const *(chemins[3]) = {"policeNormale.png", "policeDeco.png", "policeGrande.png"};

static unsigned int const hauteurs[nbPolices] = {20, 107, 49};
static int const largeurs[nbPolices][sizeof(caracteresSupportes)][3] = {
	{{0, 12, 0}, {13, 12, 0}, {26, 12, 0}, {39, 12, 0}, {52, 12, 0}, {65, 12, 0}, {78, 12, 0}, {91, 12, 0}, {104, 12, 0}, {117, 12, 0}, {130, 13, 0}, {144, 12, 0}, {157, 13, 0}, {171, 14, 0}, {186, 11, 0}, {198, 11, 0}, {210, 14, 0}, {225, 14, 0}, {240, 7, 0}, {248, 9, 0}, {258, 12, 0}, {271, 11, 0}, {283, 17, 0}, {301, 14, 0}, {316, 15, 0}, {332, 12, 0}, {345, 15, 0}, {361, 12, 0}, {374, 11, 0}, {386, 11, 0}, {398, 14, 0}, {413, 12, 0}, {426, 18, 0}, {445, 13, 0}, {459, 12, 0}, {472, 13, 0}, {486, 11, 0}, {498, 13, 0}, {512, 11, 0}, {524, 13, 0}, {538, 12, 0}, {551, 8, 0}, {560, 13, 0}, {574, 13, 0}, {588, 7, 0}, {596, 7, 0}, {604, 11, 0}, {616, 7, 0}, {624, 18, 0}, {643, 13, 0}, {657, 12, 0}, {670, 13, 0}, {684, 13, 0}, {698, 8, 0}, {707, 10, 0}, {718, 9, 0}, {728, 12, 0}, {741, 11, 0}, {753, 16, 0}, {770, 11, 0}, {782, 11, 0}, {794, 10, 0}, {805, 7, 0}, {813, 11, 0}, {825, 11, 0}, {837, 12, 0}, {850, 12, 0}, {863, 12, 0}, {876, 12, 0}, {889, 7, 0}, {897, 7, 0}, {905, 12, 0}, {918, 12, 0}, {931, 12, 0}, {944, 11, 0}, {956, 11, 0}, {968, 11, 0}, {980, 13, 0}, {0, 11, 20}, {12, 13, 20}, {26, 11, 20}, {38, 7, 20}, {46, 7, 20}, {54, 15, 20}, {70, 15, 20}, {86, 14, 20}, {101, 13, 20}, {115, 8, 20}, {124, 8, 20}, {133, 6, 20}, {140, 9, 20}, {150, 6, 20}, {157, 10, 20}, {168, 6, 20}, {175, 6, 20}, {182, 6, 20}, {189, 9, 20}, {199, 14, 20}, {214, 14, 20}, {229, 8, 20}, {238, 14, 20}, {253, 14, 20}, {268, 16, 20}, {285, 11, 20}, {297, 13, 20}, {311, 7, 20}, {319, 19, 20}, {339, 9, 20}, {349, 19, 20}, {369, 14, 20}, {384, 10, 20}, {395, 12, 20}, {408, 12, 20}, {421, 14, 20}, {436, 12, 20}, {449, 16, 20}, {466, 8, 20}, {475, 8, 20}, {484, 8, 20}, {493, 8, 20}, {502, 19, 20}, {522, 16, 20}, {539, 18, 20}, {558, 19, 20}, {578, 12, 20}, {591, 15, 20}, {607, -1, 20}, {611, -1, 20}, {615, 12, 20}, {628, 19, 20}, {648, 19, 20}, {668, 19, 20}, {688, 19, 20}, {708, 8, 20}, {717, -1, 20}, {721, 19, 20}, {741, 15, 20}, {757, 19, 20}, {777, 12, 20}, {790, 18, 20}, {809, 12, 20}, {822, 13, 20}, {836, 16, 20}, {853, 12, 20}, {866, 11, 20}, {878, 14, 20}, {893, 13, 20}, {907, 15, 20}},
	{{0, 27, 0}, {28, 19, 0}, {48, 38, 0}, {87, 44, 0}, {132, 39, 0}, {172, 39, 0}, {212, 42, 0}, {255, 36, 0}, {292, 46, 0}, {339, 43, 0}, {383, 46, 0}, {430, 46, 0}, {477, 41, 0}, {519, 37, 0}, {557, 53, 0}, {611, 41, 0}, {653, 36, 0}, {690, 41, 0}, {732, 19, 0}, {752, 40, 0}, {793, 53, 0}, {847, 45, 0}, {893, 62, 0}, {0, 47, 107}, {48, 34, 107}, {83, 36, 107}, {120, 42, 107}, {163, 43, 107}, {207, 50, 107}, {258, 46, 107}, {305, 41, 107}, {347, 49, 107}, {397, 62, 107}, {460, 59, 107}, {520, 41, 107}, {562, 43, 107}, {606, 37, 107}, {644, 38, 107}, {683, 35, 107}, {719, 38, 107}, {758, 43, 107}, {802, 37, 107}, {840, 37, 107}, {878, 36, 107}, {915, 19, 107}, {935, 38, 107}, {0, 43, 214}, {44, 34, 214}, {79, 59, 214}, {139, 41, 214}, {181, 32, 214}, {214, 38, 214}, {253, 44, 214}, {298, 43, 214}, {342, 48, 214}, {391, 51, 214}, {443, 42, 214}, {486, 48, 214}, {535, 55, 214}, {591, 53, 214}, {645, 42, 214}, {688, 49, 214}, {738, 22, 214}, {761, 37, 214}, {799, 37, 214}, {837, 43, 214}, {881, 43, 214}, {925, 43, 214}, {0, 43, 321}, {44, 19, 321}, {64, 19, 321}, {84, 32, 321}, {117, 32, 321}, {150, 42, 321}, {193, 35, 321}, {229, 53, 321}, {283, 53, 321}, {337, 46, 321}, {384, 53, 321}, {438, 46, 321}, {485, 53, 321}, {539, 19, 321}, {559, 20, 321}, {580, 34, 321}, {615, 34, 321}, {650, 41, 321}, {692, 41, 321}, {734, 27, 321}, {762, 28, 321}, {791, 18, 321}, {810, 35, 321}, {846, 19, 321}, {866, 47, 321}, {914, 19, 321}, {934, 18, 321}, {953, 18, 321}, {0, 45, 428}, {46, 34, 428}, {81, 32, 428}, {114, 32, 428}, {147, 36, 428}, {184, 37, 428}, {222, 43, 428}, {266, 42, 428}, {309, 46, 428}, {356, 22, 428}, {379, 22, 428}, {402, 45, 428}, {448, 52, 428}, {501, 37, 428}, {539, 33, 428}, {573, 39, 428}, {613, 39, 428}, {653, 36, 428}, {690, 29, 428}, {720, 49, 428}, {770, 27, 428}, {798, 28, 428}, {827, 24, 428}, {852, 24, 428}, {877, 57, 428}, {935, 59, 428}, {0, 56, 535}, {57, 46, 535}, {104, 32, 535}, {137, 34, 535}, {172, 36, 535}, {209, 36, 535}, {246, 42, 535}, {289, 56, 535}, {346, 32, 535}, {379, 30, 535}, {410, 34, 535}, {445, 26, 535}, {472, 36, 535}, {509, 55, 535}, {565, 34, 535}, {600, 67, 535}, {668, 46, 535}, {715, 67, 535}, {783, 41, 535}, {825, 46, 535}, {872, 72, 535}, {945, 41, 535}, {0, 36, 642}, {37, 41, 642}, {79, 46, 642}, {126, 39, 642}},
	{{0, 25, 0}, {26, 25, 0}, {52, 25, 0}, {78, 25, 0}, {104, 25, 0}, {130, 25, 0}, {156, 25, 0}, {182, 25, 0}, {208, 25, 0}, {234, 25, 0}, {260, 29, 0}, {290, 26, 0}, {317, 28, 0}, {346, 32, 0}, {379, 24, 0}, {404, 24, 0}, {429, 31, 0}, {461, 31, 0}, {493, 13, 0}, {507, 18, 0}, {526, 26, 0}, {553, 23, 0}, {577, 38, 0}, {616, 31, 0}, {648, 33, 0}, {682, 25, 0}, {708, 33, 0}, {742, 26, 0}, {769, 24, 0}, {794, 24, 0}, {819, 31, 0}, {851, 26, 0}, {878, 40, 0}, {919, 27, 0}, {947, 26, 0}, {0, 27, 49}, {28, 23, 49}, {52, 27, 49}, {80, 23, 49}, {104, 27, 49}, {132, 25, 49}, {158, 15, 49}, {174, 27, 49}, {202, 27, 49}, {230, 12, 49}, {243, 13, 49}, {257, 22, 49}, {280, 12, 49}, {293, 39, 49}, {333, 27, 49}, {361, 27, 49}, {389, 27, 49}, {417, 27, 49}, {445, 16, 49}, {462, 20, 49}, {483, 17, 49}, {501, 27, 49}, {529, 23, 49}, {553, 35, 49}, {589, 22, 49}, {612, 23, 49}, {636, 21, 49}, {658, 13, 49}, {672, 23, 49}, {696, 23, 49}, {720, 25, 49}, {746, 25, 49}, {772, 25, 49}, {798, 25, 49}, {824, 12, 49}, {837, 12, 49}, {850, 27, 49}, {878, 27, 49}, {906, 27, 49}, {934, 23, 49}, {958, 24, 49}, {0, 24, 98}, {25, 29, 98}, {55, 24, 98}, {80, 29, 98}, {110, 24, 98}, {135, 13, 98}, {149, 13, 98}, {163, 33, 98}, {197, 33, 98}, {231, 31, 98}, {263, 28, 98}, {292, 15, 98}, {308, 15, 98}, {324, 11, 98}, {336, 17, 98}, {354, 11, 98}, {366, 20, 98}, {387, 11, 98}, {399, 11, 98}, {411, 11, 98}, {423, 19, 98}, {443, 30, 98}, {474, 30, 98}, {505, 16, 98}, {522, 30, 98}, {553, 30, 98}, {584, 35, 98}, {620, 24, 98}, {645, 28, 98}, {674, 12, 98}, {687, 43, 98}, {731, 18, 98}, {750, 43, 98}, {794, 30, 98}, {825, 21, 98}, {847, 25, 98}, {873, 25, 98}, {899, 30, 98}, {930, 26, 98}, {957, 36, 98}, {0, 15, 147}, {16, 15, 147}, {32, 15, 147}, {48, 15, 147}, {64, 43, 147}, {108, 36, 147}, {145, 41, 147}, {187, 43, 147}, {231, 27, 147}, {259, 33, 147}, {293, -1, 147}, {297, -1, 147}, {301, 26, 147}, {328, 43, 147}, {372, 43, 147}, {416, 43, 147}, {460, 43, 147}, {504, 14, 147}, {519, -1, 147}, {523, 43, 147}, {567, 33, 147}, {601, 43, 147}, {645, 25, 147}, {671, 41, 147}, {713, 26, 147}, {740, 29, 147}, {770, 36, 147}, {807, 26, 147}, {834, 23, 147}, {858, 31, 147}, {890, 29, 147}, {920, 33, 147}}
};
static float const taillesRendu[nbPolices] = {16, 64, 40};

police_t &operator++(police_t &p) {
	p = static_cast<police_t>(static_cast<int>(p + 1));
	return p;
}


Texte::Texte(Unichar const &txt, police_t police, taillePolice_t taille, Couleur const &coul) : _texte(txt), _police(police), _taille(taille), _couleur(coul), _dimensions() {
	_dimensions = Texte::dimensions(_texte, _police, _taille);
}

Texte::Texte(Texte const &t) : _texte(t._texte), _police(t._police), _taille(t._taille), _couleur(t._couleur), _dimensions(t._dimensions) {

}

Texte &Texte::operator=(Texte const &t) {
	_texte = t._texte;
	_police = t._police;
	_taille = t._taille;
	_dimensions = t._dimensions;
	_couleur = t._couleur;
	
	return *this;
}

Texte::~Texte() {

}

Texte &Texte::definir(Unichar const &txt) {
	_texte = txt;
	_dimensions = Texte::dimensions(_texte, _police, _taille);
	
	return *this;
}

Texte &Texte::definir(police_t police, taillePolice_t taille) {
	_police = police;
	_taille = taille;
	_dimensions = Texte::dimensions(_texte, _police, _taille);
	
	return *this;
}

Texte &Texte::definir(taillePolice_t taille) {
	_taille = taille;
	_dimensions = Texte::dimensions(_texte, _police, _taille);
	
	return *this;
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

void Texte::afficher(glm::vec2 const &pos) const {
	Texte::afficher(_texte, _police, _taille, _couleur, pos);
}

void Texte::afficher(Unichar const &txt, police_t police, taillePolice_t taille, Couleur const &coul, glm::vec2 pos) {
	Couleur teinte = Affichage::teinte();
	
	Affichage::ajouterOpacite(coul.a);
	Affichage::definirTeinte(Couleur(coul, 255));
	
	int hauteur = hauteurs[police];
	float tailleRendu = taillesRendu[police];
	Image *image = polices[police];
	if(!image) {
		image = polices[police] = new Image(Session::cheminRessources() + chemins[police]);
	}

	float rapportTaille = taille / tailleRendu;
	image->redimensionner(rapportTaille);

	initialiserCaracteres();
	glm::vec2 posInitiale = pos;
	
	for(Unichar::const_iterator i = txt.begin(); i != txt.end(); ++i) {
		Unichar::unichar c = *i;

		if(c == '\n') {
			pos.x = posInitiale.x;
			pos.y += 4.0 / 4.0 * hauteur * rapportTaille;
		}
		else {
			uindex_t p = std::find(caracteres, caracteres + nbCaracteres, c) - caracteres;
			if(p == nbCaracteres)
				std::cerr << "Le caractère " << c << " n'est pas géré par le système de texte." << std::endl;

			Rectangle filtre(largeurs[police][p][0], largeurs[police][p][2], largeurs[police][p][1], hauteur);
			image->afficher(pos, filtre);
			pos.x += filtre.largeur * rapportTaille;
		}
	}
	
	Affichage::definirTeinte(teinte);
	Affichage::supprimerOpacite();
}

static void initialiserCaracteres() {
	if(caracteres)
		return;
	Unichar texte(caracteresSupportes);
	nbCaracteres = texte.size() - 1;
	caracteres = new Unichar::unichar[nbCaracteres];
	std::copy(texte.unitxt(), texte.unitxt() + nbCaracteres, caracteres);
}

glm::vec2 Texte::dimensions(Unichar const &texte, police_t police, taillePolice_t taille) {
	glm::vec2 retour;
	if(!texte.size())
		return retour;
	
	unsigned int hauteur =  hauteurs[police];
	float tailleRendu = taillesRendu[police];
	initialiserCaracteres();
	
	double abscisse = 0;
	
	retour.y = hauteur * taille / tailleRendu;
	
	for(Unichar::const_iterator i = texte.begin(); i != texte.end(); ++i) {
		Unichar::unichar c = *i;
		
		if(c == '\n') {
			retour.x = std::max<coordonnee_t>(abscisse, retour.x);
			retour.y += 4.0 / 4 * hauteur * taille / tailleRendu;
			abscisse = 0;
		}
		else {
			uindex_t p = std::find(caracteres, caracteres + nbCaracteres, c) - caracteres;
			if(p == nbCaracteres)
				std::cerr << "Le caractère " << c << " n'est pas géré par le système de texte." << std::endl;

			abscisse += largeurs[police][p][1];
		}
	}
	
	retour.x = (std::max<coordonnee_t>(abscisse, retour.x)) * taille / tailleRendu;
	
	return retour;
}

dimension_t Texte::hauteur(police_t police, taillePolice_t taille) {
	unsigned int h = hauteurs[police];
	float tailleRendu = taillesRendu[police];

	return h * taille / tailleRendu;
}

void Texte::nettoyer() {
	for(police_t i = premierePolice; i != nbPolices; ++i) {
		delete polices[i];
	}

	delete[] caracteres;
}
