/*
 *  MenuDeroulant.h
 *  Jeu C++
 *
 *  Created by Rémi on 23/02/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "Bouton.h"
#include <vector>

class MenuInterface : public VueInterface {
public:
	struct sous_menu {
		Texte _texte;
		index_t _tag;
		glm::vec2 _pos;
		//bool _selectionne;
		
		inline sous_menu(Unichar const &txt) : _texte(txt), _tag(0), _pos() { }
		inline sous_menu(std::pair<Unichar, index_t> const &txt) : _texte(txt.first), _tag(txt.second), _pos() { }
	};
	
	MenuInterface(VueInterface *parent, std::vector<Unichar> const &elements, int sousMenuSelectionne = 0);
	MenuInterface(VueInterface *parent, std::vector<std::pair<Unichar, index_t>> const &elements, int sousMenuSelectionne = 0);
	virtual ~MenuInterface() { }
	
	inline const std::vector<sous_menu> &elements() const {
		return _elements;
	}
	void definirElements(std::vector<Unichar> const &elements);
	void definirElements(std::vector<std::pair<Unichar, index_t>> const &elements);

	inline Unichar const &menuSelectionne() const { return this->valeurIndeterminee() ? Unichar::uninull : _elements[this->sousMenuSelectionne()]._texte.texte(); }
	inline index_t tagSelectionne() const { return _elements[this->sousMenuSelectionne()]._tag; }

	inline index_t sousMenuSelectionne() const { return _sousMenuSelectionne; }
	void definirSousMenuSelectionne(index_t selection);
	void definirSousMenuSelectionneAvecTag(index_t tag);
	
	inline Texte &texteSelectionne() { return _elements[this->sousMenuSelectionne()]._texte; }
	
	inline glm::vec2 const &centrage() const { return _centrage; }
	inline void definirCentrage(glm::vec2 const &c) { _centrage = c; }
	
	void definirMasque(bool m);

	inline bool valeurIndeterminee() const { return _valeurIndeterminee; }
	inline virtual void definirValeurIndeterminee(bool i) { _valeurIndeterminee = i; }
		
private:
	std::vector<sous_menu> _elements;
	index_t _sousMenuSelectionne = 0;
	index_t _sousMenuSurvole = 0;
	glm::vec2 _centrage = glm::vec2(0, 0);
	index_t _premierAffiche = 0;
	index_t _dernierAffiche = 0;
	bool _valeurIndeterminee = false;
	
	static Image _fH, _fB;
	static double _hF;
	static bool _imagesChargees;
	static horloge_t _horlogeDefilement;

	void dessiner();
	void gestionSouris(bool vueActuelle, glm::vec2 const &souris, bool gauche, bool droit);
	void gestionClavier();
	void prendreLaMain(VueInterface *ancienne);
	void rendreLaMain(VueInterface *prochainne);
	inline bool focusClavier() const { return true; }

	void insererBlanc();
	void supprimerBlanc();
	
	void init(VueInterface *parent, index_t selection);
};

class MenuDeroulant : public ControleInterface {
	friend class MenuInterface;
public:
	MenuDeroulant(glm::vec2 const &pos, std::vector<Unichar> const &elements, int sousMenuSelectionne = 0);
	virtual ~MenuDeroulant();
	
	inline Unichar const &menuSelectionne() const { return _menu.menuSelectionne(); }
	inline void definirSousMenuSelectionne(int selection) { _menu.definirSousMenuSelectionne(selection); }
	inline void definirSousMenuSelectionneAvecTag(index_t tag) { _menu.definirSousMenuSelectionneAvecTag(tag); }
	
	inline Unichar valeurTexte() const { return this->menuSelectionne(); }
	inline int valeurInt() const { return static_cast<int>(_menu.sousMenuSelectionne()); }
	
	inline index_t tagSelectionne() const { return _menu.tagSelectionne(); }

	inline void definirElements(std::vector<Unichar> const &elements) { _menu.definirElements(elements); }
	inline void definirElements(std::vector<std::pair<Unichar, index_t>> const &elements) { _menu.definirElements(elements); }
	inline void definirValeurIndeterminee(bool ind) {
		_menu.definirValeurIndeterminee(ind);
	}
	
	inline bool valeurIndeterminee() const {
		return _menu.valeurIndeterminee();
	}

	
private:
	MenuInterface _menu;
		
	void dessiner();
	void gestionSouris(bool vueActuelle, glm::vec2 const &souris, bool gauche, bool droit);
		
	static Image _g, _d, _m;
	static bool _imagesChargees;
	
	inline void exec() { this->executerAction(); }
	
	MenuDeroulant(MenuDeroulant const &) = delete;
	MenuDeroulant &operator=(MenuDeroulant const &) = delete;
};
