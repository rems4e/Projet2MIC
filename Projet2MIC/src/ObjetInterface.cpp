/*
 *  ObjetInterface.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 07/05/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#include "ObjetInterface.h"

VueInterface *vueAvecObjets(std::list<ObjetInterface> const &objets, glm::vec2 const &ecartement) {
	VueInterface *retour = new VueInterface(Rectangle(0, 0, 2 * ecartement.x, 2 * ecartement.y));
	glm::vec2 position(ecartement), pMax;
	coordonnee_t largeur = ecartement.x;
	
	for(std::list<ObjetInterface>::const_iterator i = objets.begin(); i != objets.end(); ++i) {
		VueInterface *o = 0;
		TexteEtiquette *desc = 0;
		glm::vec2 posObj(position);
		if(!i->description.empty()) {
			desc = new TexteEtiquette(i->description, position, POLICE_NORMALE, 14, Couleur(200));
			posObj += glm::vec2(0, 10 + desc->dimensions().y);
		}
		switch(i->type) {
			case ObjetInterface::DESCRIPTION:
				break;
			case ObjetInterface::BOUTON:
				if(i->valeur.size() && i->valeur[0] == '/')
					o = new Bouton(posObj, Image(i->valeur));
				else
					o = new Bouton(posObj, i->valeur);
				static_cast<ControleInterface *>(o)->definirAction(i->action);
				static_cast<ControleInterface *>(o)->definirParametreAction(i->paramAction);
				break;
			case ObjetInterface::CASE_A_COCHER:
				o = new CaseACocher(posObj, i->valeur, false, Couleur(200));
				static_cast<ControleInterface *>(o)->definirAction(i->action);
				static_cast<ControleInterface *>(o)->definirParametreAction(i->paramAction);
				break;
			case ObjetInterface::CHAMP_DE_TEXTE:
				o = new ChampDeTexte(Rectangle(posObj, i->dimensions));
				static_cast<ChampDeTexte *>(o)->definirValeurTexte(i->valeur);
				static_cast<ControleInterface *>(o)->definirAction(i->action);
				static_cast<ControleInterface *>(o)->definirParametreAction(i->paramAction);
				break;
			case ObjetInterface::CHAMP_DE_NOMBRE:
				o = new ChampDeNombre(Rectangle(posObj, i->dimensions));
				static_cast<ChampDeNombre *>(o)->definirValeurTexte(i->valeur);
				static_cast<ControleInterface *>(o)->definirAction(i->action);
				static_cast<ControleInterface *>(o)->definirParametreAction(i->paramAction);
				break;
			case ObjetInterface::MENU_DEROULANT:
				o = new MenuDeroulant(posObj, std::vector<Unichar>(1, TRAD("mdef <menu non initialisé>")));
				static_cast<ControleInterface *>(o)->definirAction(i->action);
				static_cast<ControleInterface *>(o)->definirParametreAction(i->paramAction);
				break;
			case ObjetInterface::AFFICHEUR_IMAGE:
				o = new AfficheurImage(Rectangle(posObj, i->dimensions), Image(i->valeur));
				break;
		}
		
		if(o) {
			o->definirTag(i->tag);
			if(desc)
				retour->ajouterEnfant(*desc);
			retour->ajouterEnfant(*o);
			pMax.x = o->position().x + std::max(desc ? desc->dimensions().x : 0, o->dimensions().x);
			pMax.y = std::max(pMax.y, o->position().y + o->dimensions().y);
			largeur = std::max(largeur, pMax.x);
			
			if(i->saut == ObjetInterface::LIGNE) {
				position.x = ecartement.x;
				position.y = pMax.y + ecartement.y;
			}
			else if(i->saut == ObjetInterface::LIGNE_GROUPEE) {
				position.x = ecartement.x;
				position.y = pMax.y + ecartement.y / 2;
			}
			else if(i->saut == ObjetInterface::COLONNE) {
				position.x = pMax.x + ecartement.x;
			}
		}
		else {
			retour->ajouterEnfant(*desc);
			pMax.x = desc->position().x + desc->dimensions().x;
			pMax.y = desc->position().y + desc->dimensions().y;
			largeur = std::max(largeur, pMax.x);
		
			position.x = ecartement.x;
			position.y = pMax.y + ecartement.y / 2;
		}			
	}
	
	retour->definirDimensions(glm::vec2(largeur, pMax.y) + ecartement);
	
	return retour;
}
