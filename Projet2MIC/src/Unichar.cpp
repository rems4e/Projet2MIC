/*
 *  Unichar.cpp
 *
 *  Created by Rémi on 06/05/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

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

Unichar::Unichar(char const *txt, size_t nbCar) : _unitxt(), _utf8(), _utf8AJour(false) {
	this->init(txt, nbCar);
}

Unichar::Unichar(std::string const &txt) : _unitxt(), _utf8(), _utf8AJour(false) {
	this->init(txt.c_str(), txt.size());
}

Unichar::Unichar(Unichar const &txt) : _unitxt(txt._unitxt), _utf8(txt._utf8), _utf8AJour(txt._utf8AJour) {
}

Unichar &Unichar::operator=(Unichar const &txt) {
	_unitxt = txt._unitxt;
	_utf8AJour = txt._utf8AJour;
	if(_utf8AJour)
		_utf8 = txt._utf8;
	
	return *this;
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
	_utf8.assign(txt, nbCar);
	_utf8AJour = true;

	for(uindex_t i = 0; i < nbCar; ++i) {
		unichar c = reinterpret_cast<unsigned char const *>(txt)[i];
		if(c >= 0xF0) {
			c = static_cast<unichar>(txt[i] & 0x07) << 18;
			c |= static_cast<unichar>(txt[++i] & 0x3F) << 12;
			c |= static_cast<unichar>(txt[++i] & 0x3F) << 6;
			c |= static_cast<unichar>(txt[++i] & 0x3F);
		}
		else if(c >= 0xE0) {
			c = static_cast<unichar>(txt[i] & 0x0F) << 12;
			c |= static_cast<unichar>(txt[++i] & 0x3F) << 6;
			c |= static_cast<unichar>(txt[++i] & 0x3F);
		}
		else if(c >= 0xC0) {
			c = static_cast<unichar>(txt[i] & 0x1F) << 6;
			c |= static_cast<unichar>(txt[++i] & 0x3F);
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
	size_t taille = 0;
	for(unsigned char c = *reinterpret_cast<unsigned char const *>(utf8txt); *utf8txt; ++utf8txt, ++taille) {
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
	
	return taille;	
}

void Unichar::replace(iterator i1, iterator i2, Unichar const &txt) {
	iterator i = _unitxt.erase(i1, i2);
	_unitxt.insert(i, txt.begin(), txt.end());
	_utf8AJour = false;
}

void Unichar::replace(iterator i1, iterator i2, std::string const &txt) {
	Unichar t(txt);
	iterator i = _unitxt.erase(i1, i2);
	_unitxt.insert(i, t.begin(), t.end());
	_utf8AJour = false;
}

void Unichar::replace(iterator i1, iterator i2, unichar c) {
	iterator i = _unitxt.erase(i1, i2);
	_unitxt.insert(i, c);
	_utf8AJour = false;
}

void Unichar::calcUtf8() const {
	_utf8.clear();
	for(uindex_t i = 0; i < _unitxt.size(); ++i) {
		int c = _unitxt[i];
		
		if(c < 0x80) {
			_utf8.push_back(c);
		}
		else if(c < 0x800) {
			_utf8.push_back(0xC0 | c >> 6);
			_utf8.push_back(0x80 | (c & 0x3F));
		}
		else if(c < 0x10000) {
			_utf8.push_back(0xE0 | c >> 12);
			_utf8.push_back(0x80 | (c >> 6 & 0x3F));
			_utf8.push_back(0x80 | (c & 0x3F));
		}
		else if(c < 0x200000) {
			_utf8.push_back(0xF0 | c >> 18);
			_utf8.push_back(0x80 | (c >> 12 & 0x3F));
			_utf8.push_back(0x80 | (c >> 6 & 0x3F));
			_utf8.push_back(0x80 | (c & 0x3F));
		}
	}	
}
