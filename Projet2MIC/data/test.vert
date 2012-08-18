#version 120

uniform vec2 _ecran;

uniform mat4 _matProjection;
uniform mat4 _matModeleVue;

attribute vec2 texCoord;
attribute vec3 vertCoord;
attribute vec4 color;
attribute vec3 norm;

varying vec2 _texCoord;
varying vec4 _color;
varying vec3 _normale;
varying vec3 _positionModele;

void main() {
	gl_Position = _matProjection * (_matModeleVue * vec4(vertCoord, 1.0));

	_texCoord = texCoord;
	_color = color;
	_normale = norm;
	_positionModele = vertCoord;
}
