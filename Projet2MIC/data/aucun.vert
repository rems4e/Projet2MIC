uniform vec2 _dim;
uniform vec2 _ecran;

attribute vec2 texCoord;
attribute vec2 vertCoord;
attribute vec4 color;

varying vec2 _texCoord;
varying vec4 _color;

void main(void) {
	gl_Position = vec4(vertCoord.x, vertCoord.y , 0.1, 1.0);
	gl_Position.z = 0.1;
	
	_color = color;
	_texCoord = /*(gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;//*/texCoord;
}