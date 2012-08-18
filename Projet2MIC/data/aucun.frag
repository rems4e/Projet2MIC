#version 120

uniform sampler2D _tex;
uniform vec2 _ecran;

varying vec2 _texCoord;
varying vec4 _color;

void main(void) {
	vec4 couleur = _color * texture2D(_tex, _texCoord);

	gl_FragColor = couleur;
}