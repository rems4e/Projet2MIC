//
//  fonctions.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_FONCTIONS
#define EN_TETE_FONCTIONS

#include <string>
#include <vector>

template <typename T>
T signe(T const &t) {
	return (t > 0)  ? 1 : ((t < 0) ? -1 : 0);
}

// Nombre aléatoire entre 0 et (nombreMax - 1)
int nombreAleatoire(int nombreMax);

template <typename T>
bool comprisEntre(T valeur, T borneInf, T borneSup) {
	return valeur >= borneInf && valeur <= borneSup;
}

int caractereVersHexa(char c);
char hexaVersCaractere(int c);
int caractereVersBase64(char c);
char base64VersCaractere(int c);

double texteVersNombre(std::string const &texte);
std::string nombreVersTexte(double nombre, int decimales);
std::string nombreVersTexte(long nombre);

int nombreDecimales(double nombre);

template <unsigned int val>
struct nombreChiffres {
	enum {nb = nombreChiffres<val / 10 + 1>::nb + 1};
};

void decouperChaine(std::string c, std::string const &separateur, std::vector<std::string> &sousChaines);

template <>
struct nombreChiffres<1> {
	enum {nb = 1};
};

template <>
struct nombreChiffres<2> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<3> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<4> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<5> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<6> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<7> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<8> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<9> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<0> {
	enum {nb = 1};
};

#endif
