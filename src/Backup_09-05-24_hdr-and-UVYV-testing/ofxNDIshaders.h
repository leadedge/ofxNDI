/*

				ofxNDIshaders.h

		Functions to manage compute shaders

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Copyright (c) 2024, Lynn Jarvis. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#pragma once
#ifndef __ofxNDIshaders__
#define __ofxNDIshaders__

#include <windows.h>
#include <algorithm> // for std::replace
// #include "..\apps\SpoutGL\SpoutGLextensions.h"
// #include "..\apps\SpoutGL\SpoutCommon.h"
// #include "..\apps\SpoutGL\SpoutUtils.h"

// using namespace spoututils;

//
// Shader resource re-definitions
//
// Definitions here are duplicated from SpoutShaders.rc
// to avoid a requiring a separate resource header file
#define IDC_SHADERFILE0		1000 // copy
#define IDC_SHADERFILE1		1001 // flip
#define IDC_SHADERFILE2		1002 // mirror
#define IDC_SHADERFILE3		1003 // swap
#define IDC_SHADERFILE4		1004 // temp
#define IDC_SHADERFILE5		1005 // brcosa
#define IDC_SHADERFILE6		1006 // hblur
#define IDC_SHADERFILE7		1007 // vblur
#define IDC_SHADERFILE8		1008 // bloom
#define IDC_SHADERFILE9		1009 // sharpen
#define IDC_SHADERFILE10	1010 // cas
#define IDC_SHADERFILE11	1011 // kuwahara

class SPOUT_DLLEXP ofxNDIshaders {

	public:

		ofxNDIshaders();
		~ofxNDIshaders();

		// Texture copy
		bool Copy(GLuint SourceID, GLuint DestID,
			unsigned int width, unsigned int height,
			bool bInvert = false, bool swap = false);

		// Flip image in place
		bool Flip(GLuint SourceID, unsigned int width, unsigned int height, bool bSwap = false);

		// Mirror image in place
		bool Mirror(GLuint SourceID, unsigned int width, unsigned int height, bool bSwap = false);

		// Swap RGBA <> BGRA
		bool Swap(GLuint SourceID, unsigned int width, unsigned int height);

		// Temperature
		bool Temperature(GLuint SourceID, GLuint DestID,
			unsigned int width, unsigned int height, float temp);

		// Image adjust - brightness, contrast, saturation, gamma
		bool Adjust(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float brightness, float contrast, 
			float saturation, float gamma);

		// Gaussian blur
		bool Blur(GLuint SourceID, GLuint DestID,
			unsigned int width, unsigned int height, float amount);

		// Bloom effect
		bool Bloom(GLuint SourceID,
			unsigned int width, unsigned int height, float amount);

		// Unsharp mask sharpen
		bool Sharpen(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float sharpenWidth, float sharpenStrength);

		// Contrast adaptive sharpen
		bool AdaptiveSharpen(GLuint SourceID,
			unsigned int width, unsigned int height, float caswidth, float caslevel);

		// Kuwahara
		bool Kuwahara(GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height, float amount);

		// Shader format
		void SetGLformat(GLint glformat);

	protected :

		void CheckShaderFormat(std::string& shaderstr);
		bool ComputeShader(std::string &shader, GLuint &program, 
			GLuint SourceID, GLuint DestID, 
			unsigned int width, unsigned int height,
			float uniform0 = -1.0, float uniform1 = -1.0,
			float uniform2 = -1.0, float uniform3 = -1.0);
		GLuint CreateComputeShader(std::string shader, unsigned int nWgX, unsigned int nWgY);
		std::string GetFileString(const char* filepath); // Get shader string from file
		std::string LoadResourceFile(int index);  // Load shader string from resources

		// Global program identifiers
		GLuint m_copyProgram     = 0;
		GLuint m_flipProgram     = 0;
		GLuint m_mirrorProgram   = 0;
		GLuint m_swapProgram     = 0;
		GLuint m_tempProgram     = 0;
		GLuint m_brcosaProgram   = 0;
		GLuint m_hBlurProgram    = 0;
		GLuint m_vBlurProgram    = 0;
		GLuint m_sharpenProgram  = 0;
		GLuint m_casProgram      = 0;
		GLuint m_kuwaharaProgram = 0;
		GLuint m_bloomProgram    = 0;

		// Formats
		GLint m_GLformat = GL_RGBA8;
		std::string m_GLformatName = "rgba8";

		// Shader source loaded from resources
		std::string m_copystr;
		std::string m_flipstr;
		std::string m_mirrorstr;
		std::string m_swapstr;
		std::string m_tempstr;
		std::string m_brcosastr;
		std::string m_hblurstr;
		std::string m_vblurstr;
		std::string m_sharpenstr;
		std::string m_casstr;
		std::string m_kuwaharastr;
		std::string m_bloomstr;

};

#endif
