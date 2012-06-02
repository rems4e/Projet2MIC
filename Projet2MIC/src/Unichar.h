//
//  Unichar.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef UNICHAR_H
#define UNICHAR_H

#include <string>
#include <vector>
#include <iostream>
#include "Constantes.h"

class Unichar {
public:
	typedef unsigned int unichar;
	typedef std::vector<unichar>::iterator iterator;
	typedef std::vector<unichar>::const_iterator const_iterator;
	typedef std::vector<unichar>::reverse_iterator reverse_iterator;
	typedef std::vector<unichar>::const_reverse_iterator const_reverse_iterator;

	static size_t unisize(char const *utf8txt);

	static Unichar const uninull;
	
	Unichar(char const * txt = 0, size_t nbCar = (size_t)-1);
	Unichar(std::string const &txt);
	
	template<typename _InputIterator>
	Unichar(_InputIterator __first, _InputIterator __last) : _unitxt(__first, __last), _txt(this->calcUtf8()) { }
	
	Unichar(Unichar const &txt);
	
	~Unichar();
	
	inline unichar *unitxt() { return this->size() ? &_unitxt[0] : 0; }
	inline unichar const *unitxt() const { return this->size() ? &_unitxt[0] : 0; }
	inline unichar operator[](uindex_t pos) const { return _unitxt[pos]; }
	inline size_t size() const { return _unitxt.size(); }
	inline bool empty() const { return !this->size(); }
	
	inline std::string const &utf8() const { return _txt; }
	
	inline bool operator==(Unichar const &u) const { return _unitxt == u._unitxt; }
	inline bool operator==(std::string const &t) const  { return _txt == t; }
	inline bool operator==(char const *t) const  { return _txt == t; }
	
	inline bool operator!=(Unichar const &u) const { return !(_unitxt == u._unitxt); }
	inline bool operator!=(std::string const &t) const  { return !(_txt == t); }
	inline bool operator!=(char const *t) const  { return !(_txt == t); }
	
	inline bool operator<(Unichar const &u) const { return _txt < u._txt; }
	
	inline Unichar operator+(char const *u) const { Unichar uu(*this); return uu += u; }
	inline Unichar &operator+=(char const *u) { this->replace(this->end(), this->end(), std::string(u)); return *this; }

	inline Unichar operator+(std::string const &u) const { Unichar uu(*this); return uu += u; }
	inline Unichar &operator+=(std::string const &u) { this->replace(this->end(), this->end(), u); return *this; }

	inline Unichar operator+(Unichar const &u) const { Unichar uu(*this); return uu += u; }
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
	void replace(iterator i1, iterator i2, char c);

	inline void replace(uindex_t p, size_t n, std::string const &txt) { this->replace(_unitxt.begin() + p, _unitxt.begin() + p + n, txt); }
	inline void replace(uindex_t p, size_t n, char c) { this->replace(_unitxt.begin() + p, _unitxt.begin() + p + n, c); }
	
	inline operator std::string() const { return _txt; }
	
protected:
	std::vector<unichar> _unitxt;
	std::string _txt;
	
	void init(char const *txt, size_t nbCar);
	std::string calcUtf8() const;
};

inline bool operator==(std::string const &t, Unichar const &u) { return u == t; }

inline std::ostream &operator<<(std::ostream &o, Unichar const &u) {
	return o << u.utf8();
}

#endif
