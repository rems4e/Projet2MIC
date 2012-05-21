#include "TableauDeBord.h"
#include "Geometrie.h"
#include "Ecran.h"
#include "Joueur.h"
#include "fonctions.h"

TableauDeBord::TableauDeBord(Joueur *j) : _joueur(j), _fond(Session::cheminRessources() + "clearedFont.png"), _interactionJoueur("", POLICE_NORMALE, 12, Couleur::noir) {
	
}


TableauDeBord::~TableauDeBord() {
	
}

void TableauDeBord::afficher() {
	dimension_t posTab = Ecran::hauteur() - this->hauteur();
	
	float facteur = Ecran::largeur() / _fond.dimensionsReelles().x;
	_fond.redimensionner(Coordonnees(facteur, Ecran::echelle().y));

	Image::definirOpacite(220);
	_fond.afficher(Coordonnees(0, posTab));
	Image::definirOpacite(255);
	
	size_t lBarre = 200, hBarre = 30;

	for(index_t i = 0; i != hBarre; ++i) {
		Ecran::afficherRectangle(Rectangle(50 * Ecran::echelle().x, posTab + (100 + i) * Ecran::echelle().y, lBarre * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(255 - 6 * std::abs(15 - i), 80, 80));
		Ecran::afficherRectangle(Rectangle(50 * Ecran::echelle().x, posTab + (100 + i) * Ecran::echelle().y, _joueur->vieActuelle() * lBarre / _joueur->vieTotale() * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(230 - 8 * std::abs(15 - i), 0, 0));
	}
	 
	Texte chiffresVie(nombreVersTexte(_joueur->vieActuelle()) + "/" + nombreVersTexte(_joueur->vieTotale()), POLICE_GRANDE, 16 * Ecran::echelleMin(), Couleur::blanc);
	chiffresVie.afficher(Coordonnees(50 * Ecran::echelle().x, posTab + 100 * Ecran::echelle().y) + (Coordonnees(lBarre * Ecran::echelle().x, hBarre * Ecran::echelle().y) - chiffresVie.dimensions()) / 2);
	
	char const *txt = "";
	switch(_joueur->interaction()) {
		case Joueur::ij_attaquer:
			txt = "Attaquer l'ennemi";
			break;
		case Joueur::ij_commerce:
			txt = "Acheter/vendre des objets";
			break;
		case Joueur::ij_aucune:
			break;
	}
	
	_interactionJoueur.definir(12 * Ecran::echelleMin());
	_interactionJoueur.definir(txt);
	_interactionJoueur.afficher(Coordonnees((Ecran::largeur() - _interactionJoueur.dimensions().x) / 2, Ecran::hauteur() - _interactionJoueur.dimensions().y - 20 * Ecran::echelle().y));
}

dimension_t TableauDeBord::hauteur() const {
	return 150 * Ecran::echelle().y;
}

