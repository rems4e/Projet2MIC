uniform vec2 _dim;
uniform vec2 _ecran;

attribute vec2 texCoord;
attribute vec2 vertCoord;
attribute vec4 color;

varying vec2 _texCoord;
varying vec4 _color;

uniform float angle;
uniform vec3 axe;
uniform vec3 position;

void main(void) {
	gl_Position = vec4(vertCoord.x, vertCoord.y , 0.1, 1.0);

	gl_Position.xyz += position * 2.0;
	
	vec3 u = normalize(axe);

	float c = cos(angle), s = sin(angle);
	mat3 mat = mat3(
			vec3(u.x * u.x + (1.0 - u.x * u.x) * c, u.x * u.y * (1.0 - c) + u.z * s, u.x * u.z * (1.0 - c) - u.y * s),
			vec3(u.x * u.y * (1.0 - c) - u.z * s, u.y * u.y + (1.0 - u.y * u.y) * c, u.y * u.z * (1.0 - c) + u.x * s),
			vec3(u.x * u.z * (1.0 - c) + u.y * s, u.y * u.z * (1.0 - c) - u.x * s, u.z * u.z + (1.0 - u.z * u.z) * c)
			);
		
	gl_Position.xyz = mat * gl_Position.xyz;
	gl_Position.xyz -= position.xyz * 2.0;

	_color = color;
	_texCoord = texCoord;
}