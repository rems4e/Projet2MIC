//
//  test.h
//  Projet2MIC
//
//  Created by Rémi on 31/07/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef __Projet2MIC__test__
#define __Projet2MIC__test__

#include "Session.h"
#include "VueInterface.h"
#include "Image.h"

void test();

class VueTest : public VueInterface {
public:
	VueTest();
	virtual ~VueTest();
	
protected:
	
	void preparationDessin() override;
	void dessiner() override;
	void gestionClavier() override;

private:
	Bitmap _img1;
	Shader _shader;
	GLuint _vbo;
	std::vector<GLfloat> _vert;
	std::vector<GLfloat> _norm;
	std::vector<GLubyte> _coul;
	
	index_t _nbSommets;
};

#endif /* defined(__Projet2MIC__test__) */
