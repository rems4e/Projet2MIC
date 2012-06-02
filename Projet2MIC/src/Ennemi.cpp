//
//  Ennemi.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Ennemi.h"
#include "Joueur.h"
#include "tinyxml.h"

Ennemi::Ennemi(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(decoupagePerspective, n, index, cat, new InventaireEnnemi(*this, CAPACITE_ENNEMI)), _recherche(false) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	for(competences_t c = premiereCompetence; c != nbCompetences; ++c) {
		_coef[c] = 1.0;
		std::string val = "c" + nombreVersTexte(c);
		if(e->Attribute(val))
			e->Attribute(val, &(_coef[c]));
	}

	this->equilibrerAvecJoueur();
	if(cat == boss) {
		this->inventaire()->ajouterObjet(ElementNiveau::elementNiveau<ObjetInventaire>(false, n, 666));
	}
}

Ennemi::~Ennemi() {
	
}

void Ennemi::animer() {
	this->Personnage::animer();
	if(this->mort()) {
		_recherche = false;
		return;
	}

	Joueur *j = this->niveau()->joueur();
	if((j->position() - this->position()).norme() < this->porteeVision() * LARGEUR_CASE) {
		_cible = j->position();
		_recherche = true;

		bool suivre;
		Coordonnees dep = _cible - this->position();
		if(dep.vecteurNul()) {
			suivre = false;
		}
		else {
			dep.normaliser();
			dep *= this->vitesse() * 60.0 / Ecran::frequenceInstantanee();
			index_t pX = this->nPX(this->position().x + dep.x), pY = this->nPY(this->position().y + dep.y);
			suivre = j->pX() != pX || j->pY() != pY;
			Niveau::const_listeElements_t liste = this->niveau()->elements(pX, pY, this->couche());
			for(Niveau::elements_t::const_iterator el = liste.first; el != liste.second; ++el) {
				if(el->entite != this && el->entite->mobile() && static_cast<EntiteMobile *>(el->entite)->categorieMobile() == EntiteMobile::em_ennemi) {
					Ennemi *e = static_cast<Ennemi *>(el->entite);
					
					if(e->_recherche) {
						suivre = false;
						break;
					}
				}
			}
		}
		if(suivre && this->deplacerPosition(dep) && this->definirAction(EntiteMobile::a_deplacer)) {
			
		}
		else {
			this->definirAction(EntiteMobile::a_immobile);
		}
	}
	else {
		this->definirAction(EntiteMobile::a_immobile);
		_cible = Coordonnees::aucun;
		_recherche = false;
	}
}

bool Ennemi::interagir(Personnage *p, bool test) {
	switch(p->categorieMobile()) {
		case EntiteMobile::em_joueur:
			this->attaquer(p);
			return true;
		case EntiteMobile::em_ennemi:
		case EntiteMobile::em_marchand:
			break;
	}
	
	return false;
}

void Ennemi::jeterObjets() {
	InventaireEnnemi *e = static_cast<InventaireEnnemi *>(this->inventaire());
	std::list<ObjetInventaire *> liste;
	index_t pX = this->pX(), pY = this->pY();
	for(InventaireEnnemi::iterator i = e->debut(); i != e->fin(); ++i) {
		liste.push_back(*i);
		this->niveau()->ajouterElement(pX, pY, Niveau::cn_objetsInventaire, *i);
	}
	e->vider();	
}

index_t Ennemi::porteeVision() const {
	return 8;
}

double Ennemi::vitesse() const {
	return Personnage::vitesse() * 0.75;
}

EntiteMobile::categorie_t Ennemi::categorieMobile() const {
	return EntiteMobile::em_ennemi;
}

void Ennemi::equilibrerAvecJoueur() {
	Competences cc = this->niveau()->joueur()->competences();
	for(competences_t c = premiereCompetence; c != nbCompetences; ++c) {
		double nb = (nombreAleatoire(21) - 10) / 100.0;
		cc[c] *= std::max(0.0, _coef[c] + nb);
		cc[c] = std::max<size_t>(cc[c] / 1.5, 1);
	}
	this->definirCompetences(cc);
	this->modifierVieActuelle(cc[endurance] * 10);

	if(nombreAleatoire(2))
		this->inventaire()->modifierMonnaie(10 + 10 * this->niveau()->joueur()->niveauXp());
	else {
		ObjetInventaire *o = ElementNiveau::elementNiveau<ObjetInventaire>(false, this->niveau(), nombreAleatoire(ElementNiveau::nombreEntites(ElementNiveau::objetInventaire)));
		this->inventaire()->ajouterObjet(o);
	}
}

void Ennemi::mourir() {
	this->Personnage::mourir();
	this->niveau()->joueur()->gagnerXp(this);
}
