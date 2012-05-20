//
//  Parametres.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 13/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Parametres.h"
#include "Image.h"
#include "Ecran.h"
#include "Menu.h"
#include "Texte.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include "Shader.h"
#include "tinyxml.h"
#include "Audio.h"

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
	
	void video(Image const &fond, Shader const &s);
	void audio(Image const &fond, Shader const &s);
	void controles(Image const &fond, Shader const &s);
	void definirEvenementAction(action_t action, Session::evenement_t e);
	void definirVolumeMusique(float v);
	void definirVolumeEffets(float v);
	
	char const *transcriptionAction(action_t);
	char const *nomBalise(action_t);
	
	char const *_shadVolume = "_volume";
	char const *_shadCadre = "_cadre";
	
	struct etiquetteTexte_t {
		etiquetteTexte_t(Unichar const &t);
		
		Texte _texte;
		Rectangle _cadre;
	};
	
	void afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<etiquetteTexte_t> &colonne1, index_t sel1, std::vector<etiquetteTexte_t> &colonne2, index_t sel2);
	void gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, index_t &premier, index_t nbAffiches, index_t &selection, bool &enSelection, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2);
	
	Session::evenement_t _evenementsAction[nbActions];
	float _volumeMusique;
	float _volumeEffets;
	
	Audio::audio_t _sonEffets = 0;

	struct hauteurTexte_t {
		hauteurTexte_t(dimension_t ecart) : _ecart(ecart) {
			
		}
		
		dimension_t operator()(dimension_t somme, Parametres::etiquetteTexte_t const &t) {
			return somme + t._texte.dimensions().y + _ecart;
		}
		
		dimension_t _ecart;
	};
	
	struct largeurTexte_t {
		bool operator()(Parametres::etiquetteTexte_t const &t1, Parametres::etiquetteTexte_t const &t2) {
			return t1._texte.dimensions().x < t2._texte.dimensions().x;
		}
	};
	
	struct afficheurTexte_t {
		afficheurTexte_t(Rectangle &cadre, dimension_t ecart) : _cadre(cadre), _ecart(ecart) {
			
		}
		
		void operator()(Parametres::etiquetteTexte_t const &t) {
			t._texte.afficher(_cadre.origine());
			_cadre.hauteur = t._texte.dimensions().y;
			_cadre.haut += _cadre.hauteur + _ecart;
		}
		
		Rectangle &_cadre;
		dimension_t _ecart;
	};
	
	struct initCadres_t {
		initCadres_t(Rectangle &cadre, dimension_t ecart) : _cadre(cadre), _ecart(ecart) {
			
		}
		
		void operator()(Parametres::etiquetteTexte_t &t) {
			_cadre.hauteur = t._texte.dimensions().y;
			t._cadre = _cadre;
			_cadre.haut += _cadre.hauteur + _ecart;
		}
		
		Rectangle &_cadre;
		dimension_t _ecart;
	};
	
	struct trouveSouris_t {
		bool operator()(Parametres::etiquetteTexte_t const &e) {
			return Session::souris() < e._cadre;
		}
	};
}

void Parametres::charger() {
	for(action_t a = premiereAction; a != nbActions; ++a)
		_evenementsAction[a] = Session::aucunEvenement;
	
	_evenementsAction[depBas] = Session::T_BAS;
	_evenementsAction[depDroite] = Session::T_DROITE;
	_evenementsAction[depGauche] = Session::T_GAUCHE;
	_evenementsAction[depHaut] = Session::T_HAUT;
	
	_evenementsAction[afficherInventaire] = Session::T_i;
	_evenementsAction[afficherJournal] = Session::T_TAB;
	_evenementsAction[ramasserObjet] = Session::T_ENTREE;
	_evenementsAction[interagir] = Session::T_ESPACE;

	_evenementsAction[remplirVie] = Session::T_h;

	_evenementsAction[sort1] = Session::T_a;
	_evenementsAction[sort2] = Session::T_z;
	_evenementsAction[sort3] = Session::T_e;
	_evenementsAction[sort4] = Session::T_r;
	
	for(action_t a = premiereAction; a != nbActions; ++a) {
		if(_evenementsAction[a] == Session::aucunEvenement)
			std::cout << "Aucune valeur par défaut pour l'événement " << + transcriptionAction(a) << std::endl;
	}
	
	_volumeEffets = 1.0f;
	_volumeMusique = 1.0f;
	
	TiXmlDocument document(Session::cheminRessources() + "config.xml");
	if(document.LoadFile()) {
		TiXmlElement *config = document.FirstChildElement("Config");
		if(config) {
			TiXmlElement *clavier = config->FirstChildElement("Clavier");
			for(action_t a = premiereAction; clavier && a != nbActions; ++a) {
				TiXmlElement *action = clavier->FirstChildElement(Parametres::nomBalise(a));
				if(!action)
					continue;
				char const *evenement = action->Attribute("valeur");
				for(Session::evenement_t e = Session::PREMIER_EVENEMENT_CLAVIER; e <= Session::DERNIER_EVENEMENT_CLAVIER; ++e) {
					std::string ee = Session::transcriptionEvenement(e);
					if(ee == evenement) {
						_evenementsAction[a] = e;
						break;
					}
				}
			}
			
			if(TiXmlElement *audio = config->FirstChildElement("Audio")) {
				double val;
				if(audio->Attribute("volumeEffets")) {
					audio->Attribute("volumeEffets", &val);
					_volumeEffets = val;
				}
				if(audio->Attribute("volumeMusique")) {
					audio->Attribute("volumeMusique", &val);
					_volumeMusique = val;
				}
			}
		}
	}
	
	_sonEffets = Audio::chargerSon(Session::cheminRessources() + "testEffet.wav");
}

void Parametres::enregistrer() {
	std::string valeurTexte;
	char const *documentBase = 
	"<?xml version=\"1.0\" standalone='no' >\n"
	"<Config>\n"
	"<Clavier>\n"
	"</Clavier>\n"
	"<Audio>\n"
	"</Audio>\n"
	"</Config>";
	TiXmlDocument *document = new TiXmlDocument(Session::cheminRessources() + "config.xml");
	document->Parse(documentBase);
	
	TiXmlElement *config = document->FirstChildElement("Config");
	TiXmlElement *clavier = config->FirstChildElement("Clavier");
	for(action_t a = premiereAction; a != nbActions; ++a) {
		TiXmlElement action(Parametres::nomBalise(a));
		action.SetAttribute("valeur", Session::transcriptionEvenement(Parametres::evenementAction(a)));
		clavier->InsertEndChild(action);
	}
	
	TiXmlElement *audio = config->FirstChildElement("Audio");
	audio->SetAttribute("volumeEffets", Parametres::volumeEffets());
	audio->SetAttribute("volumeMusique", Parametres::volumeMusique());

	if(!document->SaveFile())
		std::cout << "L'enregistrement du fichier de paramètres " << document->Value() << " a échoué." << std::endl;
}

void Parametres::nettoyer() {
	Parametres::enregistrer();
	Audio::libererSon(_sonEffets);
}

Session::evenement_t Parametres::evenementAction(action_t action) {
	return _evenementsAction[action];
}

void Parametres::definirEvenementAction(action_t action, Session::evenement_t e) {
	for(action_t a = premiereAction; a != nbActions; ++a) {
		if(a == action)
			continue;
		if(_evenementsAction[a] == e)
			_evenementsAction[a] = Session::aucunEvenement;
	}
	
	_evenementsAction[action] = e;
}

float Parametres::volumeMusique() {
	return _volumeMusique;
}

float Parametres::volumeEffets() {
	return _volumeEffets;
}

void Parametres::definirVolumeMusique(float v) {
	_volumeMusique = std::min(1.0f, std::max(0.0f, v));
}

void Parametres::definirVolumeEffets(float v) {
	_volumeEffets = std::min(1.0f, std::max(0.0f, v));
}

void Parametres::editerParametres(Image const &fond, Shader const &s) {
	std::vector<Unichar> elements;
	elements.push_back("Réglages contrôles");
	elements.push_back("Réglages audios");
	elements.push_back("Réglages vidéos");
	Menu menu("Réglages", elements);
	
	index_t selection = 0;
	do {
		selection = menu.afficher(selection, fond, s);
		switch(selection) {
			case 2:
				Parametres::video(fond, s);
				break;
			case 1:
				Parametres::audio(fond, s);
				break;
			case 0:
				Parametres::controles(fond, s);
				break;
		}
	} while(selection != elements.size());
}

void Parametres::video(Image const &fond, Shader const &s) {
	std::cout << "vidéo" << std::endl;
}

void Parametres::audio(Image const &fond, Shader const &s) {
	std::vector<etiquetteTexte_t> texte, vide;
	texte.push_back(Unichar("Volume musique"));
	texte.push_back(Unichar("Volume effets"));
	Unichar blanc("                                       ");
	vide.push_back(blanc);
	vide.push_back(blanc);
	
	Shader shader(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "volume.frag");
	
	bool continuer = true;
	index_t premier = 0;
	size_t nbAffiches = 2;
	
	index_t selection = -1;
	
	horloge_t ancienSon = 0;
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		Parametres::afficherColonnes("Audio", premier, nbAffiches, false, texte, -1, vide, -1);
		
		float *valeur[2] = {&_volumeMusique, &_volumeEffets};
		shader.activer();
		for(int i = 0; i < 2; ++i) {
			Rectangle const &r = vide[i]._cadre;
			shader.definirParametre(_shadVolume, *(valeur[i]));
			Rectangle s = Shader::versShader(r);
			shader.definirParametre(_shadCadre, s.gauche, s.haut, s.largeur, s.hauteur);

			Ecran::afficherTriangle(r.origine() + Coordonnees(0, r.hauteur), r.origine() + r.dimensions(), r.origine() + Coordonnees(r.largeur, 0), Couleur::blanc);
		}
		Shader::desactiver();
		
		Ecran::finaliser();
		
		if(!Session::evenement(Session::B_GAUCHE)) {
			selection = -1;
		}

		if(selection == -1) {
			if(Session::evenement(Session::B_GAUCHE)) {
				std::vector<etiquetteTexte_t>::iterator i = std::find_if(vide.begin(), vide.end(), trouveSouris_t());
				if(i != vide.end())
					selection = std::distance(vide.begin(), i);
			}
		}
		else {
			*(valeur[selection]) = std::min(1.0f, std::max<float>(0.0f, (Session::souris().x - vide[selection]._cadre.gauche) / vide[selection]._cadre.largeur));
			Audio::definirVolumeMusique(_volumeMusique);
			Audio::definirVolumeEffets(_volumeEffets);
			if(selection == 1 && horloge() - ancienSon > 0.2) {
				ancienSon = horloge();
				Audio::jouerSon(_sonEffets);
			}
		}
		if(Session::evenement(Session::T_ESC))
			continuer = false;
		
		Ecran::maj();
	}
}

void Parametres::controles(Image const &fond, Shader const &s) {
	std::vector<etiquetteTexte_t> actions, evenements;

	for(action_t i = premiereAction; i != nbActions; ++i) {
		actions.push_back(Unichar(transcriptionAction(i)));
		evenements.push_back(Session::transcriptionEvenement(Parametres::evenementAction(i)));
	}
	
	bool continuer = true, modification = false;
	index_t selection = 0, premier = 0;
	size_t nbAffiches = std::min<size_t>(actions.size(), 8);
	horloge_t ancienDefilement = horloge();
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		Parametres::afficherColonnes("Contrôles", premier, nbAffiches, modification, actions, -1, evenements, selection);
		Ecran::finaliser();
		
		for(action_t i = premiereAction; i != nbActions; ++i) {
			evenements[i]._texte.definir(Session::transcriptionEvenement(_evenementsAction[i]));
		}

		if(!modification) {
			Parametres::gestionEvenementsAfficheur(ancienDefilement, continuer, premier, nbAffiches, selection, modification, actions, evenements);
		}
		else {
			if(Session::evenement(Session::T_ESC) || Session::evenement(Session::T_ENTREE)) {
				Session::reinitialiser(Session::T_ESC);
				Session::reinitialiser(Session::T_ENTREE);
				modification = false;
			}
			else {
				for(Session::evenement_t e = Session::PREMIER_EVENEMENT_CLAVIER; e <= Session::DERNIER_EVENEMENT_CLAVIER; ++e) {
					if(Session::evenement(e)) {
						Parametres::definirEvenementAction(static_cast<action_t>(selection), e);
						modification = false;
						break;
					}
				}
			}
		}
				
		Ecran::maj();
	}
}

Parametres::etiquetteTexte_t::etiquetteTexte_t(Unichar const &t) : _texte(t, POLICE_DECO, 20 * Ecran::echelleMin(), Couleur::blanc) {

}

void Parametres::afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<Parametres::etiquetteTexte_t> &colonne1, index_t sel1, std::vector<Parametres::etiquetteTexte_t> &colonne2, index_t sel2) {
	static float teinteSelection = 0.0f;
	static int sensTeinte = 1;
	if(!modif) {
		teinteSelection = 0.0f;
		sensTeinte = 1;
	}
	else {
		teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * sensTeinte;
		if(teinteSelection > 0.7f) {
			teinteSelection = 0.7f;
			sensTeinte = -1;
		}
		else if(teinteSelection < 0) {
			teinteSelection = 0;
			sensTeinte = 1;
		}
	}
	
	if(modif) {
		if(sel1 >= 0)
			colonne1[sel1]._texte.definir(Couleur(teinteSelection * 255));
		if(sel2 >= 0)
			colonne2[sel2]._texte.definir(Couleur(teinteSelection * 255));
	}
	else {
		if(sel1 >= 0)
			colonne1[sel1]._texte.definir(Couleur::noir);
		if(sel2 >= 0)
			colonne2[sel2]._texte.definir(Couleur::noir);
	}
	
	bool premierAffiche = premier == 0;
	if(!premierAffiche)
		--nb;
	bool dernierAffiche = premier + nb == colonne1.size();
	if(!dernierAffiche)
		--nb;
	
	Texte tt(titre, POLICE_DECO, 40 * Ecran::echelleMin(), Couleur::blanc);
	Texte points("...", POLICE_DECO, 20 * Ecran::echelleMin(), Couleur::blanc);
	
	Rectangle cadre(Coordonnees(80, 80) * Ecran::echelleMin(), tt.dimensions());
	tt.afficher(cadre.origine());
	
	cadre.haut += cadre.hauteur + 20 * Ecran::echelleMin();
	cadre.gauche += 20 * Ecran::echelleMin();
	
	dimension_t largeur1 = std::max_element(colonne1.begin(), colonne1.end(), largeurTexte_t())->_texte.dimensions().x, largeur2 = -40 * Ecran::echelleMin();
	if(colonne2.size())
		largeur2 = std::max_element(colonne2.begin(), colonne2.end(), largeurTexte_t())->_texte.dimensions().x;
	
	dimension_t hauteur = std::accumulate(colonne1.begin() + premier, colonne1.begin() + premier + nb, dimension_t(0), hauteurTexte_t(10 * Ecran::echelleMin())) - 10 * Ecran::echelleMin();
	if(!premierAffiche) {
		hauteur += points.dimensions().y + 10 * Ecran::echelleMin();
	}
	if(!dernierAffiche)
		hauteur += points.dimensions().y + 10 * Ecran::echelleMin();
	
	Ecran::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeur1 + largeur2 + 20 * Ecran::echelleMin() + 40 * Ecran::echelleMin(), hauteur + 20 * Ecran::echelleMin()), Couleur(200, 205, 220, 128));
	
	cadre += Coordonnees(10, 10) * Ecran::echelleMin();

	if(!premierAffiche) {
		points.afficher(cadre.origine() + Coordonnees((largeur1 + largeur2 + 40 * Ecran::echelleMin() - points.dimensions().x) / 2, 0));
		cadre.haut += points.dimensions().y + 10 * Ecran::echelleMin();
	}

	coordonnee_t sauveHaut = cadre.haut;
	afficheurTexte_t afficheur(cadre, 10 * Ecran::echelleMin());
	for_each(colonne1.begin() + premier, colonne1.begin() + premier + nb, afficheur);
	cadre.haut = sauveHaut;
	cadre.gauche += largeur1 + 40 * Ecran::echelleMin();
	if(colonne2.size())
		for_each(colonne2.begin() + premier, colonne2.begin() + premier + nb, afficheur);
	if(sel1 >= 0)
		colonne1[sel1]._texte.definir(Couleur::blanc);
	if(sel2 >= 0)
		colonne2[sel2]._texte.definir(Couleur::blanc);
	
	cadre.gauche -= largeur1 + 40 * Ecran::echelleMin();
	cadre.largeur = largeur1 + 40 * Ecran::echelleMin();
	cadre.haut = sauveHaut;
	initCadres_t initCadres(cadre, 10 * Ecran::echelleMin());
	for_each(colonne1.begin() + premier, colonne1.begin() + premier + nb, initCadres);
	cadre.largeur = largeur2;
	cadre.haut = sauveHaut;
	cadre.gauche += largeur1 + 40 * Ecran::echelleMin();
	for_each(colonne2.begin() + premier, colonne2.begin() + premier + nb, initCadres);
	
	cadre.gauche -= largeur1 + 40 * Ecran::echelleMin();
	if(!dernierAffiche) {
		cadre += Coordonnees(10, 10) * Ecran::echelleMin();
		points.afficher(cadre.origine() + Coordonnees((largeur1 + largeur2 + 40 * Ecran::echelleMin() - points.dimensions().x) / 2, 0));
		cadre.haut += points.dimensions().y + 10 * Ecran::echelleMin();
	}
	cadre -= Coordonnees(10, -10) * Ecran::echelleMin();
}

void Parametres::gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, index_t &premier, index_t nbAffiches, index_t &selection, bool &modification, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2) {
	bool premierAffiche = premier == 0;
	if(!premierAffiche)
		--nbAffiches;
	bool dernierAffiche = premier + nbAffiches == colonne1.size();
	if(!dernierAffiche)
		--nbAffiches;

	if(Session::evenement(Session::T_ESC)) {
		continuer = false;
	}
	else if(Session::evenement(Session::T_ENTREE)) {
		modification = true;
		Session::reinitialiser(Session::T_ENTREE);
	}
	else if(Session::evenement(Session::B_GAUCHE)) {
		std::vector<etiquetteTexte_t>::const_iterator souris = std::find_if(colonne1.begin(), colonne1.end(), trouveSouris_t());
		if(souris != colonne1.end()) {
			selection = std::distance(colonne1.begin(), souris);
			modification = true;
		}
		else  {
			souris = std::find_if(colonne2.begin(), colonne2.end(), trouveSouris_t());
			if(souris != colonne2.end()) {
				selection = std::distance(colonne2.begin(), souris);
				modification = true;
			}
		}
	}

	else if(Session::evenement(Session::T_HAUT) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
		if(selection > 0) {
			--selection;
			if(selection < premier)
				--premier;
		}
		else {
			selection = colonne1.size() - 1;
			if(colonne1.size() > nbAffiches)
				premier = colonne1.size() - nbAffiches;
		}
		
		ancienDefilement = horloge();
	}
	else if(Session::evenement(Session::T_BAS) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
		if(selection < colonne1.size() - 1) {
			++selection;
			if(selection >= premier + nbAffiches)
				++premier;
			if(premier != 0 && premierAffiche && premier + 1 + nbAffiches != colonne1.size()) {
				++premier;
			}
		}
		else {
			selection = 0;
			premier = 0;
		}
		
		ancienDefilement = horloge();
	}
	else if(Session::evenement(Session::SOURIS)) {
		std::vector<etiquetteTexte_t>::const_iterator souris = std::find_if(colonne1.begin(), colonne1.end(), trouveSouris_t());
		if(souris != colonne1.end()) {
			selection = std::distance(colonne1.begin(), souris);
		}
		else  {
			souris = std::find_if(colonne2.begin(), colonne2.end(), trouveSouris_t());
			if(souris != colonne2.end()) {
				selection = std::distance(colonne2.begin(), souris);
			}
		}
	}
}

char const *Parametres::transcriptionAction(action_t a) {
	switch(a) {
		case depBas:
			return "Bas";			
		case depHaut:
			return "Haut";
		case depDroite:
			return "Droite";
		case depGauche:
			return "Gauche";
		case afficherInventaire:
			return "Afficher/masquer l'inventaire";
		case ramasserObjet:
			return "Ramasser l'objet";
		case interagir:
			return "Interagir";
		case remplirVie:
			return "Potion de vie";
		case sort1:
			return "Sort 1";
		case sort2:
			return "Sort 2";
		case sort3:
			return "Sort 3";
		case sort4:
			return "Sort 4";
		case afficherJournal:
			return "Afficher/masquer le journal";
		case nbActions:
			return 0;
	}
}

char const *Parametres::nomBalise(action_t a) {
	switch(a) {
		case depBas:
			return "Bas";			
		case depHaut:
			return "Haut";
		case depDroite:
			return "Droite";
		case depGauche:
			return "Gauche";
		case afficherInventaire:
			return "AfficherMasquerInventaire";
		case ramasserObjet:
			return "RamasserObjet";
		case interagir:
			return "Interagir";
		case remplirVie:
			return "PotionVie";
		case sort1:
			return "Sort1";
		case sort2:
			return "Sort2";
		case sort3:
			return "Sort3";
		case sort4:
			return "Sort4";
		case afficherJournal:
			return "AfficherMasquerJournal";
		case nbActions:
			return 0;
	}
}
