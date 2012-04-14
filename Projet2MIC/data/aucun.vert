uniform vec2 _pos;
uniform vec2 _dim;
uniform vec2 _ecran;

void main(void) {
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
	gl_Position.z = 0.1;
}