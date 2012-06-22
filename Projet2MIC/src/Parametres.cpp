//
//  Parametres.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
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
	
	std::string transcriptionResolution(std::pair<int, int> const &r);
	Rectangle afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<etiquetteTexte_t> &colonne1, index_t sel1, std::vector<etiquetteTexte_t> &colonne2, index_t sel2, coordonnee_t abscisse = 80);
	void gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, index_t &premier, index_t nbAffiches, index_t &selection, bool &enSelection, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2);
	
	Session::evenement_t _evenementsAction[nbActions];
	float _volumeMusique;
	float _volumeEffets;
	
	Audio::audio_t _sonEffets = 0;
	
	int _largeurEcran;
	int _hauteurEcran;
	bool _pleinEcran;

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
	_evenementsAction[interagir] = Session::T_ESPACE;

	_evenementsAction[remplirVie] = Session::T_h;
	
	for(action_t a = premiereAction; a != nbActions; ++a) {
		if(_evenementsAction[a] == Session::aucunEvenement)
			std::cout << "Aucune valeur par défaut pour l'événement " << + Parametres::transcriptionAction(a) << std::endl;
	}
	
	_volumeEffets = 1.0f;
	_volumeMusique = 1.0f;
	
	_largeurEcran = 800;
	_hauteurEcran = 600;
	_pleinEcran = false;

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
			
			if(TiXmlElement *video = config->FirstChildElement("Video")) {
				if(video->Attribute("largeurEcran")) {
					video->Attribute("largeurEcran", &_largeurEcran);
				}
				if(video->Attribute("hauteurEcran")) {
					video->Attribute("hauteurEcran", &_hauteurEcran);
				}
				if(video->Attribute("pleinEcran")) {
					int val;
					video->Attribute("pleinEcran", &val);
					_pleinEcran = val;
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
	"<Video>\n"
	"</Video>\n"
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

	TiXmlElement *video = config->FirstChildElement("Video");
	video->SetAttribute("largeurEcran", Parametres::largeurEcran());
	video->SetAttribute("hauteurEcran", Parametres::hauteurEcran());
	video->SetAttribute("pleinEcran", Parametres::pleinEcran());

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

int Parametres::largeurEcran() {
	return _largeurEcran;
}

int Parametres::hauteurEcran() {
	return _hauteurEcran;
}

bool Parametres::pleinEcran() {
	return _pleinEcran;
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

std::string Parametres::transcriptionResolution(std::pair<int, int> const &r) {
	return nombreVersTexte(r.first) + "*" + nombreVersTexte(r.second);
}

void Parametres::video(Image const &fond, Shader const &s) {		
	std::vector<std::pair<int, int> > resolutions[2];
	{
		std::list<std::pair<int, int> > r = Ecran::resolutionsDisponibles(false);
		resolutions[0].assign(r.begin(), r.end());
		
		r = Ecran::resolutionsDisponibles(true);
		resolutions[1].assign(r.begin(), r.end());
	}
	
	bool pleinEcran = Ecran::pleinEcran();
	int resolutionActuelle = std::find(resolutions[pleinEcran].begin(), resolutions[pleinEcran].end(), std::make_pair(Ecran::largeur(), Ecran::hauteur())) - resolutions[pleinEcran].begin();
	
	char const *txt[2] = {"Désactivé", "Activé"};
	
	std::vector<etiquetteTexte_t> enTetes, champs;

	enTetes.push_back(Unichar("Plein écran :"));
	champs.push_back(Unichar());
	
	enTetes.push_back(Unichar("Résolution :"));
	champs.push_back(Unichar());
	
	Texte valider("Valider",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	Texte annuler("Annuler",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	
	Rectangle cadreAnnuler, cadreValider;
	
	bool continuer = true, modification = false;
	index_t selection = 0, premier = 0;
	size_t nbAffiches = std::min<size_t>(enTetes.size(), 8);
	horloge_t ancienDefilement = horloge();
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		champs[0]._texte.definir(Unichar(txt[pleinEcran]));
		
		champs[1]._texte.definir(Unichar(Parametres::transcriptionResolution(resolutions[pleinEcran][resolutionActuelle])));

		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));
		
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		s.definirParametre(Shader::tempsAbsolu, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		Rectangle cadre = Parametres::afficherColonnes("Vidéo", premier, nbAffiches, modification, enTetes, -1, champs, selection);
		
		valider.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		annuler.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		
		cadreValider.definirDimensions(valider.dimensions()); 
		cadreAnnuler.definirDimensions(annuler.dimensions()); 
		cadreValider.definirOrigine(cadre.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur)); 
		cadreAnnuler.definirOrigine(cadreValider.origine() + Coordonnees(80 * Ecran::echelleMin() + cadreValider.largeur, 0)); 

		valider.afficher(cadreValider.origine());
		annuler.afficher(cadreAnnuler.origine());

		Ecran::finaliser();
		
		if(!modification) {
			Parametres::gestionEvenementsAfficheur(ancienDefilement, continuer, premier, nbAffiches, selection, modification, enTetes, champs);
			if(Session::evenement(Session::B_GAUCHE)) {
				if(Session::souris() < cadreValider) {
					_largeurEcran = resolutions[pleinEcran][resolutionActuelle].first;
					_hauteurEcran = resolutions[pleinEcran][resolutionActuelle].second;
					_pleinEcran = pleinEcran;
					
					Ecran::modifierResolution(_largeurEcran, _hauteurEcran, _pleinEcran);
					continuer = false;
					break;
				}
				else if(Session::souris() < cadreAnnuler) {
					continuer = false;
					break;
				}
			}
		}
		else {
			if(Session::evenement(Session::T_ESC) || Session::evenement(Session::T_ENTREE)) {
				Session::reinitialiser(Session::T_ESC);
				Session::reinitialiser(Session::T_ENTREE);
				modification = false;
			}
			else {
				if(selection == 0) {
					if(Session::evenement(Session::T_GAUCHE) || Session::evenement(Session::T_DROITE)) {
						Session::reinitialiser(Session::T_GAUCHE);
						Session::reinitialiser(Session::T_DROITE);
						pleinEcran = !pleinEcran;
						resolutionActuelle = 0;
					}
				}
				else {
					int dep = 0;
					if(Session::evenement(Session::T_GAUCHE))
						dep = -1;
					else if(Session::evenement(Session::T_DROITE))
						dep = 1;
					if(dep) {
						resolutionActuelle += dep;
						if(resolutionActuelle < 0)
							resolutionActuelle = resolutions[pleinEcran].size() - 1;
						else if(resolutionActuelle >= resolutions[pleinEcran].size())
							resolutionActuelle = 0;
						
						Session::reinitialiser(Session::T_GAUCHE);
						Session::reinitialiser(Session::T_DROITE);
					}
				}
			}
		}
		
		Ecran::maj();
	}
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
		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));

		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		s.definirParametre(Shader::tempsAbsolu, horloge());
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
		actions.push_back(Unichar(Parametres::transcriptionAction(i)));
		evenements.push_back(Session::transcriptionEvenement(Parametres::evenementAction(i)));
	}
	
	bool continuer = true, modification = false;
	index_t selection = 0, premier = 0;
	size_t nbAffiches = std::min<size_t>(actions.size(), 8);
	horloge_t ancienDefilement = horloge();
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));

		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		s.definirParametre(Shader::tempsAbsolu, horloge());
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

Rectangle Parametres::afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<Parametres::etiquetteTexte_t> &colonne1, index_t sel1, std::vector<Parametres::etiquetteTexte_t> &colonne2, index_t sel2, coordonnee_t abscisse) {
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
	
	Rectangle cadre(Coordonnees(abscisse, 80) * Ecran::echelleMin(), tt.dimensions());
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
	
	Rectangle total = Rectangle(cadre.gauche, cadre.haut, largeur1 + largeur2 + 20 * Ecran::echelleMin() + 40 * Ecran::echelleMin(), hauteur + 20 * Ecran::echelleMin());
	Ecran::afficherRectangle(total, Couleur(200, 205, 220, 128));
	
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
	if(colonne2.size())
		for_each(colonne2.begin() + premier, colonne2.begin() + premier + nb, initCadres);
	
	cadre.gauche -= largeur1 + 40 * Ecran::echelleMin();
	if(!dernierAffiche) {
		cadre += Coordonnees(10, 10) * Ecran::echelleMin();
		points.afficher(cadre.origine() + Coordonnees((largeur1 + largeur2 + 40 * Ecran::echelleMin() - points.dimensions().x) / 2, 0));
		cadre.haut += points.dimensions().y + 10 * Ecran::echelleMin();
	}
	cadre -= Coordonnees(10, -10) * Ecran::echelleMin();
	
	return total;
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
		case interagir:
			return "Interagir/Ramasser";
		case remplirVie:
			return "Potion de vie";
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
		case interagir:
			return "Interagir";
		case remplirVie:
			return "PotionVie";
		case nbActions:
			return 0;
	}
}

void Parametres::afficherCredits(Image const &fond, Shader const &s) {
	std::vector<etiquetteTexte_t> noms, roles;
	
	noms.push_back(Unichar("Marc Promé"));
	roles.push_back(Unichar("Level design, graphismes, programmation"));

	noms.push_back(Unichar("Rémi Saurel"));
	roles.push_back(Unichar("Level design, graphismes, programmation"));
	
	noms.push_back(Unichar("OpenGameArt.org"));
	roles.push_back(Unichar("Graphismes"));
	
	noms.push_back(Unichar("Semaphore (Newgrounds)"));
	roles.push_back(Unichar("Musique \"Dream of Water and Land\""));
	
	noms.push_back(Unichar("Sexual-Lobster (Newgrounds)"));
	roles.push_back(Unichar("Musique \"Man vs. Walrus-Man\""));

	noms.push_back(Unichar("Xerferic (Newgrounds)"));
	roles.push_back(Unichar("Musique \"Laid-Back\""));
	
	for(int i = 0; i < noms.size(); ++i) {
		noms[i]._texte.definir(POLICE_DECO, 16 * Ecran::echelleMin());
		roles[i]._texte.definir(POLICE_DECO, 16 * Ecran::echelleMin());
	}
	
	bool continuer = true;
	Texte ok("Retour",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	Rectangle cadreOk;
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));
		
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		s.definirParametre(Shader::tempsAbsolu, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		Rectangle cadre = Parametres::afficherColonnes("Crédits", 0, noms.size(), false, noms, -1, roles, -1, 40);
		
		cadreOk.definirDimensions(ok.dimensions()); 
		cadreOk.definirOrigine(cadre.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur)); 
		
		ok.afficher(cadreOk.origine());

		Ecran::finaliser();
		
		if(Session::evenement(Session::T_ESC) || (Session::evenement(Session::B_GAUCHE) && Session::souris() < cadreOk)) {
			continuer = false;
			Session::reinitialiser(Session::T_ESC);
			Session::reinitialiser(Session::B_GAUCHE);
		}
		
		Ecran::maj();
	}
}

