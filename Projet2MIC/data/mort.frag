#version 120

uniform sampler2D _tex;
uniform vec2 _dim;
uniform vec2 _ecran;

varying vec2 _texCoord;
varying vec4 _color;

uniform float temps;
uniform float duree;

void main(void) {
	gl_FragColor = _color;
	
	float perturbation = sin(cos(gl_FragCoord.x / 20 + 20*cos(temps)) + sqrt(gl_FragCoord.x));
	float avancement = min(1.0, temps / duree);
	
	if(gl_FragCoord.y > (_ecran.y - avancement * _ecran.y) + 20 * (perturbation - 1)) {
		
		float x = gl_FragCoord.x;
		float y = gl_FragCoord.y;
		float mov0 = x + y + cos(sin(temps / 2) * 2.0) * 100.0 + sin(x / 100.0) * 1000.0;
		float mov1 = y / _ecran.y / 0.2 + temps / 2;
		float mov2 = x / _ecran.x / 0.2;
		float c1 = abs(sin(mov1 + temps / 2) / 2.0 + mov2 / 2.0 - mov1 - mov2 + temps / 2);
		float c2 = abs(sin(c1 + sin(mov0 / 1000.0 + temps / 2) + sin(y / 40.0 + temps / 2) + sin((x + y) / 100.0) * 3.0));
		gl_FragColor = vec4((c2 + 1.0) / 3, 0.0, 0.0, 0.6);
	}
	else
		gl_FragColor.a = 0.0;
}