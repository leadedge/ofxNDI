//
// GL3
//

#version 150

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in vec2 texcoord;

// Output coords to fragment shader
out vec2 vTexCoord;

void main()
{
    vTexCoord = texcoord;
    gl_Position = modelViewProjectionMatrix * position;
}
