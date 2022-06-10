
// these are for the programmable pipeline system
uniform mat4 modelViewProjectionMatrix;
attribute vec4 position;
attribute vec2 texcoord;

// Coords for the fragment shader
vec2 texCoord;

void main()
{
	texCoord = texcoord;
    // send the vertices to the fragment shader
	gl_Position = modelViewProjectionMatrix * position;
}