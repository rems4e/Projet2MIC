#version 120

uniform sampler2D _tex;
uniform vec2 _ecran;

uniform float _volume;
uniform vec4 _cadre;

varying vec2 _texCoord;
varying vec4 _color;

void main(void) {
	vec4 couleur = _color * texture2D(_tex, _texCoord);
	vec2 dec = (_cadre.xy + vec2(1.0, 1.0)) / 2.0;
	vec2 position = (gl_FragCoord.xy / _ecran - dec) / _cadre.zw;
	couleur.rgb = vec3(1.0 - pow(position.x, 3) / 2.0, 1.0 - pow(position.x, 3) / 2.0, 1.0);
	position.y += 1.0;
	
	if(position.x > _volume)
		couleur.a = 0.0;
	
	gl_FragColor = couleur;
}