//
//  Unichar.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Unichar.h"
#include <cstring>

#define AC_AIGU 0x301
#define AC_GRAVE 0x300
#define AC_CIRCONFLEXE 0x302
#define AC_TREMA 0x308
#define AC_TILDE 0x303

Unichar const Unichar::uninull;

enum {L_A, L_E, L_I, L_O, L_U,
	nbLettresAccentuees
};

enum {GRAVE, AIGU, CIRCONFLEXE, TILDE, TREMA};


static Unichar::unichar const uniAcc[nbLettresAccentuees] = {0xC0, 0xC8, 0xCC, 0xD2, 0xD9};
static bool const tilde[nbLettresAccentuees] = {true, false, false, true, false};
static int const correspondanceAccents[9] = {GRAVE, AIGU, CIRCONFLEXE, TILDE, -1, -1, -1, -1, TREMA};

//template Unichar::Unichar(std::vector<int>::const_iterator, std::vector<int>::const_iterator);

Unichar::Unichar(char const *txt, size_t nbCar) : _unitxt(), _txt() {
	this->init(txt, nbCar);
}

Unichar::Unichar(std::string const &txt) : _unitxt(), _txt() {
	this->init(txt.c_str(), txt.size());
}

Unichar::Unichar(Unichar const &txt) : _unitxt(txt._unitxt), _txt(txt._txt) {
}

Unichar::~Unichar() {
	
}

void Unichar::init(char const *txt, size_t nbCar) {
	if(nbCar == (size_t)-1) {
		if(txt == 0)
			nbCar = 0;
		else
			nbCar = std::strlen(txt);
	}
	
	_unitxt.clear();
	_txt.assign(txt, nbCar);

	for(unichar i = 0, c; i < nbCar; ++i) {
		c = ((unsigned char *)txt)[i];
		if(c >= 0xF0) {
			c = (unichar)(txt[i] & 0x07) << 18;
			c |= (unichar)(txt[++i] & 0x3F) << 12;
			c |= (unichar)(txt[++i] & 0x3F) << 6;
			c |= (unichar)(txt[++i] & 0x3F);
		}
		else if(c >= 0xE0) {
			c = (unichar)(txt[i] & 0x0F) << 12;
			c |= (unichar)(txt[++i] & 0x3F) << 6;
			c |= (unichar)(txt[++i] & 0x3F);
		}
		else if(c >= 0xC0) {
			c = (unichar)(txt[i] & 0x1F) << 6;
			c |= (unichar)(txt[++i] & 0x3F);
		}
		if((c == AC_GRAVE  || c == AC_AIGU || c == AC_CIRCONFLEXE || c == AC_TILDE || c == AC_TREMA) && _unitxt.size()) {
			int let = -1;
			bool maj = false;
			switch(_unitxt.back()) {
				case 'A':
					maj = true;
				case 'a':
					let = L_A;
					break;
				case 'E':
					maj = true;
				case 'e':
					let = L_E;
					break;
				case 'I':
					maj = true;
				case 'i':
					let = L_I;
					break;
				case 'O':
					maj = true;
				case 'o':
					let = L_O;
					break;
				case 'U':
					maj = true;
				case 'u':
					let = L_U;
					break;
				default:
					break;
			}
			if(let != -1) {
				Unichar::unichar retour = uniAcc[let] + 32 * !maj + correspondanceAccents[c - 0x300] - (!tilde[let] && correspondanceAccents[c - 0x300] == CIRCONFLEXE);
				_unitxt.back() = retour;
			}
		}
		else
			_unitxt.push_back(c);
	}
	
}

size_t Unichar::unisize(char const *utf8txt) {
	size_t i = 0;
	for(long c; *utf8txt; ++utf8txt, ++i) {
		c = *((unsigned char *)utf8txt);
		if(c >= 0xF0) {
			utf8txt += 3;
		}
		else if(c >= 0xE0) {
			utf8txt += 2;
		}
		else if(c >= 0xC0) {
			utf8txt += 1;
		}
	}
	
	return i;
}

void Unichar::replace(iterator i1, iterator i2, Unichar const &txt) {
	iterator i = _unitxt.erase(i1, i2);
	_unitxt.insert(i, txt.begin(), txt.end());
	_txt = this->calcUtf8();
}

void Unichar::replace(iterator i1, iterator i2, std::string const &txt) {
	Unichar t(txt);
	iterator i = _unitxt.erase(i1, i2);
	_unitxt.insert(i, t.begin(), t.end());
	_txt = this->calcUtf8();
}

void Unichar::replace(iterator i1, iterator i2, char c) {
	if(i1 == i2)
		return;
	_unitxt.erase(i1 + 1, i2);
	*i1 = c;
}

std::string Unichar::calcUtf8() const {
	std::string txt;
	for(unsigned int i = 0; i < _unitxt.size(); ++i) {
		int c = _unitxt[i];
		
		if(c < 0x80) {
			txt.push_back(c);
		}
		else if(c < 0x800) {
			txt.push_back(0xC0 | c >> 6);
			txt.push_back(0x80 | (c & 0x3F));
		}
		else if(c < 0x10000) {
			txt.push_back(0xE0 | c >> 12);
			txt.push_back(0x80 | (c >> 6 & 0x3F));
			txt.push_back(0x80 | (c & 0x3F));
		}
		else if(c < 0x200000) {
			txt.push_back(0xF0 | c >> 18);
			txt.push_back(0x80 | (c >> 12 & 0x3F));
			txt.push_back(0x80 | (c >> 6 & 0x3F));
			txt.push_back(0x80 | (c & 0x3F));
		}
	}
	
	return txt;
}
