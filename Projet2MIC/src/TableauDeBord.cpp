#include "TableauDeBord.h"
#include "Geometrie.h"
#include "Affichage.h"
#include "Joueur.h"
#include "nombre.h"

TableauDeBord::TableauDeBord(Joueur *j) : VueInterface(Rectangle::zero), _joueur(j), _fond(Session::cheminRessources() + "clearedFont.png"), _interactionJoueur("", POLICE_NORMALE, 12, Couleur::noir) {
	
}

TableauDeBord::~TableauDeBord() {
	
}

void TableauDeBord::preparationDessin() {
	float facteur = Ecran::largeur() / _fond.dimensionsReelles().x;
	_fond.redimensionner(glm::vec2(facteur, Ecran::echelle().y));
	this->definirCadre(Rectangle(0, Ecran::hauteur() - this->hauteur(), _fond.dimensions().x, _fond.dimensions().y));
}

void TableauDeBord::dessiner() {
	Affichage::ajouterOpacite(220);
	_fond.afficher(glm::vec2(0, 0));
	Affichage::supprimerOpacite();
	
	size_t lBarre = 200, hBarre = 30;

	glm::vec2 const positionVie(50, 100);

	for(index_t i = 0; i != hBarre; ++i) {
		Affichage::afficherRectangle(Rectangle(positionVie.x * Ecran::echelle().x, (positionVie.y + i) * Ecran::echelle().y, lBarre * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(255 - 6 * std::abs(15 - i), 80, 80));
		Affichage::afficherRectangle(Rectangle(positionVie.x * Ecran::echelle().x, (positionVie.y + i) * Ecran::echelle().y, _joueur->vieActuelle() * lBarre / _joueur->vieTotale() * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(230 - 8 * std::abs(15 - i), 0, 0));
	}
	 
	Texte chiffres(TRAD("tabBord vie %1/%2", _joueur->vieActuelle(), _joueur->vieTotale()), POLICE_GRANDE, 16 * Ecran::echelleMin(), Couleur::blanc);
	chiffres.afficher(glm::vec2(positionVie.x * Ecran::echelle().x, positionVie.y * Ecran::echelle().y) + (glm::vec2(lBarre * Ecran::echelle().x, hBarre * Ecran::echelle().y) - chiffres.dimensions()) / 2.0f);
	
	glm::vec2 const positionXp(Ecran::largeur() / Ecran::echelle().x - 50 - lBarre , positionVie.y);
	for(index_t i = 0; i != hBarre; ++i) {
		Affichage::afficherRectangle(Rectangle(positionXp.x * Ecran::echelle().x, (positionXp.y + i) * Ecran::echelle().y, lBarre * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(12 * std::abs(15 - i), 12 * std::abs(15 - i), 12 * std::abs(15 - i)));
		Affichage::afficherRectangle(Rectangle(positionXp.x * Ecran::echelle().x, (positionXp.y + i) * Ecran::echelle().y, _joueur->xp() * lBarre / _joueur->xpTotale() * Ecran::echelle().x, 1 * Ecran::echelle().y), Couleur(6 * std::abs(15 - i), 6 * std::abs(15 - i), 6 * std::abs(15 - i)));
	}
	
	chiffres.definir(TRAD("tabBord Niveau %1 - %2/%3", _joueur->niveauXp(), _joueur->xp(), _joueur->xpTotale()));
	chiffres.afficher(glm::vec2(positionXp.x * Ecran::echelle().x,  positionXp.y * Ecran::echelle().y) + (glm::vec2(lBarre * Ecran::echelle().x, hBarre * Ecran::echelle().y) - chiffres.dimensions()) / 2.0f);

	Unichar txt;
	switch(_joueur->interaction()) {
		case Joueur::ij_attaquer: {
			Personnage *p = _joueur->attaque();
			txt = TRAD("tabBord Attaquer l'ennemi (%1/%2 vie)", p->vieActuelle(), p->vieTotale());
			break;
		}
		case Joueur::ij_commerce:
			txt = TRAD("tabBord Acheter/vendre des objets");
			break;
		case Joueur::ij_ramasser: {
			std::pair<size_t, size_t> nbObjets = _joueur->nombreObjets();
			if(nbObjets.first == 1 && nbObjets.second == 0)
				txt = TRAD("tabBord Ramasser 1 objet");
			else if(nbObjets.first > 1 && nbObjets.second == 0)
				txt = TRAD("tabBord Ramasser %1 objets", nbObjets.first);
			else if(nbObjets.first == 1 && nbObjets.second == 1)
				txt = TRAD("tabBord Ramasser 1 objet et 1 or");
			else if(nbObjets.first > 1 && nbObjets.second == 1)
				txt = TRAD("tabBord Ramasser %1 objets et 1 or", nbObjets.first);
			else if(nbObjets.first == 0 && nbObjets.second == 1)
				txt = TRAD("tabBord Ramasser 1 or");
			else if(nbObjets.first == 1 && nbObjets.second > 1)
				txt = TRAD("tabBord Ramasser 1 objet et %1 or", nbObjets.second);
			else if(nbObjets.first > 1 && nbObjets.second > 1)
				txt = TRAD("tabBord Ramasser %1 objets et %2 or", nbObjets.first, nbObjets.second);
			else if(nbObjets.first == 0 && nbObjets.second > 1)
				txt = TRAD("tabBord Ramasser %1 or", nbObjets.second);
			break;
		}
		case Joueur::ij_finirNiveau:
			txt = TRAD("Terminer le niveau");
		case Joueur::ij_aucune:
			break;
	}
	
	_interactionJoueur.definir(12 * Ecran::echelleMin());
	_interactionJoueur.definir(txt);
	_interactionJoueur.afficher(glm::vec2((Ecran::largeur() - _interactionJoueur.dimensions().x) / 2, (positionVie.y + hBarre / 2) * Ecran::echelle().y - _interactionJoueur.dimensions().y / 2));
}

void TableauDeBord::gestionClavier() {
	
}

dimension_t TableauDeBord::hauteur() const {
	return 150 * Ecran::echelle().y;
}

