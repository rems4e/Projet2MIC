//
//  Parametres.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Parametres.h"
#include "Image.h"
#include "Affichage.h"
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
#include "nombre.h"
#include <sstream>

namespace Session {
	void rechargerLangue();
}

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
	
	void definirEvenementAction(action_t action, Session::evenement_t e);
	void definirVolumeMusique(float v);
	void definirVolumeEffets(float v);
	
	Unichar transcriptionAction(action_t);
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
	
	std::string transcriptionResolution(glm::ivec2 const &r);
	Rectangle afficherColonnes(Unichar const &titre, index_t premier, size_t nb, bool modif, std::vector<etiquetteTexte_t> &colonne1, index_t sel1, std::vector<etiquetteTexte_t> &colonne2, index_t sel2, bool fleches, coordonnee_t abscisse = 80);
	int gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, index_t &premier, index_t nbAffiches, index_t &selection, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2);
	
	Session::evenement_t _evenementsAction[nbActions];
	float _volumeMusique;
	float _volumeEffets;
	
	Audio::audio_t _sonEffets = 0;
	
	std::string _langue;
	
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
				Parametres::_flecheG->afficher(_cadre.origine() + glm::vec2(0, (_cadre.dimensions().y - Parametres::_flecheG->dimensions().y) / 2));
				t._texte.afficher(_cadre.origine() + glm::vec2(Parametres::_flecheG->dimensions().x + 10 * Ecran::echelleMin(), 0));
				Parametres::_flecheD->afficher(_cadre.origine() + glm::vec2(_cadre.dimensions().x - _flecheD->dimensions().x, (_cadre.dimensions().y - Parametres::_flecheD->dimensions().y) / 2));
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
				t._flecheG.definirDimensions(glm::vec2(Parametres::_flecheG->dimensions().x, std::max(Parametres::_flecheG->dimensions().y, _cadre.hauteur)));
				
				t._flecheD.definirOrigine(_cadre.origine() + _cadre.dimensions() - Parametres::_flecheD->dimensions());
				t._flecheD.definirDimensions(glm::vec2(Parametres::_flecheD->dimensions().x, std::max(Parametres::_flecheD->dimensions().y, _cadre.hauteur)));
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
			return e._cadre.contientPoint(Session::souris());
		}
	};
	
	struct VueReglages : public VueInterface {
	public:
		VueReglages(Image const &fond, Shader const &s, Unichar const &titre, bool fleches) : VueInterface(Rectangle::zero), _s(s), _fond(fond), _titre(titre), _fleches(fleches) {
			
		}
	protected:
		void preparationDessin() override {
			this->definirCadre(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur()));
		}
		
		void dessiner() override {
			_nbAffiches = std::min<size_t>(_enTetes.size(), 8);
			
			Texte valider(TRAD("reg Valider"),  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
			Texte annuler(TRAD("reg Annuler"),  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc);
			
			_fond.redimensionner(glm::vec2(Ecran::largeur() / _fond.dimensionsReelles().x, Ecran::hauteur() / _fond.dimensionsReelles().y));
			
			Ecran::definirPointeurAffiche(true);
			
			_s.activer();
			_s.definirParametre(Shader::temps, horloge());
			_s.definirParametre(Shader::tempsAbsolu, horloge());
			_fond.afficher(glm::vec2(0));
			Shader::desactiver();
			
			Rectangle cadre = Parametres::afficherColonnes(_titre, _premier, _nbAffiches, _cligno, _enTetes, -1, _champs, _selection, _fleches);
			
			valider.definir(POLICE_DECO, 26 * Ecran::echelleMin());
			annuler.definir(POLICE_DECO, 26 * Ecran::echelleMin());
			
			_cadreValider.definirDimensions(valider.dimensions());
			_cadreAnnuler.definirDimensions(annuler.dimensions());
			_cadreValider.definirOrigine(cadre.origine() + glm::vec2(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur));
			_cadreAnnuler.definirOrigine(_cadreValider.origine() + glm::vec2(80 * Ecran::echelleMin() + _cadreValider.largeur, 0));
			
			valider.afficher(_cadreValider.origine());
			annuler.afficher(_cadreAnnuler.origine());
		}
		
		Shader const &_s;
		Image const &_fond;
		
		Rectangle _cadreValider, _cadreAnnuler;
		std::vector<etiquetteTexte_t> _enTetes, _champs;
		index_t _premier = 0, _selection = 0;
		size_t _nbAffiches;
		
		horloge_t _ancienDefilement = 0;
		Unichar _titre;
		bool _fleches;
		bool _cligno = false;
		
		Unichar _txt[2] = {TRAD("reg Désactivé"), TRAD("reg Activé")};
	};
	
	class VueGeneral : public VueReglages {
	public:
		VueGeneral(Image const &fond, Shader const &s) : VueReglages(fond, s, TRAD("reg Général"), true) {
			_enTetes.push_back(Unichar(TRAD("reg Rechercher les mises à jour")));
			_champs.push_back(_txt[_maj]);
		}
	protected:
		void gestionClavier() override;
		
	private:
		bool _maj = Parametres::rechercherMaj();
	};
	
	class VueControles : public VueReglages {
	public:
		VueControles(Image const &fond, Shader const &s) : VueReglages(fond, s, TRAD("reg Contrôles"), false) {
			for(action_t i = premiereAction; i != nbActions; ++i) {
				_enTetes.push_back(Unichar(Parametres::transcriptionAction(i)));
				_champs.push_back(Session::transcriptionEvenement(Parametres::evenementAction(i), true));
				_evenements[i] = _evenementsAction[i];
			}
		}
	protected:
		void gestionClavier() override;
		
	private:
		Session::evenement_t _evenements[nbActions];
		bool _modification = false;
	};
	class VueAudio : public VueReglages {
	public:
		VueAudio(Image const &fond, Shader const &s) : VueReglages(fond, s, TRAD("reg Audio"), false), _shader(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "volume.frag") {
			_enTetes.push_back(Unichar(TRAD("reg Volume musique")));
			_enTetes.push_back(Unichar(TRAD("reg Volume effets")));
			Unichar blanc("                                       ");
			_champs.push_back(blanc);
			_champs.push_back(blanc);
		}
	protected:
		void gestionClavier() override;
		void dessiner() override;
		
	private:
		Shader _shader;

		horloge_t _ancienSon = 0;
		float _vMusique = Parametres::volumeMusique();
		float _vEffets = Parametres::volumeEffets();
	};
	class VueVideo : public VueReglages {
	public:
		VueVideo(Image const &fond, Shader const &s) : VueReglages(fond, s, TRAD("reg Vidéo"), true) {
			{
				std::list<glm::ivec2> r = Ecran::resolutionsDisponibles(false);
				_resolutions[0].assign(r.begin(), r.end());
				
				r = Ecran::resolutionsDisponibles(true);
				_resolutions[1].assign(r.begin(), r.end());
			}
			_resolutionActuelle = std::find(_resolutions[_pleinEcran].begin(), _resolutions[_pleinEcran].end(), Ecran::dimensions()) - _resolutions[_pleinEcran].begin();
			
			_enTetes.push_back(Unichar(TRAD("reg Plein écran :")));
			_champs.push_back(Unichar(_txt[_pleinEcran]));
			
			_enTetes.push_back(Unichar(TRAD("reg Résolution :")));
			_champs.push_back(Unichar(Parametres::transcriptionResolution(_resolutions[_pleinEcran][_resolutionActuelle])));
			
			_enTetes.push_back(Unichar(TRAD("reg Synchronisation verticale :")));
			_champs.push_back(Unichar(_txt[_synchroVerticale]));
			
			_enTetes.push_back(Unichar(TRAD("reg Limiter la fréquence d'affichage :")));
			_champs.push_back(Unichar(_txt[_limiteIPS]));
			
			_enTetes.push_back(Unichar(TRAD("reg Afficher les images par seconde :")));
			_champs.push_back(Unichar(_txt[_ips]));
		}
	protected:
		void gestionClavier() override;

	private:
		bool _pleinEcran = Ecran::pleinEcran();
		std::vector<glm::ivec2> _resolutions[2];
		index_t _resolutionActuelle;
		bool _ips = Parametres::ips();
		bool _limiteIPS = Parametres::limiteIPS();
		bool _synchroVerticale = Parametres::synchroVerticale();
	};
	
	class VueCredits : public VueInterface {
	public:
		VueCredits(Image const &fond, Shader const &s) : VueInterface(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur())), _ok(TRAD("cred Retour"),  POLICE_DECO, 26 * Ecran::echelleMin(), Couleur::blanc), _fond(fond), _s(s) {
			_noms.push_back(Unichar("Marc Promé"));
			_roles.push_back(Unichar(TRAD("cred Level design, graphismes, programmation")));
			
			_noms.push_back(Unichar("Rémi Saurel"));
			_roles.push_back(Unichar(TRAD("cred Level design, graphismes, programmation")));
			
			_noms.push_back(Unichar("OpenGameArt.org"));
			_roles.push_back(Unichar(TRAD("cred Graphismes")));
			
			_noms.push_back(Unichar("Semaphore (Newgrounds)"));
			_roles.push_back(Unichar(TRAD("cred Musique %1", "Dream of Water and Land")));
			
			_noms.push_back(Unichar("Sexual-Lobster (Newgrounds)"));
			_roles.push_back(Unichar(TRAD("cred Musique %1", "Man vs. Walrus-Man")));
			
			_noms.push_back(Unichar("Xerferic (Newgrounds)"));
			_roles.push_back(Unichar(TRAD("cred Musique %1", "Laid-Back")));
			
			for(int i = 0; i < _noms.size(); ++i) {
				_noms[i]._texte.definir(POLICE_DECO, 16 * Ecran::echelleMin());
				_roles[i]._texte.definir(POLICE_DECO, 16 * Ecran::echelleMin());
			}
		}
	protected:
		void dessiner() override;
		void gestionClavier() override;
	private:
		std::vector<etiquetteTexte_t> _noms, _roles;
				
		Texte _ok;
		Rectangle _cadreOk;
		
		Image const &_fond;
		Shader const &_s;
	};
}

static size_t ecrireDonnees(char *ptr, size_t size, size_t nmemb, void *userdata) {
	std::ostringstream *stream = (std::ostringstream *)userdata;
	size_t count = size * nmemb;
	stream->write(ptr, count);
	
	return count;
}

bool Parametres::majDisponible() {
	char const *fichierMaj = "etud.insa-toulouse.fr/~saurel/projet2MIC-version.txt";
	
	//FIXME: timeout connexion
	std::ostringstream data;
	cURL::CURL *session = cURL::curl_easy_init();
	cURL::curl_easy_setopt(session, cURL::CURLOPT_URL, fichierMaj);
	cURL::curl_easy_setopt(session,  cURL::CURLOPT_WRITEDATA, &data);
	cURL::curl_easy_setopt(session, cURL::CURLOPT_SSL_VERIFYPEER, false);
	cURL::curl_easy_setopt(session,  cURL::CURLOPT_WRITEFUNCTION, ecrireDonnees);
	cURL::curl_easy_setopt(session,  cURL::CURLOPT_DNS_CACHE_TIMEOUT, 1);
	cURL::curl_easy_setopt(session,  cURL::CURLOPT_TIMEOUT, 1);
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
			std::cerr << "Fichier de mise à jour malformé." << std::endl;
			return false;
		}
	}
}

char const *Parametres::URLMaj() {
	return "http://r4.saurel.free.fr/Projet2MIC/index.php";
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
	
#if defined(DEVELOPPEMENT)
	for(action_t a = premiereAction; a != nbActions; ++a) {
		if(_evenementsAction[a] == Session::aucunEvenement) {
			std::cerr << "Aucune valeur par défaut pour l'événement " << Parametres::transcriptionAction(a) << std::endl;
			throw 0;
		}
	}
#endif
	
	_volumeEffets = 1.0f;
	_volumeMusique = 1.0f;
	
	_largeurEcran = 800;
	_hauteurEcran = 600;
	_pleinEcran = false;
	_ips = false;
	_limiteIPS = true;
	_synchroVerticale = true;
	_rechercherMaj = true;
	
	_langue = "fr";
	
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
					std::string ee = Session::transcriptionEvenement(e, false);
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
				if(general->Attribute("langue")) {
					_langue = general->Attribute("langue");
				}
			}
		}
	}
	
	Session::rechargerLangue();
	
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
		action.SetAttribute("valeur", Session::transcriptionEvenement(Parametres::evenementAction(a), false));
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
	general->SetAttribute("langue", Parametres::langue());
	general->SetAttribute("rechercherMaj", Parametres::rechercherMaj());
	
	if(!document->SaveFile())
		std::cerr << "L'enregistrement du fichier de paramètres " << document->Value() << " a échoué." << std::endl;
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

std::string const &Parametres::langue() {
	return _langue;
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
	
	index_t selection = 0;
	do {
		elements.clear();
		elements.push_back(TRAD("reg Contrôles"));
		elements.push_back(TRAD("reg Audio"));
		elements.push_back(TRAD("reg Vidéo"));
		elements.push_back(TRAD("reg Général"));
		Menu menu(TRAD("reg Réglages"), elements);
		
		selection = menu.afficher(selection, fond, s);
		switch(selection) {
			case 3: {
				VueGeneral general(fond, s);
				Session::ajouterVueFenetre(&general);
				break;
			}
			case 2: {
				VueVideo video(fond, s);
				Session::ajouterVueFenetre(&video);
				break;
			}
			case 1: {
				VueAudio audio(fond, s);
				Session::ajouterVueFenetre(&audio);
				break;
			}
			case 0: {
				VueControles controles(fond, s);
				Session::ajouterVueFenetre(&controles);
				break;
			}
		}
	} while(selection != elements.size());
}

std::string Parametres::transcriptionResolution(glm::ivec2 const &r) {
	return nombreVersTexte(r.x) + "*" + nombreVersTexte(r.y);
}

void Parametres::VueGeneral::gestionClavier() {
	bool continuer = true;
	int dep = Parametres::gestionEvenementsAfficheur(_ancienDefilement, continuer, _premier, _nbAffiches,  _selection, _enTetes, _champs);
	if(Session::evenement(Session::B_GAUCHE)) {
		if(_cadreValider.contientPoint(Session::souris())) {
			_rechercherMaj = _maj;
			
			continuer = false;
		}
		else if(_cadreAnnuler.contientPoint(Session::souris())) {
			continuer = false;
		}
	}
	if(dep) {
		if(_selection == 0) {
			Session::reinitialiser(Session::T_GAUCHE);
			Session::reinitialiser(Session::T_DROITE);
			_maj = !_maj;
			_champs[0]._texte.definir(_txt[_maj]);
		}
	}
	
	if(!continuer) {
		Session::supprimerVueFenetre();
	}
}

void Parametres::VueVideo::gestionClavier() {
	bool continuer = true;
	int dep = Parametres::gestionEvenementsAfficheur(_ancienDefilement, continuer, _premier, _nbAffiches, _selection, _enTetes, _champs);
	if(Session::evenement(Session::B_GAUCHE)) {
		if(_cadreValider.contientPoint(Session::souris())) {
			Parametres::_largeurEcran = _resolutions[_pleinEcran][_resolutionActuelle].x;
			Parametres::_hauteurEcran = _resolutions[_pleinEcran][_resolutionActuelle].y;
			Parametres::_pleinEcran = _pleinEcran;
			Parametres::_ips = _ips;
			Parametres::_limiteIPS = _limiteIPS;
			Parametres::_synchroVerticale = _synchroVerticale;
			
			Ecran::modifierResolution(_largeurEcran, _hauteurEcran, _pleinEcran);
			continuer = false;
		}
		else if(_cadreAnnuler.contientPoint(Session::souris())) {
			continuer = false;
		}
	}
	if(dep) {
		if(_selection == 0) {
			Session::reinitialiser(Session::T_GAUCHE);
			Session::reinitialiser(Session::T_DROITE);
			
			_pleinEcran = !_pleinEcran;
			_resolutionActuelle = 0;
			
			_champs[0]._texte.definir(Unichar(_txt[_pleinEcran]));
			_champs[1]._texte.definir(Unichar(Parametres::transcriptionResolution(_resolutions[_pleinEcran][_resolutionActuelle])));
		}
		else if(_selection == 1) {
			Session::reinitialiser(Session::T_GAUCHE);
			Session::reinitialiser(Session::T_DROITE);

			_resolutionActuelle += dep;
			if(_resolutionActuelle < 0)
				_resolutionActuelle = _resolutions[_pleinEcran].size() - 1;
			else if(_resolutionActuelle >= _resolutions[_pleinEcran].size())
				_resolutionActuelle = 0;
			
			_champs[1]._texte.definir(Unichar(Parametres::transcriptionResolution(_resolutions[_pleinEcran][_resolutionActuelle])));
			
		}
		else if(_selection == 2) {
			Session::reinitialiser(Session::T_GAUCHE);
			Session::reinitialiser(Session::T_DROITE);
			
			_synchroVerticale = !_synchroVerticale;

			_champs[2]._texte.definir(Unichar(_txt[_synchroVerticale]));
		}
		else if(_selection == 3) {
			Session::reinitialiser(Session::T_GAUCHE);
			Session::reinitialiser(Session::T_DROITE);
			
			_limiteIPS = !_limiteIPS;

			_champs[3]._texte.definir(Unichar(_txt[_limiteIPS]));
		}
		else if(_selection == 4) {
			Session::reinitialiser(Session::T_GAUCHE);
			Session::reinitialiser(Session::T_DROITE);
			
			_ips = !_ips;

			_champs[4]._texte.definir(Unichar(_txt[_ips]));
		}
	}

	if(!continuer) {
		Session::supprimerVueFenetre();
	}
}

void Parametres::VueAudio::dessiner() {
	this->VueReglages::dessiner();
	
	float *valeur[2] = {&_vMusique, &_vEffets};
	_shader.activer();
	for(int i = 0; i < 2; ++i) {
		Rectangle const &r = _champs[i]._cadre;
		_shader.definirParametre(_shadVolume, *(valeur[i]));
		Rectangle s = Shader::versShader(r);
		_shader.definirParametre(_shadCadre, glm::vec4(s.gauche, s.haut, s.largeur, s.hauteur));
		
		Affichage::afficherTriangle(r.origine() + glm::vec2(0, r.hauteur), r.origine() + r.dimensions(), r.origine() + glm::vec2(r.largeur, 0), Couleur::blanc);
	}
	Shader::desactiver();
}

void Parametres::VueAudio::gestionClavier() {
	bool continuer = true;
	
	float *valeur[2] = {&_vMusique, &_vEffets};

	if(!Session::evenement(Session::B_GAUCHE)) {
		_selection = -1;
	}
	
	if(_selection == -1) {
		if(Session::evenement(Session::B_GAUCHE)) {
			std::vector<etiquetteTexte_t>::iterator i = std::find_if(_champs.begin(), _champs.end(), trouveSouris_t());
			if(i != _champs.end())
				_selection = std::distance(_champs.begin(), i);
		}
	}
	else {
		*(valeur[_selection]) = std::min(1.0f, std::max<float>(0.0f, (Session::souris().x - _champs[_selection]._cadre.gauche) / _champs[_selection]._cadre.largeur));
		Audio::definirVolumeMusique(_vMusique);
		Audio::definirVolumeEffets(_vEffets);
		if(_selection == 1 && horloge() - _ancienSon > 0.2) {
			_ancienSon = horloge();
			Audio::jouerSon(_sonEffets);
		}
	}
	if(Session::evenement(Session::B_GAUCHE)) {
		if(_cadreValider.contientPoint(Session::souris())) {
			_volumeEffets = _vEffets;
			_volumeMusique = _vMusique;
			continuer = false;
		}
		else if(_cadreAnnuler.contientPoint(Session::souris())) {
			Audio::definirVolumeMusique(_volumeMusique);
			Audio::definirVolumeEffets(_volumeEffets);
			continuer = false;
		}
	}
	if(Session::evenement(Session::T_ESC))
		continuer = false;


	if(!continuer) {
		Session::supprimerVueFenetre();
	}
}

void Parametres::VueControles::gestionClavier() {
	bool continuer = true;
	_cligno = _modification;
	if(!_modification) {
		Parametres::gestionEvenementsAfficheur(_ancienDefilement, continuer, _premier, _nbAffiches, _selection, _enTetes, _champs);
		if(Session::evenement(Session::T_ENTREE)) {
			_modification = true;
			Session::reinitialiser(Session::T_ENTREE);
		}
		if(Session::evenement(Session::B_GAUCHE)) {
			if(_cadreValider.contientPoint(Session::souris())) {
				for(action_t i = premiereAction; i != nbActions; ++i) {
					Parametres::definirEvenementAction(i, _evenements[i]);
				}
				continuer = false;
			}
			else if(_cadreAnnuler.contientPoint(Session::souris())) {
				continuer = false;
			}
			else {
				std::vector<etiquetteTexte_t>::const_iterator souris = std::find_if(_champs.begin(), _champs.end(), trouveSouris_t());
				if(souris != _champs.end()) {
					_modification = true;
				}
			}
		}
	}
	else {
		if(Session::evenement(Session::T_ESC) || Session::evenement(Session::T_ENTREE)) {
			Session::reinitialiser(Session::T_ESC);
			Session::reinitialiser(Session::T_ENTREE);
			_modification = false;
		}
		else {
			for(Session::evenement_t e = Session::PREMIER_EVENEMENT_CLAVIER; e <= Session::DERNIER_EVENEMENT_CLAVIER; ++e) {
				if(Session::evenement(e)) {
					_evenements[static_cast<action_t>(_selection)] = e;
					_champs[static_cast<action_t>(_selection)]._texte.definir(Session::transcriptionEvenement(_evenements[static_cast<action_t>(_selection)], true));
					_modification = false;
					Session::reinitialiser(e);
					break;
				}
			}
		}
	}

	if(!continuer) {
		Session::supprimerVueFenetre();
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
	
	Rectangle cadre(glm::vec2(abscisse, 80) * Ecran::echelleMin(), tt.dimensions());
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
	Affichage::afficherRectangle(total, Couleur(200, 205, 220, 128));
	
	cadre.decaler(glm::vec2(10, 10) * Ecran::echelleMin());
	
	if(!premierAffiche) {
		points.afficher(cadre.origine() + glm::vec2((largeur1 + largeur2 + 40 * Ecran::echelleMin() - points.dimensions().x) / 2, 0));
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
		initCadres.definirFleches(fleches);
		for_each(colonne2.begin() + premier, colonne2.begin() + premier + nb, initCadres);
	}
	
	cadre.gauche -= largeur1 + 40 * Ecran::echelleMin();
	if(!dernierAffiche) {
		cadre.decaler(glm::vec2(10, 10) * Ecran::echelleMin());
		points.afficher(cadre.origine() + glm::vec2((largeur1 + largeur2 + 40 * Ecran::echelleMin() - points.dimensions().x) / 2, 0));
		cadre.haut += points.dimensions().y + 10 * Ecran::echelleMin();
	}
	cadre.decaler(-glm::vec2(10, -10) * Ecran::echelleMin());
	
	return total;
}

int Parametres::gestionEvenementsAfficheur(horloge_t &ancienDefilement, bool &continuer, index_t &premier, index_t nbAffiches, index_t &selection, std::vector<etiquetteTexte_t> const &colonne1, std::vector<etiquetteTexte_t> const &colonne2) {
	int dep = 0;
	
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
		if(colonne2[selection]._flecheG.contientPoint(Session::souris())) {
			dep = -1;
			Session::reinitialiser(Session::B_GAUCHE);
		}
		else if(colonne2[selection]._flecheD.contientPoint(Session::souris())) {
			dep = 1;
			Session::reinitialiser(Session::B_GAUCHE);
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
	
	return dep;
}

Unichar Parametres::transcriptionAction(action_t a) {
	switch(a) {
		case depBas:
			return TRAD("reg ta Bas");
		case depHaut:
			return TRAD("reg ta Haut");
		case depDroite:
			return TRAD("reg ta Droite");
		case depGauche:
			return TRAD("reg ta Gauche");
		case afficherInventaire:
			return TRAD("reg ta Afficher/masquer l'inventaire");
		case interagir:
			return TRAD("reg ta Interagir/ramasser");
		case remplirVie:
			return TRAD("reg ta Potion de vie");
		case nbActions:
			return Unichar::uninull;
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
	VueCredits vue(fond, s);
	Session::ajouterVueFenetre(&vue);
}

void Parametres::VueCredits::gestionClavier() {
	if(Session::evenement(Session::T_ESC) || (Session::evenement(Session::B_GAUCHE) && _cadreOk.contientPoint(Session::souris()))) {
		Session::supprimerVueFenetre();
		Session::reinitialiser(Session::T_ESC);
		Session::reinitialiser(Session::B_GAUCHE);
	}
}

void Parametres::VueCredits::dessiner() {
	_fond.redimensionner(glm::vec2(Ecran::largeur() / _fond.dimensionsReelles().x, Ecran::hauteur() / _fond.dimensionsReelles().y));
	
	Ecran::definirPointeurAffiche(true);
	
	_s.activer();
	_s.definirParametre(Shader::temps, horloge());
	_s.definirParametre(Shader::tempsAbsolu, horloge());
	_fond.afficher(glm::vec2(0));
	Shader::desactiver();
	
	Rectangle cadre = Parametres::afficherColonnes(TRAD("cred Crédits"), 0, _noms.size(), false, _noms, -1, _roles, -1, false, 40);
	
	_cadreOk.definirDimensions(_ok.dimensions());
	_cadreOk.definirOrigine(cadre.origine() + glm::vec2(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur));
	
	_ok.afficher(_cadreOk.origine());
}
