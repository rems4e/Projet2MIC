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

	Coordonnees const positionVie(50, 100);

	for(index_t i = 0; i != hBarre; ++i) {
		Ecran::afficherRectangle(Rectangle(positionVie.x * Ecran::echelle().x, posTab + (positionVie.y + i) * Ecran::echelle().y, lBarre * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(255 - 6 * std::abs(15 - i), 80, 80));
		Ecran::afficherRectangle(Rectangle(positionVie.x * Ecran::echelle().x, posTab + (positionVie.y + i) * Ecran::echelle().y, _joueur->vieActuelle() * lBarre / _joueur->vieTotale() * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(230 - 8 * std::abs(15 - i), 0, 0));
	}
	 
	Texte chiffres(nombreVersTexte(_joueur->vieActuelle()) + "/" + nombreVersTexte(_joueur->vieTotale()), POLICE_GRANDE, 16 * Ecran::echelleMin(), Couleur::blanc);
	chiffres.afficher(Coordonnees(positionVie.x * Ecran::echelle().x, posTab + positionVie.y * Ecran::echelle().y) + (Coordonnees(lBarre * Ecran::echelle().x, hBarre * Ecran::echelle().y) - chiffres.dimensions()) / 2);
	
	Coordonnees const positionXp(Ecran::largeur() / Ecran::echelle().x - 50 - lBarre , positionVie.y);
	for(index_t i = 0; i != hBarre; ++i) {
		Ecran::afficherRectangle(Rectangle(positionXp.x * Ecran::echelle().x, posTab + (positionXp.y + i) * Ecran::echelle().y, lBarre * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(12 * std::abs(15 - i), 12 * std::abs(15 - i), 12 * std::abs(15 - i)));
		Ecran::afficherRectangle(Rectangle(positionXp.x * Ecran::echelle().x, posTab + (positionXp.y + i) * Ecran::echelle().y, _joueur->xp() * lBarre / _joueur->xpTotale() * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(6 * std::abs(15 - i), 6 * std::abs(15 - i), 6 * std::abs(15 - i)));
	}
	
	chiffres.definir("Niveau " + nombreVersTexte(_joueur->niveauXp()) + " - " + nombreVersTexte(_joueur->xp()) + "/" + nombreVersTexte(_joueur->xpTotale()));
	chiffres.afficher(Coordonnees(positionXp.x * Ecran::echelle().x, posTab + positionXp.y * Ecran::echelle().y) + (Coordonnees(lBarre * Ecran::echelle().x, hBarre * Ecran::echelle().y) - chiffres.dimensions()) / 2);

	std::string txt;
	switch(_joueur->interaction()) {
		case Joueur::ij_attaquer: {
			Personnage *p = _joueur->attaque();
			txt = "Attaquer l'ennemi (" + nombreVersTexte(p->vieActuelle()) + "/" + nombreVersTexte(p->vieTotale()) + " vie)";
			break;
		}
		case Joueur::ij_commerce:
			txt = "Acheter/vendre des objets";
			break;
		case Joueur::ij_ramasser: {
			std::pair<size_t, size_t> nbObjets = _joueur->nombreObjets();
			txt = "Ramasser ";
			if(nbObjets.first) {
				txt += nombreVersTexte(nbObjets.first) + " objet";
				if(nbObjets.first > 1)
					txt += "s";
			}
			if(nbObjets.second) {
				if(nbObjets.first)
					txt += " et ";
				txt += nombreVersTexte(nbObjets.second) + " or";
			}
			break;
		}
		case Joueur::ij_finirNiveau:
			txt = "Terminer le niveau";
		case Joueur::ij_aucune:
			break;
	}
	
	_interactionJoueur.definir(12 * Ecran::echelleMin());
	_interactionJoueur.definir(txt);
	_interactionJoueur.afficher(Coordonnees((Ecran::largeur() - _interactionJoueur.dimensions().x) / 2, posTab + (positionVie.y + hBarre / 2) * Ecran::echelle().y - _interactionJoueur.dimensions().y / 2));
}

dimension_t TableauDeBord::hauteur() const {
	return 150 * Ecran::echelle().y;
}

