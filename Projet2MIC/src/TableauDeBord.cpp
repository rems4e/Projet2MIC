#include "TableauDeBord.h"
#include "Geometrie.h"
#include "Ecran.h"
#include "Joueur.h"
#include "Texte.h"
#include "fonctions.h"

TableauDeBord::TableauDeBord(Joueur *j) : _joueur(j), _fond(Session::cheminRessources() + "clearedFont.png"){
	
}


TableauDeBord::~TableauDeBord() {
	
}

void TableauDeBord::afficher() {
	Image::definirOpacite(220);
	dimension_t posTab = Ecran::hauteur() - 150;
	_fond.afficher(Coordonnees(0, posTab));
	Image::definirOpacite(255);
	
	size_t lBarre = 200, hBarre = 30;

	for(index_t i = 0; i != hBarre; ++i) {
		Ecran::afficherRectangle(Rectangle(50, posTab + 100 + i, lBarre, 1), Couleur(255 - 6 * std::abs(15 - i), 80, 80));
		Ecran::afficherRectangle(Rectangle(50, posTab + 100 + i, _joueur->vieActuelle() * 200 / _joueur->vieTotale(), 1), Couleur(230 - 8 * std::abs(15 - i), 0, 0));
	}
	 
	Texte chiffresVie(nombreVersTexte(_joueur->vieActuelle()) + "/" + nombreVersTexte(_joueur->vieTotale()), POLICE_GRANDE, 16, Couleur::blanc);
	chiffresVie.afficher(Coordonnees(50, posTab + 100) + (Coordonnees(lBarre, hBarre) - chiffresVie.dimensions()) / 2);
}
