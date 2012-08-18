/*
 *  ObjetInterface.h
 *  Jeu C++
 *
 *  Created by Rémi on 07/05/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#ifndef OBJET_INTERFACE_H
#define OBJET_INTERFACE_H

#include "VueInterface.h"
#include "Bouton.h"
#include "ChampDeTexte.h"
#include "AfficheurImage.h"
#include "BoiteDialogue.h"
#include "VueDefilante.h"
#include "MenuDeroulant.h"


struct ObjetInterface {
	enum objet_interface_t  {DESCRIPTION, BOUTON, CASE_A_COCHER, CHAMP_DE_TEXTE, CHAMP_DE_NOMBRE, MENU_DEROULANT, AFFICHEUR_IMAGE};
	enum saut_t {LIGNE, LIGNE_GROUPEE, COLONNE};
	
	objet_interface_t type;
	saut_t saut;
	glm::vec2 dimensions;
	Unichar valeur, description;
	VueInterface::VueInterface::tag_t tag;
	ControleInterface::ActionParam action;
	void *paramAction;
	
	static inline ObjetInterface descriptionObjets(Unichar const &_valeur) { return ObjetInterface(DESCRIPTION, 0, "", _valeur, LIGNE, glm::vec2(0)); }
	
	inline ObjetInterface(objet_interface_t _type, VueInterface::tag_t _tag, Unichar const &_valeur, Unichar const &_description, saut_t _saut, glm::vec2 const &_dimensions, ControleInterface::ActionParam const &_action = 0, void *_paramAction = 0) : type(_type), tag(_tag), valeur(_valeur), description(_description), saut(_saut), dimensions(_dimensions), action(_action), paramAction(_paramAction) { }
	inline virtual ~ObjetInterface() { }
	
	inline ObjetInterface(ObjetInterface const &o) : type(o.type), tag(o.tag), valeur(o.valeur), description(o.description), saut(o.saut), dimensions(o.dimensions), action(o.action), paramAction(o.paramAction) { }

private:
	ObjetInterface &operator=(ObjetInterface const &);
};
	

VueInterface *vueAvecObjets(std::list<ObjetInterface> const &objets, glm::vec2 const &ecartement = glm::vec2(20, 20));


#endif
