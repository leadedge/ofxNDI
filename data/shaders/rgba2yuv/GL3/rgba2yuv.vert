#version 150

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in vec2 texcoord;

// Coords for the fragment shader
out vec2 texCoord;

void main()
{
	texCoord = texcoord;
    // send the vertices to the fragment shader
	gl_Position = modelViewProjectionMatrix * position;
}