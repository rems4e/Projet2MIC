/*
 *  BoiteDialogue.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 12/05/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#include "BoiteDialogue.h"
#include "ObjetInterface.h"
#include "Session.h"
#include "Ecran.h"

namespace BoiteDialogue {
	inline void bouton(ControleInterface *c, void *p) {
		*static_cast<Unichar *>(p) = static_cast<Bouton *>(c)->texte();
		Session::supprimerVueFenetre();
	}
}

Unichar BoiteDialogue::afficher(Unichar const &texte, Unichar const &description, Unichar const &bouton1, Unichar const &bouton2, Unichar const &bouton3) {
	Unichar retour;

	AfficheurImage fond(Ecran::ecran(), *Ecran::apercu(), Couleur::noir, Couleur(Couleur::noir, 150));
	TexteEtiquette titre(Unichar(), glm::vec2(20, 20), POLICE_NORMALE, 18, Couleur::blanc);
	
	titre.definirDimensions(glm::vec2(Ecran::largeur() * 4 / 5 - 40, 10));
	titre.definirValeurTexte(texte);
	
	TexteEtiquette sousTitre(Unichar(), glm::vec2(20, titre.position().y + titre.dimensions().y + 20), POLICE_NORMALE, 15, Couleur::blanc);
	sousTitre.definirDimensions(glm::vec2(Ecran::largeur() * 4 / 5 - 40, 10));
	sousTitre.definirValeurTexte(description);
	
	
	Bouton b1(glm::vec2(50, 0), bouton1);
	Bouton b2(glm::vec2(50, 0), bouton2);
	Bouton b3(glm::vec2(50, 0), bouton3);

	double largeur = b1.dimensions().x;
	if(!bouton2.empty())
		largeur += 50 + b2.dimensions().x;
	if(!bouton3.empty())
		largeur += 50 + b3.dimensions().x;

	b1.definirPosition(glm::vec2(Ecran::largeur() * 4 / 5 - 20 - b1.dimensions().x, titre.position().y + titre.dimensions().y + 50));
	b1.definirAction(BoiteDialogue::bouton);
	b1.definirParametreAction(&retour);
	b1.definirRaccourci(Session::T_ENTREE);
	
	if(!bouton2.empty()) {
		b2.definirPosition(glm::vec2(b1.position().x - 20 - b2.dimensions().x, titre.position().y + titre.dimensions().y + 50));

		b2.definirAction(BoiteDialogue::bouton);
		b2.definirParametreAction(&retour);
		b2.definirRaccourci(Session::T_ESC);
	}
	if(!bouton3.empty()) {
		if(!bouton2.empty())
			b3.definirPosition(glm::vec2(b2.position().x - 50 - b3.dimensions().x, titre.position().y + titre.dimensions().y + 50));
		else
			b3.definirPosition(glm::vec2(b1.position().x - 50 - b3.dimensions().x, titre.position().y + titre.dimensions().y + 50));

		b3.definirAction(BoiteDialogue::bouton);
		b3.definirParametreAction(&retour);
		b3.definirRaccourci(Session::T_ESPACE);
	}
	
	AfficheurCouleur cadre(Rectangle(0, 0, Ecran::largeur() * 4 / 5, b1.cadre().haut + b1.cadre().hauteur + 20), Couleur(Couleur::noir, 120));
	titre.definirPosition(glm::vec2(20, 20));
	cadre.definirPosition(glm::vec2(std::max<coordonnee_t>(0.0, (Ecran::largeur() - cadre.dimensions().x)/ 2), 3 * Ecran::hauteur() / 7 - cadre.dimensions().y /2));
	
	fond.ajouterEnfant(cadre);
	
	cadre.ajouterEnfant(b1);
	if(!bouton2.empty())
		cadre.ajouterEnfant(b2);
	if(!bouton3.empty())
		cadre.ajouterEnfant(b3);

	cadre.ajouterEnfant(titre);
	cadre.ajouterEnfant(sousTitre);
	
	Ecran::definirPointeurAffiche(true);
	
	Session::ajouterVueFenetre(&fond);
	
	if(Session::evenement(Session::QUITTER))
		Session::reinitialiser(Session::QUITTER);
		
	return retour;
}
