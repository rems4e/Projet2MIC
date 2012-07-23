/*
 *  Unichar.h
 *
 *  Created by Rémi on 06/05/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef UNICHAR_H
#define UNICHAR_H

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <sstream>
#include <float.h>
#include "nombre.h"

class Unichar {
public:
	typedef unsigned int unichar;
	typedef std::vector<unichar>::iterator iterator;
	typedef std::vector<unichar>::const_iterator const_iterator;
	typedef std::vector<unichar>::reverse_iterator reverse_iterator;
	typedef std::vector<unichar>::const_reverse_iterator const_reverse_iterator;
	
	typedef unsigned long uindex_t;

	static size_t unisize(char const *utf8txt);

	static Unichar const uninull;
	
	Unichar(char const * txt = 0, size_t nbCar = (size_t)-1);
	Unichar(std::string const &txt);
	
	template<typename _InputIterator>
	Unichar(_InputIterator __first, _InputIterator __last) : _unitxt(__first, __last), _utf8(), _utf8AJour(false) { }	
	Unichar(Unichar const &txt);
	Unichar &operator=(Unichar const &txt);
	
	// Création avec un format
	template<typename... Args>
	Unichar(Unichar const &format, Args... args) : _unitxt(), _utf8(), _utf8AJour(false) {
		Unichar::insertion(format.begin(), format.end(), *this, args...);
	}
	
	Unichar(Unichar const &format, Unichar const &arg1) : _unitxt(), _utf8(), _utf8AJour(false) {
		Unichar::insertion(format.begin(), format.end(), *this, arg1);
	}
	
	~Unichar();
	
	inline unichar *unitxt() { return this->size() ? &_unitxt[0] : 0; }
	inline unichar const *unitxt() const { return this->size() ? &_unitxt[0] : 0; }
	inline unichar operator[](uindex_t pos) const { return _unitxt[pos]; }
	inline size_t size() const { return _unitxt.size(); }
	inline bool empty() const { return !this->size(); }
	
	inline std::string const &utf8() const {
		if(!_utf8AJour)
			this->calcUtf8();
		return _utf8;
	}
	
	inline bool operator==(Unichar const &u) const { return _unitxt == u._unitxt; }
	inline bool operator==(std::string const &t) const  {
		if(!_utf8AJour)
			this->calcUtf8();
		return _utf8 == t;
	}
	inline bool operator==(char const *t) const  {
		if(!_utf8AJour)
			this->calcUtf8();
		return _utf8 == t;
	}
	
	inline bool operator!=(Unichar const &u) const { return !(*this == u); }
	inline bool operator!=(std::string const &t) const  { return !(*this == t); }
	inline bool operator!=(char const *t) const  { return !(*this == t); }
	
	inline bool operator<(Unichar const &u) const { return _unitxt < u._unitxt; }
	
	inline Unichar &operator+=(char const *u) { this->replace(this->end(), this->end(), std::string(u)); return *this; }
	inline Unichar &operator+=(std::string const &u) { this->replace(this->end(), this->end(), u); return *this; }
	inline Unichar &operator+=(Unichar const &u) { this->replace(this->end(), this->end(), u); return *this; }

	inline iterator begin() { return _unitxt.begin(); }
	inline const_iterator begin() const { return _unitxt.begin(); }
	inline iterator end() { return _unitxt.end(); }
	inline const_iterator end() const { return _unitxt.end(); }
	
	inline reverse_iterator rbegin() { return _unitxt.rbegin(); }
	inline const_reverse_iterator rbegin() const { return _unitxt.rbegin(); }
	inline reverse_iterator rend() { return _unitxt.rend(); }
	inline const_reverse_iterator rend() const { return _unitxt.rend(); }
	
	Unichar substr(unsigned long i, size_t n) const { return Unichar(this->begin() + i, this->begin() + i + n); }
	void replace(iterator i1, iterator i2, Unichar const &txt);
	void replace(iterator i1, iterator i2, std::string const &txt);
	void replace(iterator i1, iterator i2, char const *txt) { this->replace(i1, i2, std::string(txt)); }
	void replace(iterator i1, iterator i2, char c) { this->replace(i1, i2, static_cast<unichar>(c)); }
	void replace(iterator i1, iterator i2, unichar c);

	inline void replace(uindex_t p, size_t n, std::string const &txt) { this->replace(_unitxt.begin() + p, _unitxt.begin() + p + n, txt); }
	inline void replace(uindex_t p, size_t n, char c) { this->replace(_unitxt.begin() + p, _unitxt.begin() + p + n, c); }
	
	inline operator std::string() const {
		if(!_utf8AJour)
			this->calcUtf8();
		return _utf8;
	}
		
protected:
	std::vector<unichar> _unitxt;
	mutable std::string _utf8;
	mutable bool _utf8AJour;
	
	void init(char const *txt, size_t nbCar);
	void calcUtf8() const;
	
	static void insertion(Unichar::const_iterator debutFormat, Unichar::const_iterator const finFormat, Unichar &txt) {
		while(debutFormat != finFormat) {
			if(*debutFormat == '%' && *(++debutFormat) != '%') {
				txt.replace(txt.end(), txt.end(), "<%@>");
				++debutFormat;
			}
			else {
				txt.replace(txt.end(), txt.end(), *debutFormat++);
			}
		}
	}
	
	template<typename T, typename... Args>
	static void insertion(Unichar::const_iterator debutFormat, Unichar::const_iterator const finFormat, Unichar &txt, T const &value, Args... args) {
		while(debutFormat != finFormat) {
			if(*debutFormat == '%' && *(++debutFormat) != '%') {
				ajouter(txt, value);
				++debutFormat;
				insertion(debutFormat, finFormat, txt, args...); // call even when *s == 0 to detect extra arguments
				return;
			}
			txt.replace(txt.end(), txt.end(), *debutFormat++);
		}
	}

	template<typename T>
	static void ajouter(Unichar &u, T const &valeur) {
		u += valeur;
	}
};

inline bool operator==(std::string const &t, Unichar const &u) { return u == t; }

inline Unichar operator+(char const *s, Unichar const &u) { Unichar uu(s); return uu += u; }
inline Unichar operator+(std::string const &s, Unichar const &u) { Unichar uu(s); return uu += u; }
inline Unichar operator+(Unichar const &u1, Unichar const & u2) { Unichar uu(u1); return uu += u2; }

inline Unichar operator+(Unichar const &u, char const *s) { Unichar uu(u); return uu += s; }
inline Unichar operator+(Unichar const &u, std::string const &s) { Unichar uu(u); return uu += s; }

inline std::ostream &operator<<(std::ostream &o, Unichar const &u) {
	return o << u.utf8();
}

template<>
inline void Unichar::ajouter(Unichar &u, double const &valeur) {
	u += nombreVersTexte(valeur, nombreDecimales(valeur));
}

template<>
inline void Unichar::ajouter(Unichar &u, float const &valeur) {
	u += nombreVersTexte(valeur, nombreDecimales(valeur));
}

template<>
inline void Unichar::ajouter(Unichar &u, signed int const &valeur) {
	u += nombreVersTexte(valeur);
}

template<>
inline void Unichar::ajouter(Unichar &u, unsigned int const &valeur) {
	u += nombreVersTexte(valeur);
}

template<>
inline void Unichar::ajouter(Unichar &u, signed long const &valeur) {
	u += nombreVersTexte(valeur);
}

template<>
inline void Unichar::ajouter(Unichar &u, unsigned long const &valeur) {
	u += nombreVersTexte(valeur);
}

template<>
inline void Unichar::ajouter(Unichar &u, char const &valeur) {
	u.replace(u.end(), u.end(), valeur);
}

template<>
inline void Unichar::ajouter(Unichar &u, unsigned char const &valeur) {
	u += nombreVersTexte(valeur);
}

template<>
inline void Unichar::ajouter(Unichar &u, signed char const &valeur) {
	u += nombreVersTexte(valeur);
}

#endif
