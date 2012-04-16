uniform vec2 _pos;
uniform vec2 _dim;

attribute vec2 texCoord;
attribute vec2 vertCoord;
attribute vec4 color;

varying vec2 _texCoord;
varying vec4 _color;

void main(void) {
	gl_Position = /*gl_ModelViewProjectionMatrix * */vec4(vertCoord.x, vertCoord.y , 0.1, 1.0);
	gl_Position.z = 0.1;
	
	_color = color;
	_texCoord = texCoord;
}