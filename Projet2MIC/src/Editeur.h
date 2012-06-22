//
//  Editeur.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Editeur_h
#define Projet2MIC_Editeur_h

#include <string>
#include <vector>
#include <exception>
#include "Niveau.h"
#include "Image.h"
#include "ElementNiveau.h"
#include "Geometrie.h"
#include <list>
#include <set>
#include <stack>

class Editeur {
public:
	class Exc_ChargementEditeur : public std::exception {
	public:
		Exc_ChargementEditeur(std::string const &v) throw() : std::exception(), _valeur(v) {
			std::cerr << v << std::endl;
		}
		virtual ~Exc_ChargementEditeur() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};

	static Editeur *editeur();
	void ouvrirEditeur(Image &fond, Shader &);
	
	virtual ~Editeur();

private:
	enum Outil {o_selection, o_coller};
	
	struct ElementEditeur {
		ElementEditeur(ElementNiveau::elementNiveau_t cat, index_t index);
		ElementEditeur(index_t loiProba);
		
		uint16_t operator()() const;
		
		Image const &image() const;
		Rectangle cadre() const;
		Coordonnees dimensions() const;
		Coordonnees origine() const;
		Couleur teinte() const;
		
	private:
		bool _proba;
		index_t _indexProba;
		index_t _index;
		
		Image _image;
		Rectangle _cadre;
		Coordonnees _origine;
		Couleur _teinte;
		Coordonnees _dimensions;
		
		ElementNiveau::elementNiveau_t _categorie;
	};
	
	struct Case {
		ElementEditeur *_contenu[Niveau::nbCouches];
		
		Case();
		ElementEditeur * &operator[](Niveau::couche_t c);
	};
	
	typedef std::vector<Case> Colonne;
	typedef std::vector<Colonne> Ligne;
	
	struct LoiProba {
		LoiProba(std::string const &nom, std::string const &loi = "");
		int &operator[](ElementNiveau::elementNiveau_t c);

		int _proba[ElementNiveau::nbTypesElement];
		std::string _nom;
	};

	struct NiveauEditeur {
		NiveauEditeur(std::string const &fichier);
		~NiveauEditeur();
		
		Ligne _elements;
		std::vector<LoiProba> _probas;
		std::string const _fichier;
		size_t _dimX, _dimY;
		std::string _musique;
	};

	typedef void (Editeur::*fonctionEditeur_t)();
	
	typedef std::list<std::pair<Rectangle, fonctionEditeur_t> > listeFonctions_t;
	enum etat_selection_t {es_ok, es_aj, es_sup};
	struct ElementSelection {
		ElementEditeur const *_e;
		index_t _posX, _posY;
		etat_selection_t _etat;
		ElementSelection(ElementEditeur const *e, index_t posX, index_t posY, etat_selection_t etat) : _e(e), _posX(posX), _posY(posY), _etat(etat) { }
		
		inline bool operator==(ElementSelection const &e) const {
			if(_e == 0 && e._e == 0)
				return _posX == e._posX && _posY == e._posY;
			return _e == e._e; }
	};

	typedef std::list<ElementSelection> selection_t;
	
	class ActionEditeur {
	public:
		ActionEditeur() {
			
		}
		virtual ~ActionEditeur() {
			
		}
		
		virtual void operator()() = 0;
		virtual ActionEditeur *oppose() const = 0;
		
		index_t pX() {
			return _x;
		}
		index_t pY() {
			return _y;
		}
		
	protected:
		void definirPosition(index_t x, index_t y) {
			_x = x, _y = y;
		}
	private:
		index_t _x, _y;
	};
			
	class ActionsEditeur : public ActionEditeur {
	public:
		ActionsEditeur(std::list<ActionEditeur *> a) : _a(a) {
			index_t mX = std::numeric_limits<index_t>::max(), mY = std::numeric_limits<index_t>::max();
			for(std::list<ActionEditeur *>::const_iterator i =_a.begin(); i != _a.end(); ++i) {
				mX = std::min(mX, (*i)->pX());
				mY = std::min(mY, (*i)->pY());
			}
			
			this->definirPosition(mX, mY);
		}
		
		~ActionsEditeur() {
			for(std::list<ActionEditeur *>::iterator i = _a.begin(); i != _a.end(); ++i) {
				delete *i;
			}
		}
		
		void operator()() {
			for(std::list<ActionEditeur *>::const_iterator i =_a.begin(); i != _a.end(); ++i) {
				(*i)->operator()();
			}
		}
		
		ActionEditeur *oppose() const {
			std::list<ActionEditeur *> l;
			for(std::list<ActionEditeur *>::const_iterator i =_a.begin(); i != _a.end(); ++i) {
				l.push_back((*i)->oppose());
			}
			
			return new ActionsEditeur(l);
		}
		
		size_t taille() const {
			return _a.size();
		}
	private:
		std::list<ActionEditeur *> _a;
	};

	class RemplacerEntite : public ActionEditeur {
	public:
		RemplacerEntite(ElementEditeur const *remplacant, ElementEditeur const *remplace, index_t x, index_t y, Niveau::couche_t c) : _remplacant(0), _remplace(0), _x(x), _y(y), _c(c) {
			if(remplacant)
				_remplacant = new ElementEditeur(*remplacant);
			if(remplace)
				_remplace = new ElementEditeur(*remplace);
			
			this->definirPosition(x, y);
		}
		
		~RemplacerEntite() {
			delete _remplacant;
			delete _remplace;
		}
		
		void operator()();
		ActionEditeur *oppose() const {
			return new RemplacerEntite(_remplace, _remplacant, _x, _y, _c);
		}
		
		ElementEditeur *remplacant() {
			return _remplacant;
		}
		
		ElementEditeur *remplace() {
			return _remplace;
		}
		
	private:
		ElementEditeur *_remplacant;
		ElementEditeur *_remplace;
		index_t _x, _y;
		Niveau::couche_t _c;
	};

	class RedimensionnerNiveau : public ActionEditeur {
	public:
		RedimensionnerNiveau(size_t dimX, size_t dimY);
		
		~RedimensionnerNiveau() {
			for(std::list<ActionEditeur *>::const_iterator i = _cases.begin(); i != _cases.end(); ++i) {
				delete *i;
			}
		}
		
		void operator()();
		ActionEditeur *oppose() const {
			std::list<ActionEditeur *> l;
			for(std::list<ActionEditeur *>::const_iterator i = _cases.begin(); i != _cases.end(); ++i) {
				l.push_back((*i)->oppose());
			}
			return new RedimensionnerNiveau(_aDX, _aDY, l);
		}
	private:		
		RedimensionnerNiveau(size_t dimX, size_t dimY, std::list<ActionEditeur *> remplacements);
		std::list<ActionEditeur *> _cases;
		index_t _dX, _dY;
		index_t _aDX, _aDY;
	};
	
	class AjouterLoiProba : public ActionEditeur {
	public:
		AjouterLoiProba(index_t pos, LoiProba const &loi) : _pos(pos), _loi(loi) {
			
		}
		
		~AjouterLoiProba() {
			
		}
		
		void operator()() {
			Editeur::editeur()->_niveau->_probas.insert(Editeur::editeur()->_niveau->_probas.begin() + _pos, _loi);
		}
		
		ActionEditeur *oppose() const {
			return new SupprimerLoiProba(_pos, _loi);
		}
	private:
		LoiProba _loi;
		index_t _pos;
	};

	class SupprimerLoiProba : public ActionEditeur {
	public:
		SupprimerLoiProba(index_t pos, LoiProba const &loi) : _pos(pos), _loi(loi) {
			
		}
		
		~SupprimerLoiProba() {
			
		}
		
		void operator()() {
			Editeur::editeur()->_niveau->_probas.erase(Editeur::editeur()->_niveau->_probas.begin() + _pos);
		}
		
		ActionEditeur *oppose() const {
			return new AjouterLoiProba(_pos, _loi);
		}
	private:
		LoiProba _loi;
		index_t _pos;
	};

	class ModifierLoiProba : public ActionEditeur {
	public:
		ModifierLoiProba(index_t pos, LoiProba const &originale, LoiProba const &nouvelle) : _pos(pos), _originale(originale), _nouvelle(nouvelle) {
			
		}
		
		~ModifierLoiProba() {
			
		}
		
		void operator()() {
			Editeur::editeur()->_niveau->_probas[_pos] = _nouvelle;
		}
		
		ActionEditeur *oppose() const {
			return new ModifierLoiProba(_pos, _originale, _nouvelle);
		}
	private:
		index_t _pos;
		LoiProba _originale, _nouvelle;
	};

	//friend class Editeur::RedimensionnerNiveau;

protected:
	Editeur();
	void editerNiveau(std::string const &fichier);
	
	void enregistrer();
	void recharger();
	
	void outilAnnuler();
	void outilRetablir();
	void annuler();
	void retablir();
	void posterAction(ActionEditeur *a);
	void reinitialiserActions();
	
	void outilCopier();
	void outilColler();
	void outilSelection();
	void outilRecharger();
	void copier();
	void coller(index_t pX, index_t pY);

	void presenter(index_t x, index_t y);
	void afficher();
	void afficherCouche(Niveau::couche_t couche);
	void afficherGrille(unsigned char opacite);
	void afficherInterface();
	void afficherInventaire();
	void afficherControles();
	void afficherCarte();
	
	void demandeEnregistrement(Image const &fond);
	
	void sourisInventaire();
	void sourisControles();
	void sourisEditeur();
	void sourisCarte();
	
	Rectangle const &cadreInventaire() const;
	Rectangle const &cadreControles() const;
	Rectangle const &cadreEditeur() const;
	Rectangle const &cadreCarte() const;
	
	void modifCouche();
	void modifProba();
	void modifIndexProba();
	void modifIndex();
	void modifCategorie();
	void modifDimensions();
	
	void modifLoisProbas();
	void editerLoiProba(index_t loi, Image &fond);
	
	void selectionnerSemblables();
		
private:
	bool _continuer;
	bool _modifie;
	Image _sauve;
	Image _select;
	Image _copier;
	Image _coller;
	Image _recharger;
	Image _annuler;
	Image _retablir;
	
	NiveauEditeur *_niveau;
	Coordonnees _origine;
	Niveau::couche_t _coucheEdition;
	Rectangle const *_ancienRectangle;
	Rectangle _affichageSelection;
	Rectangle _cadreSelection;
	
	listeFonctions_t _fonctionsControles;
	listeFonctions_t _fonctionsInventaire;
	selection_t _selection;
	
	bool _aProba;
	index_t _aIndexProba;
	ElementNiveau::elementNiveau_t _aCat;
	index_t _aIndex;
	
	Outil _outil;
	std::vector<std::vector<ElementEditeur *> > _pressePapier;
	std::stack<ActionEditeur *> _pileAnnulations;
	std::stack<ActionEditeur *> _pileRetablissements;

	static Rectangle _cadreEditeur;
	static Rectangle _cadreControles;
	static Rectangle _cadreInventaire;
	static Rectangle _cadreCarte;
	
	static void initCadres();
	
	static Editeur *_editeur;
};

#endif
