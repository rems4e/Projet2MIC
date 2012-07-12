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
namespace cURL {
#include <curl/curl.h>
}
#include "fonctions.h"
#include <sstream>

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
	
	void general(Image const &fond, Shader const &s);
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
		etiquetteTexte_t(Unichar const &t) : _texte(t, POLICE_DECO, 20 * Ecran::echelleMin(), Couleur::blanc), _flecheG(Rectangle::aucun), _flecheD(Rectangle::aucun) {
			
		}
		
		Texte _texte;
		Rectangle _cadre;
		
		Rectangle _flecheG;
		Rectangle _flecheD;
	};
	
	std::string transcriptionResolution(std::pair<int, int> const &r);
	Rectangle afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<etiquetteTexte_t> &colonne1, index_t sel1, std::vector<etiquetteTexte_t> &colonne2, index_t sel2, bool fleches, coordonnee_t abscisse = 80);
	void gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, int &dep, index_t &premier, index_t nbAffiches, index_t &selection, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2);
	
	Session::evenement_t _evenementsAction[nbActions];
	float _volumeMusique;
	float _volumeEffets;
	
	Audio::audio_t _sonEffets = 0;
	
	int _largeurEcran;
	int _hauteurEcran;
	bool _pleinEcran;
	bool _ips;
	bool _limiteIPS;
	bool _synchroVerticale;
	bool _rechercherMaj;
	Image *_flecheG = 0;
	Image *_flecheD = 0;

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
		afficheurTexte_t(Rectangle &cadre, dimension_t ecart) : _cadre(cadre), _ecart(ecart), _fleches(false) {
			
		}
		
		void operator()(Parametres::etiquetteTexte_t const &t) {
			if(_fleches) {
				Parametres::_flecheG->afficher(_cadre.origine() + Coordonnees(0, (_cadre.dimensions().y - Parametres::_flecheG->dimensions().y) / 2));
				t._texte.afficher(_cadre.origine() + Coordonnees(Parametres::_flecheG->dimensions().x + 10 * Ecran::echelleMin(), 0));
				Parametres::_flecheD->afficher(_cadre.origine() + Coordonnees(_cadre.dimensions().x - _flecheD->dimensions().x, (_cadre.dimensions().y - Parametres::_flecheD->dimensions().y) / 2));
			}
			else {
				t._texte.afficher(_cadre.origine());
			}
			_cadre.hauteur = t._texte.dimensions().y;
			_cadre.haut += _cadre.hauteur + _ecart;
		}
		
		void definirFleches(bool f) {
			_fleches = f;
		}
		
		Rectangle &_cadre;
		dimension_t _ecart;
		bool _fleches;
	};
	
	struct initCadres_t {
		initCadres_t(Rectangle &cadre, dimension_t ecart) : _cadre(cadre), _ecart(ecart), _fleches(false) {
			
		}
		
		void operator()(Parametres::etiquetteTexte_t &t) {
			_cadre.hauteur = t._texte.dimensions().y;
			t._cadre = _cadre;
			if(_fleches) {
				t._flecheG.definirOrigine(_cadre.origine());
				t._flecheG.definirDimensions(Coordonnees(Parametres::_flecheG->dimensions().x, std::max(Parametres::_flecheG->dimensions().y, _cadre.hauteur)));

				t._flecheD.definirOrigine(_cadre.origine() + _cadre.dimensions() - Parametres::_flecheD->dimensions());
				t._flecheD.definirDimensions(Coordonnees(Parametres::_flecheD->dimensions().x, std::max(Parametres::_flecheD->dimensions().y, _cadre.hauteur)));
}
			_cadre.haut += _cadre.hauteur + _ecart;
		}
		
		void definirFleches(bool f) {
			_fleches = f;
		}
		
		bool _fleches;
		Rectangle &_cadre;
		dimension_t _ecart;
	};
	
	struct trouveSouris_t {
		bool operator()(Parametres::etiquetteTexte_t const &e) {
			return Session::souris() < e._cadre;
		}
	};
}

static size_t ecrireDonnees(char *ptr, size_t size, size_t nmemb, void *userdata) {
	std::ostringstream *stream = (std::ostringstream *)userdata;
	size_t count = size * nmemb;
	stream->write(ptr, count);
	
	return count;
}

bool Parametres::majDisponible() {
	char const *fichierMaj = "https://etud.insa-toulouse.fr/~saurel/projet2MIC-version.txt";
	
	std::ostringstream data;
	cURL::CURL *session = cURL::curl_easy_init();
	cURL::curl_easy_setopt(session, cURL::CURLOPT_URL, fichierMaj);
	cURL::curl_easy_setopt(session,  cURL::CURLOPT_WRITEDATA, &data);
	cURL::curl_easy_setopt(session, cURL::CURLOPT_SSL_VERIFYPEER, false);
	cURL::curl_easy_setopt(session,  cURL::CURLOPT_WRITEFUNCTION, ecrireDonnees);
	cURL::CURLcode res = cURL::curl_easy_perform(session);
	std::string err = cURL::curl_easy_strerror(res);
	cURL::curl_easy_cleanup(session);

	if(res != cURL::CURLE_OK) {
		std::cerr << "Impossible de contacter le serveur de mise à jour : " << err << "." << std::endl;
		return false;
	}
	else {
		std::string val = data.str();
		std::vector<std::string> l;
		decouperChaine(val, "\n", l);
		
		if(l.size() == 3 && l[0] == "majProjet2MIC") {
			int maj = texteVersNombre(l[1]);
			int min = texteVersNombre(l[2]);
			if(maj > VERSION_MAJ || (maj == VERSION_MAJ && min > VERSION_MIN)) {
				return true;
			}
			
			return false;
		}
		else {
			return false;
		}
	}
}

char const *Parametres::URLMaj() {
	return "https://etud.insa-toulouse.fr/~saurel/projet2mic.html";
}

char const *Parametres::versionTexte() {
	static char dim[nombreChiffres<VERSION_MAJ>::nb + 1 + nombreChiffres<VERSION_MIN>::nb + 1] = {0};
	if(*dim == 0) {
		std::string version = nombreVersTexte(VERSION_MAJ) + "." + nombreVersTexte(VERSION_MIN);

		std::copy(version.begin(), version.end(), dim);
	}
	
	return dim;
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
	_pleinEcran = true;
	_ips = false;
	_limiteIPS = true;
	_synchroVerticale = true;
	_rechercherMaj = true;

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
				if(video->Attribute("imagesParSeconde")) {
					int val;
					video->Attribute("imagesParSeconde", &val);
					_ips = val;
				}
				if(video->Attribute("synchroVerticale")) {
					int val;
					video->Attribute("synchroVerticale", &val);
					_synchroVerticale = val;
				}
				if(video->Attribute("limiteIPS")) {
					int val;
					video->Attribute("limiteIPS", &val);
					_limiteIPS = val;
				}
			}
			
			if(TiXmlElement *general = config->FirstChildElement("General")) {
				if(general->Attribute("rechercherMaj")) {
					int val;
					general->Attribute("rechercherMaj", &val);
					_rechercherMaj = val;
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
	"<General>\n"
	"</General>\n"
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
	video->SetAttribute("imagesParSeconde", Parametres::ips());
	video->SetAttribute("synchroVerticale", Parametres::synchroVerticale());
	video->SetAttribute("limiteIPS", Parametres::limiteIPS());
	
	TiXmlElement *general = config->FirstChildElement("General");
	general->SetAttribute("rechercherMaj", Parametres::rechercherMaj());
	
	if(!document->SaveFile())
		std::cout << "L'enregistrement du fichier de paramètres " << document->Value() << " a échoué." << std::endl;
}

void Parametres::nettoyer() {
	Parametres::enregistrer();
	Audio::libererSon(_sonEffets);
	delete _flecheG;
	delete _flecheD;
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

bool Parametres::ips() {
	return _ips;
}

bool Parametres::limiteIPS() {
	return _limiteIPS;
}

bool Parametres::synchroVerticale() {
	return _synchroVerticale;
}

bool Parametres::rechercherMaj() {
	return _rechercherMaj;
}

void Parametres::definirVolumeMusique(float v) {
	_volumeMusique = std::min(1.0f, std::max(0.0f, v));
}

void Parametres::definirVolumeEffets(float v) {
	_volumeEffets = std::min(1.0f, std::max(0.0f, v));
}

void Parametres::editerParametres(Image const &fond, Shader const &s) {
	if(_flecheG == 0) {
		_flecheG = new Image(Session::cheminRessources() + "flecheG.png");
		_flecheD = new Image(Session::cheminRessources() + "flecheD.png");
	}
	
	std::vector<Unichar> elements;
	elements.push_back("Réglages contrôles");
	elements.push_back("Réglages audios");
	elements.push_back("Réglages vidéos");
	elements.push_back("Réglages généraux");
	Menu menu("Réglages", elements);
	
	index_t selection = 0;
	do {
		selection = menu.afficher(selection, fond, s);
		switch(selection) {
			case 3:
				Parametres::general(fond, s);
				break;
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

void Parametres::general(Image const &fond, Shader const &s) {
	bool maj = Parametres::rechercherMaj();
	
	char const *txt[2] = {"Désactivé", "Activé"};
	
	std::vector<etiquetteTexte_t> enTetes, champs;
	
	enTetes.push_back(Unichar("Recherche de mises à jour :"));
	champs.push_back(Unichar());
	
	Texte valider("Valider",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	Texte annuler("Annuler",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	
	Rectangle cadreAnnuler, cadreValider;
	
	bool continuer = true;
	index_t selection = 0, premier = 0;
	size_t nbAffiches = std::min<size_t>(enTetes.size(), 8);
	horloge_t ancienDefilement = horloge();
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		champs[0]._texte.definir(Unichar(txt[maj]));
		
		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));
		
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		s.definirParametre(Shader::tempsAbsolu, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		Rectangle cadre = Parametres::afficherColonnes("Vidéo", premier, nbAffiches, false, enTetes, -1, champs, selection, true);
		
		valider.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		annuler.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		
		cadreValider.definirDimensions(valider.dimensions()); 
		cadreAnnuler.definirDimensions(annuler.dimensions()); 
		cadreValider.definirOrigine(cadre.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur)); 
		cadreAnnuler.definirOrigine(cadreValider.origine() + Coordonnees(80 * Ecran::echelleMin() + cadreValider.largeur, 0)); 
		
		valider.afficher(cadreValider.origine());
		annuler.afficher(cadreAnnuler.origine());
		
		Ecran::finaliser();
		
		int dep;
		Parametres::gestionEvenementsAfficheur(ancienDefilement, continuer, dep, premier, nbAffiches, selection, enTetes, champs);
		if(Session::evenement(Session::B_GAUCHE)) {
			if(Session::souris() < cadreValider) {
				_rechercherMaj = maj;
				
				continuer = false;
				break;
			}
			else if(Session::souris() < cadreAnnuler) {
				continuer = false;
				break;
			}
		}
		if(dep) {
			if(selection == 0) {
				Session::reinitialiser(Session::T_GAUCHE);
				Session::reinitialiser(Session::T_DROITE);
				maj = !maj;
			}
		}
		
		Ecran::maj();
	}
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
	bool ips = Parametres::ips();
	bool limiteIPS = Parametres::limiteIPS();
	bool synchroVerticale = Parametres::synchroVerticale();
	
	char const *txt[2] = {"Désactivé", "Activé"};
	
	std::vector<etiquetteTexte_t> enTetes, champs;

	enTetes.push_back(Unichar("Plein écran :"));
	champs.push_back(Unichar());
	
	enTetes.push_back(Unichar("Résolution :"));
	champs.push_back(Unichar());

	enTetes.push_back(Unichar("Synchronisation verticale :"));
	champs.push_back(Unichar());

	enTetes.push_back(Unichar("Limiter la fréquence d'affichage :"));
	champs.push_back(Unichar());

	enTetes.push_back(Unichar("Afficher images par seconde :"));
	champs.push_back(Unichar());
	
	Texte valider("Valider",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	Texte annuler("Annuler",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	
	Rectangle cadreAnnuler, cadreValider;
	
	bool continuer = true;
	index_t selection = 0, premier = 0;
	size_t nbAffiches = std::min<size_t>(enTetes.size(), 8);
	horloge_t ancienDefilement = horloge();
	
	Session::reinitialiserEvenements();
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		champs[0]._texte.definir(Unichar(txt[pleinEcran]));
		champs[1]._texte.definir(Unichar(Parametres::transcriptionResolution(resolutions[pleinEcran][resolutionActuelle])));
		champs[2]._texte.definir(Unichar(txt[synchroVerticale]));
		champs[3]._texte.definir(Unichar(txt[limiteIPS]));
		champs[4]._texte.definir(Unichar(txt[ips]));

		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));
		
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge());
		s.definirParametre(Shader::tempsAbsolu, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		Rectangle cadre = Parametres::afficherColonnes("Vidéo", premier, nbAffiches, false, enTetes, -1, champs, selection, true);
		
		valider.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		annuler.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		
		cadreValider.definirDimensions(valider.dimensions()); 
		cadreAnnuler.definirDimensions(annuler.dimensions()); 
		cadreValider.definirOrigine(cadre.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur)); 
		cadreAnnuler.definirOrigine(cadreValider.origine() + Coordonnees(80 * Ecran::echelleMin() + cadreValider.largeur, 0)); 

		valider.afficher(cadreValider.origine());
		annuler.afficher(cadreAnnuler.origine());

		Ecran::finaliser();
		
		int dep;
		Parametres::gestionEvenementsAfficheur(ancienDefilement, continuer, dep, premier, nbAffiches, selection, enTetes, champs);
		if(Session::evenement(Session::B_GAUCHE)) {
			if(Session::souris() < cadreValider) {
				_largeurEcran = resolutions[pleinEcran][resolutionActuelle].first;
				_hauteurEcran = resolutions[pleinEcran][resolutionActuelle].second;
				_pleinEcran = pleinEcran;
				_ips = ips;
				_limiteIPS = limiteIPS;
				_synchroVerticale = synchroVerticale;
				
				Ecran::modifierResolution(_largeurEcran, _hauteurEcran, _pleinEcran);
				continuer = false;
				break;
			}
			else if(Session::souris() < cadreAnnuler) {
				continuer = false;
				break;
			}
		}
		if(dep) {
			if(selection == 0) {
				Session::reinitialiser(Session::T_GAUCHE);
				Session::reinitialiser(Session::T_DROITE);
				pleinEcran = !pleinEcran;
				resolutionActuelle = 0;
			}
			else if(selection == 1) {
				resolutionActuelle += dep;
				if(resolutionActuelle < 0)
					resolutionActuelle = resolutions[pleinEcran].size() - 1;
				else if(resolutionActuelle >= resolutions[pleinEcran].size())
					resolutionActuelle = 0;
				
				Session::reinitialiser(Session::T_GAUCHE);
				Session::reinitialiser(Session::T_DROITE);
			}
			else if(selection == 2) {
				Session::reinitialiser(Session::T_GAUCHE);
				Session::reinitialiser(Session::T_DROITE);
				synchroVerticale = !synchroVerticale;
			}
			else if(selection == 3) {
				Session::reinitialiser(Session::T_GAUCHE);
				Session::reinitialiser(Session::T_DROITE);
				limiteIPS = !limiteIPS;
			}
			else if(selection == 4) {
				Session::reinitialiser(Session::T_GAUCHE);
				Session::reinitialiser(Session::T_DROITE);
				ips = !ips;
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
	
	float vMusique = Parametres::volumeMusique(), vEffets = Parametres::volumeEffets();
	
	Shader shader(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "volume.frag");
	
	bool continuer = true;
	index_t premier = 0;
	size_t nbAffiches = 2;
	
	index_t selection = -1;
	
	horloge_t ancienSon = 0;
	
	Texte valider("Valider",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	Texte annuler("Annuler",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	
	Rectangle cadreAnnuler, cadreValider;

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
		
		Rectangle cadre = Parametres::afficherColonnes("Audio", premier, nbAffiches, false, texte, -1, vide, -1, false);
		
		float *valeur[2] = {&vMusique, &vEffets};
		shader.activer();
		for(int i = 0; i < 2; ++i) {
			Rectangle const &r = vide[i]._cadre;
			shader.definirParametre(_shadVolume, *(valeur[i]));
			Rectangle s = Shader::versShader(r);
			shader.definirParametre(_shadCadre, s.gauche, s.haut, s.largeur, s.hauteur);

			Ecran::afficherTriangle(r.origine() + Coordonnees(0, r.hauteur), r.origine() + r.dimensions(), r.origine() + Coordonnees(r.largeur, 0), Couleur::blanc);
		}
		Shader::desactiver();
		
		valider.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		annuler.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		
		cadreValider.definirDimensions(valider.dimensions()); 
		cadreAnnuler.definirDimensions(annuler.dimensions()); 
		cadreValider.definirOrigine(cadre.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur)); 
		cadreAnnuler.definirOrigine(cadreValider.origine() + Coordonnees(80 * Ecran::echelleMin() + cadreValider.largeur, 0)); 
		
		valider.afficher(cadreValider.origine());
		annuler.afficher(cadreAnnuler.origine());

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
			Audio::definirVolumeMusique(vMusique);
			Audio::definirVolumeEffets(vEffets);
			if(selection == 1 && horloge() - ancienSon > 0.2) {
				ancienSon = horloge();
				Audio::jouerSon(_sonEffets);
			}
		}
		if(Session::evenement(Session::B_GAUCHE)) {
			if(Session::souris() < cadreValider) {
				_volumeMusique = vMusique;
				_volumeEffets = vEffets;
				continuer = false;
			}
			else if(Session::souris() < cadreAnnuler) {
				continuer = false;
				break;
			}
		}
		if(Session::evenement(Session::T_ESC))
			continuer = false;
		
		Ecran::maj();
	}
	
	Audio::definirVolumeMusique(_volumeMusique);
	Audio::definirVolumeEffets(_volumeEffets);
}

void Parametres::controles(Image const &fond, Shader const &s) {
	std::vector<etiquetteTexte_t> actions, etiquetteEvenements;

	Session::evenement_t evenements[nbActions];
	
	for(action_t i = premiereAction; i != nbActions; ++i) {
		actions.push_back(Unichar(Parametres::transcriptionAction(i)));
		etiquetteEvenements.push_back(Session::transcriptionEvenement(Parametres::evenementAction(i)));
		evenements[i] = _evenementsAction[i];
	}
	
	bool continuer = true, modification = false;
	index_t selection = 0, premier = 0;
	size_t nbAffiches = std::min<size_t>(actions.size(), 8);
	horloge_t ancienDefilement = horloge();
	
	Texte valider("Valider",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	Texte annuler("Annuler",  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
	
	Rectangle cadreAnnuler, cadreValider;

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
		
		Rectangle cadre = Parametres::afficherColonnes("Contrôles", premier, nbAffiches, modification, actions, -1, etiquetteEvenements, selection, false);

		valider.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		annuler.definir(POLICE_DECO, 26 * Ecran::echelleMin());
		
		cadreValider.definirDimensions(valider.dimensions()); 
		cadreAnnuler.definirDimensions(annuler.dimensions()); 
		cadreValider.definirOrigine(cadre.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur)); 
		cadreAnnuler.definirOrigine(cadreValider.origine() + Coordonnees(80 * Ecran::echelleMin() + cadreValider.largeur, 0)); 
		
		valider.afficher(cadreValider.origine());
		annuler.afficher(cadreAnnuler.origine());
		
		Ecran::finaliser();
		
		for(action_t i = premiereAction; i != nbActions; ++i) {
			etiquetteEvenements[i]._texte.definir(Session::transcriptionEvenement(evenements[i]));
		}

		if(!modification) {
			int dep;
			Parametres::gestionEvenementsAfficheur(ancienDefilement, continuer, dep, premier, nbAffiches, selection, actions, etiquetteEvenements);
			if(Session::evenement(Session::T_ENTREE)) {
				modification = true;
				Session::reinitialiser(Session::T_ENTREE);
			}
			
			if(Session::evenement(Session::B_GAUCHE)) {
				if(Session::souris() < cadreValider) {
					for(action_t i = premiereAction; i != nbActions; ++i) {
						Parametres::definirEvenementAction(i, evenements[i]);
					}
					continuer = false;
				}
				else if(Session::souris() < cadreAnnuler) {
					continuer = false;
					break;
				}
				else {
					std::vector<etiquetteTexte_t>::const_iterator souris = std::find_if(etiquetteEvenements.begin(), etiquetteEvenements.end(), trouveSouris_t());
					if(souris != etiquetteEvenements.end()) {
						modification = true;
					}
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
				for(Session::evenement_t e = Session::PREMIER_EVENEMENT_CLAVIER; e <= Session::DERNIER_EVENEMENT_CLAVIER; ++e) {
					if(Session::evenement(e)) {
						evenements[static_cast<action_t>(selection)] = e;
						modification = false;
						break;
					}
				}
			}
		}
				
		Ecran::maj();
	}
}

Rectangle Parametres::afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<Parametres::etiquetteTexte_t> &colonne1, index_t sel1, std::vector<Parametres::etiquetteTexte_t> &colonne2, index_t sel2, bool fleches, coordonnee_t abscisse) {
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
	
	if(fleches) {
		_flecheG->redimensionner(0.5 * colonne1[0]._texte.dimensions().y / _flecheG->dimensionsReelles().y);
		_flecheD->redimensionner(0.5 * colonne1[0]._texte.dimensions().y / _flecheD->dimensionsReelles().y);
	}
	
	Rectangle cadre(Coordonnees(abscisse, 80) * Ecran::echelleMin(), tt.dimensions());
	tt.afficher(cadre.origine());
	
	cadre.haut += cadre.hauteur + 20 * Ecran::echelleMin();
	cadre.gauche += 20 * Ecran::echelleMin();
	
	dimension_t largeur1 = std::max_element(colonne1.begin(), colonne1.end(), largeurTexte_t())->_texte.dimensions().x, largeur2 = -40 * Ecran::echelleMin();
	if(colonne2.size()) {
		largeur2 = std::max_element(colonne2.begin(), colonne2.end(), largeurTexte_t())->_texte.dimensions().x;
		if(fleches)
			largeur2 += 2 * 10 * Ecran::echelleMin() + _flecheG->dimensions().x + _flecheD->dimensions().x;
	}
	
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
	if(colonne2.size()) {
		cadre.largeur = largeur2;
		afficheur.definirFleches(fleches);
		for_each(colonne2.begin() + premier, colonne2.begin() + premier + nb, afficheur);
	}
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
	if(colonne2.size()) {
		initCadres.definirFleches(true);
		for_each(colonne2.begin() + premier, colonne2.begin() + premier + nb, initCadres);
	}
	
	cadre.gauche -= largeur1 + 40 * Ecran::echelleMin();
	if(!dernierAffiche) {
		cadre += Coordonnees(10, 10) * Ecran::echelleMin();
		points.afficher(cadre.origine() + Coordonnees((largeur1 + largeur2 + 40 * Ecran::echelleMin() - points.dimensions().x) / 2, 0));
		cadre.haut += points.dimensions().y + 10 * Ecran::echelleMin();
	}
	cadre -= Coordonnees(10, -10) * Ecran::echelleMin();
	
	return total;
}

void Parametres::gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, int &dep, index_t &premier, index_t nbAffiches, index_t &selection, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2) {
	dep = 0;
	
	bool premierAffiche = premier == 0;
	if(!premierAffiche)
		--nbAffiches;
	bool dernierAffiche = premier + nbAffiches == colonne1.size();
	if(!dernierAffiche)
		--nbAffiches;

	if(Session::evenement(Session::T_ESC)) {
		continuer = false;
		Session::reinitialiser(Session::T_ESC);
	}
	else if(Session::evenement(Session::B_GAUCHE)) {
		std::vector<etiquetteTexte_t>::const_iterator souris = std::find_if(colonne1.begin(), colonne1.end(), trouveSouris_t());
		if(souris != colonne1.end()) {
			selection = std::distance(colonne1.begin(), souris);
		}
		else  {
			souris = std::find_if(colonne2.begin(), colonne2.end(), trouveSouris_t());
			if(souris != colonne2.end()) {
				selection = std::distance(colonne2.begin(), souris);
				Session::reinitialiser(Session::B_GAUCHE);
			}

			if(Session::souris() < colonne2[selection]._flecheG) {
				dep = -1;
				Session::reinitialiser(Session::B_GAUCHE);
			}
			else if(Session::souris() < colonne2[selection]._flecheD) {
				dep = 1;
				Session::reinitialiser(Session::B_GAUCHE);
			}
		}
	}
	else if(Session::evenement(Session::T_GAUCHE)) {
		dep = -1;
	}
	else if(Session::evenement(Session::T_DROITE)) {
		dep = 1;
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
		
		Rectangle cadre = Parametres::afficherColonnes("Crédits", 0, noms.size(), false, noms, -1, roles, -1, false, 40);
		
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

