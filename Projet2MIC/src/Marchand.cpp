//
//  Marchand.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "Marchand.h"
#include "Session.h"

Marchand::Marchand(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(decoupagePerspective, n, index, cat, new InventaireMarchand(*this)) {

}

Marchand::~Marchand() {
	
}

void Marchand::animer() {
	Personnage::animer();
}

bool Marchand::interagir(Personnage *p, bool test) {
	switch(p->categorieMobile()) {
		case EntiteMobile::em_joueur:
		case EntiteMobile::em_ennemi:
		case EntiteMobile::em_marchand:
			break;
	}
	
	return false;
}

void Marchand::jeterObjets() {
	
}

EntiteMobile::categorie_t Marchand::categorieMobile() const {
	return EntiteMobile::em_marchand;
}

bool Marchand::collision(index_t x, index_t y) const {
	return true;
}

ssize_t Marchand::prixAchat(ObjetInventaire *o) {
	return std::max<ssize_t>(this->prixVente(o) * 75 / 100, 1);
}

ssize_t Marchand::prixVente(ObjetInventaire *o) {	
	return 5 * (o->attaque() + o->defense()) + o->vie();
}
