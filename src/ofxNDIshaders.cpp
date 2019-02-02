/*
	Colour conversion shader functions

	using the NDI SDK to receive frames from the network

	http://NDI.NewTek.com

	Copyright (C) 2016-2019 Lynn Jarvis.

	http://www.spout.zeal.co

	=========================================================================
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================

	11.04.18 - Create file
			 - rgba to yuv shader : NDIlib_FourCC_type_UYVY
	12.04.18 - Add rgba2bgra

*/
#include "ofxNDIshaders.h"

//
// Load the shaders for rgba - yuv conversion
//
// rgba2yuv : Openframeworks RGBA to NDIlib_FourCC_type_UYVY
//
ofxNDIshaders::ofxNDIshaders()
{
	//
	// Sender : RGBA to YUV422
	//

	std::string rgba2yuvVertGL2 = STRINGIFY(
		void main() {
			gl_TexCoord[0] = gl_MultiTexCoord0;
			gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		}
	);

	std::string rgba2yuvFragGL2 = STRINGIFY(

		//
		//     RGBA to YUV422
		//
		// Y sampled at every pixel
		// U and V sampled at every second pixel 
		//

		#extension GL_ARB_texture_rectangle : enable\n

		uniform sampler2DRect tex; // fbo texture to draw to
		uniform sampler2DRect rgbatex; // rgba source texture

		void main(void) {

			vec2 currentPosition = gl_TexCoord[0].xy;

			// RGBA texture is twice as wide as the source YUV texture
			vec4 rgba0 = texture2DRect(rgbatex, vec2(currentPosition.x * 2., currentPosition.y));
			vec4 rgba1 = texture2DRect(rgbatex, vec2(currentPosition.x * 2. + 1., currentPosition.y));
					
			// BT.709
			// ITU.BT-709 HDTV studio production in Y'CbCr
			//
			// http://www.martinreddy.net/gfx/faqs/colorconv.faq
			//
			// Y'= 0.2215*R' + 0.7154*G' + 0.0721*B'
			// Cb=-0.1145*R' - 0.3855*G' + 0.5000*B'
			// Cr= 0.5016*R' - 0.4556*G' - 0.0459*B'

			// NDI documentation :
			// SD - BT.609
			// HD - Rec.709
			// UHD - Rec.2020

			float y0 = 0.2215*rgba0.r + 0.7154*rgba0.g + 0.0721*rgba0.b;
			float y1 = 0.2215*rgba1.r + 0.7154*rgba1.g + 0.0721*rgba1.b;
			float u  = -0.1145*rgba0.r - 0.3855*rgba0.g + 0.5000*rgba0.b;
			float v  = 0.5016*rgba0.r - 0.4556*rgba0.g - 0.0459*rgba0.b;

			// Convert Y from 0-255 to 16-235
			// 0 - 1  to  0.06274 - 0.92156
			// y = (y/1.16438)+0.06274
			y0 = y0 / 1.16438 + 0.06274;
			y1 = y1 / 1.16438 + 0.06274;

			// Adjust u and v to 0-1 range
			u += 0.5;
			v += 0.5;

			// u y0 v y1
			vec4 yuv422 = vec4(0.0);
			yuv422.x = u;
			yuv422.y = y0;
			yuv422.z = v;
			yuv422.w = y1;

			gl_FragColor = yuv422;

		}
	);


	std::string rgba2yuvVertGL3 = STRINGIFY(

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
	);

	std::string rgba2yuvFragGL3 = STRINGIFY(

		#extension GL_ARB_texture_rectangle : enable\n

		uniform sampler2DRect tex; // fbo texture to draw to
		uniform sampler2DRect rgbatex; // rgba source texture

		in vec2 texCoord;
		out vec4 outputColor;

		void main()
		{
			
			// RGBA texture is twice as wide as the source YUV texture
			vec4 rgba0 = texture2DRect(rgbatex, vec2(texCoord.x * 2., texCoord.y));
			vec4 rgba1 = texture2DRect(rgbatex, vec2(texCoord.x * 2. + 1., texCoord.y));

			// BT.709
			float y0 = 0.2215*rgba0.r + 0.7154*rgba0.g + 0.0721*rgba0.b;
			float y1 = 0.2215*rgba1.r + 0.7154*rgba1.g + 0.0721*rgba1.b;
			float u = -0.1145*rgba0.r - 0.3855*rgba0.g + 0.5000*rgba0.b;
			float v = 0.5016*rgba0.r - 0.4556*rgba0.g - 0.0459*rgba0.b;

			// Convert Y from 0-255 to 16-235
			y0 = y0 / 1.16438 + 0.06274;
			y1 = y1 / 1.16438 + 0.06274;

			// Adjust u and v to 0-1 range
			u += 0.5;
			v += 0.5;

			// u y0 v y1
			vec4 yuv422 = vec4(0.0);
			yuv422.x = u;
			yuv422.y = y0;
			yuv422.z = v;
			yuv422.w = y1;

			outputColor = yuv422;

		}

	});

	std::string rgba2yuvVertES2 = STRINGIFY(

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
	);

	std::string rgba2yuvFragES2 = STRINGIFY(

		#extension GL_ARB_texture_rectangle : enable\n

		//
		// TARGET_OPENGLES : Untested
		//

		precision highp float;

		uniform sampler2D tex; // fbo texture to draw to
		uniform sampler2D rgbatex; // rgba source texture
		vec2 texCoord; // Texture coords from the vertex shader

		void main()
		{

			// RGBA texture is twice as wide as the source YUV texture
			vec4 rgba0 = texture2D(rgbatex, vec2(texCoord.x * 2., texCoord.y));
			vec4 rgba1 = texture2D(rgbatex, vec2(texCoord.x * 2. + 1., texCoord.y));

			// BT.709
			float y0 = 0.2215*rgba0.r + 0.7154*rgba0.g + 0.0721*rgba0.b;
			float y1 = 0.2215*rgba1.r + 0.7154*rgba1.g + 0.0721*rgba1.b;
			float u = -0.1145*rgba0.r - 0.3855*rgba0.g + 0.5000*rgba0.b;
			float v = 0.5016*rgba0.r - 0.4556*rgba0.g - 0.0459*rgba0.b;

			// Convert Y from 0-255 to 16-235
			y0 = y0 / 1.16438 + 0.06274;
			y1 = y1 / 1.16438 + 0.06274;

			// Adjust u and v to 0-1 range
			u += 0.5;
			v += 0.5;

			// u y0 v y1
			vec4 yuv422 = vec4(0.0);
			yuv422.x = u;
			yuv422.y = y0;
			yuv422.z = v;
			yuv422.w = y1;

			gl_FragColor = yuv422;
		}
	);

#ifdef TARGET_OPENGLES
	rgba2yuvShader.setupShaderFromSource(GL_VERTEX_SHADER, rgba2yuvVertES2, "");
	rgba2yuvShader.setupShaderFromSource(GL_FRAGMENT_SHADER, rgba2yuvFragES2, "");
#else
	if (ofIsGLProgrammableRenderer()) {
		rgba2yuvShader.setupShaderFromSource(GL_VERTEX_SHADER, rgba2yuvVertGL3, "");
		rgba2yuvShader.setupShaderFromSource(GL_FRAGMENT_SHADER, rgba2yuvFragGL3, "");
	}
	else {
		rgba2yuvShader.setupShaderFromSource(GL_VERTEX_SHADER, rgba2yuvVertGL2, "");
		rgba2yuvShader.setupShaderFromSource(GL_FRAGMENT_SHADER, rgba2yuvFragGL2, "");
	}
#endif

	if(!rgba2yuvShader.linkProgram())
		printf("RGBA to YUV shader link failure\n");

	
	//
	// Sender : RGBA to BGRA (or vice versa)
	//
	// Thanks to Harvey Buchan : https://github.com/Harvey3141
	//
	std::string rgba2bgraVertES2 = STRINGIFY(
		uniform mat4 modelViewProjectionMatrix;
		attribute vec4 position;
		attribute vec2 texcoord;
		vec2 texCoord;
		void main()
		{
			texCoord = texcoord;
			gl_Position = modelViewProjectionMatrix * position;
		}
	);

	std::string rgba2bgraFragES2 = STRINGIFY(

		//
		// TARGET_OPENGLES : Untested
		//
		#extension GL_ARB_texture_rectangle : enable\n

		precision highp float;
		uniform sampler2D tex;  // Input texture
		vec2 texCoord; // Texture coords from the vertex shader
		void main()
		{
			vec4 colour = texture2D(tex, texCoord);
			vec4 colourInverted = vec4(colour.b, colour.g, colour.r, colour.a);
			gl_FragColor = colourInverted;
		}
	);

	std::string rgba2bgraVertGL2 = STRINGIFY(
		void main()
		{
			gl_TexCoord[0] = gl_MultiTexCoord0;
			gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		}
	);

	std::string rgba2bgraFragGL2 = STRINGIFY(

		#extension GL_ARB_texture_rectangle : enable\n

		uniform sampler2DRect tex;

		void main()
		{
			vec2 currentPosition = gl_TexCoord[0].st;
			vec4 colour = texture2DRect(tex, currentPosition);
			vec4 colourInverted = vec4(colour.b, colour.g, colour.r, colour.a);
			gl_FragColor = colourInverted;
		}
	);

	std::string rgba2bgraVertGL3 = STRINGIFY(

		uniform mat4 modelViewProjectionMatrix;
		in vec4 position;
		in vec2 texcoord;
		out vec2 texCoord;
		void main()
		{
			texCoord = texcoord;
			gl_Position = modelViewProjectionMatrix * position;
		}
	);

	std::string rgba2bgraFragGL3 = STRINGIFY(

		#extension GL_ARB_texture_rectangle : enable\n
		
		uniform sampler2DRect tex;  // Input texture
		in vec2 texCoord;
		out vec4 outputColor;
		void main()
		{
			vec4 colour = texture2DRect(tex, texCoord);
			vec4 colourInverted = vec4(colour.b, colour.g, colour.r, colour.a);
			outputColor = colourInverted;
		}
	);


#ifdef TARGET_OPENGLES
	rgba2bgra.setupShaderFromSource(GL_VERTEX_SHADER, rgba2bgraVertES2, "");
	rgba2bgra.setupShaderFromSource(GL_FRAGMENT_SHADER, rgba2bgraFragES2, "");
#else
	if (ofIsGLProgrammableRenderer()) {
		rgba2bgra.setupShaderFromSource(GL_VERTEX_SHADER, rgba2bgraVertGL3, "");
		rgba2bgra.setupShaderFromSource(GL_FRAGMENT_SHADER, rgba2bgraFragGL3, "");
	}
	else {
		rgba2bgra.setupShaderFromSource(GL_VERTEX_SHADER, rgba2bgraVertGL2, "");
		rgba2bgra.setupShaderFromSource(GL_FRAGMENT_SHADER, rgba2bgraFragGL2, "");
	}
#endif

	if (!rgba2bgra.linkProgram())
		printf("RGBA to BGRA shader link failed\n");

}

ofxNDIshaders::~ofxNDIshaders()
{

}
