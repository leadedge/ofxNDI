/*
				ofxNDIshaders.cpp

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
	========================

	09.05.24 - first version based on ofxNDIshaders


*/

#include "ofxNDIshaders.h"

//
// Class: ofxNDIshaders
//
// Functions to manage compute shaders
//

ofxNDIshaders::ofxNDIshaders() {

	// Load shaders from files defined in resources
	m_copystr     = LoadResourceFile(0);
	m_flipstr     = LoadResourceFile(1);
	m_mirrorstr   = LoadResourceFile(2);
	m_swapstr     = LoadResourceFile(3);
	m_tempstr     = LoadResourceFile(4);
	m_brcosastr   = LoadResourceFile(5);
	m_hblurstr    = LoadResourceFile(6);
	m_vblurstr    = LoadResourceFile(7);
	m_bloomstr    = LoadResourceFile(8);
	m_sharpenstr  = LoadResourceFile(9);
	m_casstr      = LoadResourceFile(10);
	m_kuwaharastr = LoadResourceFile(11);
}


ofxNDIshaders::~ofxNDIshaders() {

	if (m_copyProgram     > 0) glDeleteProgram(m_copyProgram);
	if (m_swapProgram     > 0) glDeleteProgram(m_swapProgram);
	if (m_flipProgram     > 0) glDeleteProgram(m_flipProgram);
	if (m_mirrorProgram   > 0) glDeleteProgram(m_mirrorProgram);
	if (m_brcosaProgram   > 0) glDeleteProgram(m_brcosaProgram);
	if (m_tempProgram     > 0) glDeleteProgram(m_tempProgram);
	if (m_hBlurProgram    > 0) glDeleteProgram(m_hBlurProgram);
	if (m_vBlurProgram    > 0) glDeleteProgram(m_vBlurProgram);
	if (m_sharpenProgram  > 0) glDeleteProgram(m_sharpenProgram);
	if (m_casProgram      > 0) glDeleteProgram(m_casProgram);
	if (m_kuwaharaProgram > 0) glDeleteProgram(m_kuwaharaProgram);
	if (m_bloomProgram    > 0) glDeleteProgram(m_bloomProgram);
}

//---------------------------------------------------------
// Function: CopyTexture
//
// OpenGL texture copy using compute shader
//    bInvert - flip image
//    bSwap - swap red/blue (RGBA/BGRA)
// Approximately X2 faster than FBO blit
// Textures must have the same size and internal format type GL_RGBA/GL_BGRA/GL_RGBA8
// Texture targets can be different, GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_ARB
// Cannot be used with GL/DX interop
bool ofxNDIshaders::Copy(GLuint SourceID, GLuint DestID,
	unsigned int width, unsigned int height, bool bInvert, bool bSwap)
{
	return ComputeShader(m_copystr, m_copyProgram, SourceID, DestID,
		width, height, bInvert, bSwap);
}

//---------------------------------------------------------
// Function: Flip
//    Flip image in place
bool ofxNDIshaders::Flip(GLuint SourceID, unsigned int width, unsigned int height, bool bSwap)
{
	return ComputeShader(m_flipstr, m_flipProgram, SourceID, 0, width, height, bSwap);
}

//---------------------------------------------------------
// Function: Mirror
//    Mirror image in place
bool ofxNDIshaders::Mirror(GLuint SourceID, unsigned int width, unsigned int height, bool bSwap)
{
	return ComputeShader(m_mirrorstr, m_mirrorProgram, SourceID, 0, width, height);
}

//---------------------------------------------------------
// Function: Swap
//    Texture swap RGBA <> BGRA
bool ofxNDIshaders::Swap(GLuint SourceID, unsigned int width, unsigned int height)
{
	return ComputeShader(m_swapstr, m_swapProgram, SourceID, 0, width, height);
}

//---------------------------------------------------------
// Function: Temperature
//     Colour temperature
//     https://www.shadertoy.com/view/ltlcWN
//     https://en.wikipedia.org/wiki/Color_temperature
//     temp -  3500 - 9500 : default 6500 (daylight)
bool ofxNDIshaders::Temperature(GLuint SourceID, GLuint DestID,
	unsigned int width, unsigned int height, float temp)
{
	return ComputeShader(m_tempstr, m_tempProgram,
		SourceID, DestID, width, height, temp);
}


//---------------------------------------------------------
// Function: Adjust
// Brightness, contrast, saturation, gamma
//    brightness   -1 > 1
//    contrast      0 > 1
//    saturation    0 > 1
//    gamma         0 > 1
bool ofxNDIshaders::Adjust(GLuint SourceID, GLuint DestID, 
	unsigned int width, unsigned int height,
	float brightness, float contrast,
	float saturation, float gamma)
{
	return ComputeShader(m_brcosastr, m_brcosaProgram, SourceID, DestID,
		width, height, brightness, contrast, saturation, gamma);
}

//---------------------------------------------------------
// Function: Blur
//     Two pass Gaussian blur
//     amount - 1 - 4 typical
bool ofxNDIshaders::Blur(GLuint SourceID, GLuint DestID,
	unsigned int width, unsigned int height, float amount)
{
	// Horizontal blur
	if (ComputeShader(m_hblurstr, m_hBlurProgram, SourceID, DestID, width, height, amount)) {
		// Vertical blur on Horizontal blur result
		return ComputeShader(m_vblurstr, m_vBlurProgram, SourceID, DestID, width, height, amount);
	}
	return false;
}

//---------------------------------------------------------
// Function: Bloom
// https://www.shadertoy.com/view/MXsSzB
//     amount - 0.0-1.0
bool ofxNDIshaders::Bloom(GLuint SourceID, unsigned int width, unsigned int height, float amount)
{
	return ComputeShader(m_bloomstr, m_bloomProgram, SourceID, 0, width, height, amount);
}

//---------------------------------------------------------
// Function: Sharpen
// Sharpen using unsharp mask
//     sharpenWidth     - 1 3x3, 2 5x5, 3 7x7
//     sharpenStrength  - 1 - 3 typical
bool ofxNDIshaders::Sharpen(GLuint SourceID, GLuint DestID, 
	unsigned int width, unsigned int height,
	float sharpenWidth, float sharpenStrength)
{
	return ComputeShader(m_sharpenstr, m_sharpenProgram, 
		SourceID, DestID,
		width, height,
		sharpenWidth, sharpenStrength);
}

//---------------------------------------------------------
// Function: AdaptiveSharpen
// Sharpen using Contrast Adaptive sharpe algorithm
//     level - 0 > 1
bool ofxNDIshaders::AdaptiveSharpen(GLuint SourceID,
	unsigned int width, unsigned int height, float caswidth, float caslevel)
{
	return ComputeShader(m_casstr, m_casProgram,
		SourceID, 0, width, height, caswidth, caslevel);
}

//---------------------------------------------------------
// Function: Kuwahara
//     Kuwahara filter
//     amount - 1 - 4 typical
bool ofxNDIshaders::Kuwahara(GLuint SourceID, GLuint DestID, 
	unsigned int width, unsigned int height, float amount)
{
	/*
	// Load shader from file for debugging
	if (m_kuwaharastr.empty()) {
		char exepath[MAX_PATH]={};
		GetModuleFileNameA(NULL, exepath, MAX_PATH);
		PathRemoveFileSpecA(exepath);
		strcat_s(exepath, "\\data\\shaders\\kuwahara.txt");
		m_kuwaharastr = GetFileString(exepath);
		// printf("%s\n", m_kuwaharastr.c_str());
	}
	*/
	return ComputeShader(m_kuwaharastr, m_kuwaharaProgram,
		SourceID, DestID, width, height, amount);
}

//---------------------------------------------------------
// Function: SetGLformat
// Set OpenGL format for shaders
// GL_RGBA8, GL_RGB10_A2, GL_RGBA16, GL_RGBA16F, GL_RGBA32F
// https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
void ofxNDIshaders::SetGLformat(GLint glformat)
{

	SpoutLogNotice("ofxNDIshaders::SetGLformat(0x%X)", glformat);

	// For final notice of change
	GLint oldformat = m_GLformat;
	std::string oldformatname = m_GLformatName;

	m_GLformat = glformat;

	// GL_RGBA not supported
	if (m_GLformat == GL_RGBA)
		m_GLformat = GL_RGBA8;

	switch (m_GLformat) {
		case GL_RGBA8:
			m_GLformatName = "rgba8";
			break;
		case GL_RGB10_A2:
			m_GLformatName = "rgb10_a2";
			break;
		case GL_RGBA16:
			m_GLformatName = "rgba16";
			break;
		case GL_RGBA16F:
			m_GLformatName = "rgba16f";
			break;
		case GL_RGBA32F:
			m_GLformatName = "rgba32f";
			break;
		default:
			break;
	}

	// All shaders have to be re-compiled
	// Shader strings are re-set in the ComputeShader function
	if (m_copyProgram     > 0) glDeleteProgram(m_copyProgram);
	if (m_swapProgram     > 0) glDeleteProgram(m_swapProgram);
	if (m_flipProgram     > 0) glDeleteProgram(m_flipProgram);
	if (m_mirrorProgram   > 0) glDeleteProgram(m_mirrorProgram);
	if (m_brcosaProgram   > 0) glDeleteProgram(m_brcosaProgram);
	if (m_tempProgram     > 0) glDeleteProgram(m_tempProgram);
	if (m_hBlurProgram    > 0) glDeleteProgram(m_hBlurProgram);
	if (m_vBlurProgram    > 0) glDeleteProgram(m_vBlurProgram);
	if (m_sharpenProgram  > 0) glDeleteProgram(m_sharpenProgram);
	if (m_casProgram      > 0) glDeleteProgram(m_casProgram);
	if (m_kuwaharaProgram > 0) glDeleteProgram(m_kuwaharaProgram);
	if (m_bloomProgram    > 0) glDeleteProgram(m_bloomProgram);
	m_copyProgram     = 0;
	m_swapProgram     = 0;
	m_flipProgram     = 0;
	m_mirrorProgram   = 0;
	m_brcosaProgram   = 0;
	m_tempProgram     = 0;
	m_casProgram      = 0;
	m_hBlurProgram    = 0;
	m_vBlurProgram    = 0;
	m_sharpenProgram  = 0;
	m_kuwaharaProgram = 0;
	m_bloomProgram    = 0;

	if (m_GLformat != oldformat) {
		SpoutLogNotice("ofxNDIshaders::SetGLformat - shader format reset from 0x%X (%s) to 0x%X (%s)",
			oldformat, oldformatname.c_str(), m_GLformat, m_GLformatName.c_str());
	}

}


//---------------------------------------------------------
// Function: CheckShaderFormat
// Check shader source for correct format description
void ofxNDIshaders::CheckShaderFormat(std::string& shaderstr)
{
	// Remove comments preceding the shader source
	size_t pos1 = shaderstr.find("layout");
	shaderstr = shaderstr.substr(pos1);

	// Find existing format name "layout(rgba8, ...etc
	pos1 = shaderstr.find("(");
	pos1 += 1; // Skip the '(' character

	// Find the next comma to extract the format name
	size_t pos2 = shaderstr.find(",");
	std::string formatname = shaderstr.substr(pos1, pos2-pos1);

	// Default format name is "rbgb8"
	// "rgba8" "rgba16" "rgba16f" or "rgba32f" supported
	// SetGLformat establishes m_GLformatName from the OpenGL format
	// Replace all occurrences of existing format name with m_GLformatName
	if(formatname != m_GLformatName) {
		// Replace shader format name
		size_t pos = 0;
		while (pos += formatname.length()) {
			pos = shaderstr.find(formatname, pos);
			if (pos == std::string::npos) {
				break;
			}
			shaderstr.replace(pos, formatname.length(), m_GLformatName);
		}
	}
}


//---------------------------------------------------------
// Function: ComputeShader
//    Apply compute shader on source to dest
//    or to read/write source with provided uniforms
bool ofxNDIshaders::ComputeShader(std::string &shaderstr, GLuint &program,
	GLuint SourceID, GLuint DestID, unsigned int width, unsigned int height,
	float uniform0, float uniform1, float uniform2, float uniform3)
{
	if (shaderstr.empty() || SourceID == 0) {
		SpoutLogWarning("ofxNDIshaders::ComputeShader - no shader or texture");
		return false;
	}

	// Find the maximum number of work group units allowed
	int workgroups = 0;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroups);
	// wgx*wgy must be less that max workgroup invocations
	unsigned int nWgX = (unsigned int)sqrt((float)workgroups);
	unsigned int nWgY = nWgX;

	// The X and Y work group sizes should match image width and height
	// Adjust down for aspect ratio
	if (width >= height) {
		nWgX = width/(unsigned int)ceil((float)width/(float)nWgX);
		nWgY = nWgX * height/width;
	}
	else {
		nWgY = height/(unsigned int)ceil((float)height/(float)nWgY);
		nWgX = nWgY * width/height;
	}

	if (program == 0) {
		// Check shader source for correct format name
		CheckShaderFormat(shaderstr);
		program = CreateComputeShader(shaderstr, nWgX, nWgY);
		if (program == 0) {
			SpoutLogWarning("ofxNDIshaders::ComputeShader - CreateComputeShader failed");
			return false;
		}
		SpoutLogNotice("ofxNDIshaders::ComputeShader - created program (%d)", program);

	}

	glUseProgram(program);
	glBindImageTexture(0, SourceID, 0, GL_FALSE, 0, GL_READ_WRITE, m_GLformat);
	if(DestID > 0)
	  glBindImageTexture(1, DestID, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_GLformat);
	if (uniform0 != -1.0) glUniform1f(0, uniform0);
	if (uniform1 != -1.0) glUniform1f(1, uniform1);
	if (uniform2 != -1.0) glUniform1f(2, uniform2);
	if (uniform3 != -1.0) glUniform1f(3, uniform3);
	glDispatchCompute(width/nWgX, height/nWgY, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, m_GLformat);
	glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_GLformat);
	glUseProgram(0);

	return true;

}

//---------------------------------------------------------
// Function: CreateComputeShader
// Create compute shader from a source string
unsigned int ofxNDIshaders::CreateComputeShader(std::string shader, unsigned int nWgX, unsigned int nWgY)
{
	// Compute shaders are only supported since openGL 4.3
	int major = 0;
	int minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	float version = (float)major + (float)minor / 10.0f;
	if (version < 4.3f) {
		SpoutLogError("CreateComputeShader - OpenGL version > 4.3 required");
		return 0;
	}

	//
	// Compute shader source initialized in header
	// See also GetFileString for testing
	//

	// Common
	std::string shaderstr = "#version 440\n";
	shaderstr += "layout(local_size_x = ";
	shaderstr += std::to_string(nWgX);
	shaderstr += ", local_size_y = ";
	shaderstr += std::to_string(nWgY);
	shaderstr += ", local_size_z = 1) in;\n";
	
	// Full shader string
	shaderstr += shader;

	// Create the compute shader program
	GLuint computeProgram = glCreateProgram();
	if (computeProgram > 0) {
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		if (computeShader > 0) {
			// Compile and link shader
			GLint status = 0;
			const char* source = shaderstr.c_str();
			glShaderSource(computeShader, 1, &source, NULL);
			glCompileShader(computeShader);
			glAttachShader(computeProgram, computeShader);
			glLinkProgram(computeProgram);

			// Link error log
			GLsizei length = 0;
			glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &length);
			if (length > 0) {
				char infoLog[1024]{};
				glGetProgramInfoLog(computeProgram, 1024, &length, infoLog);
				if (length > 0)
					SpoutLogWarning("CreateComputeShader - link error log\n%s", infoLog);
			}

			glGetProgramiv(computeProgram, GL_LINK_STATUS, &status);
			if (status == GL_FALSE) {
				SpoutLogError("CreateComputeShader - linking failed");
				glDetachShader(computeProgram, computeShader);
				glDeleteProgram(computeShader);
				glDeleteProgram(computeProgram);
				return 0;
			}
			else {
				// After linking, the shader object is not needed
				glDeleteShader(computeShader);
				return computeProgram;
			}
		}
		else {
			SpoutLogError("CreateComputeShader - glCreateShader failed");
			return 0;
		}
	}
	SpoutLogError("CreateComputeShader - glCreateProgram failed");
	return 0;
}


//---------------------------------------------------------
// Function: GetFileString
// Load shader source from file
// Used for development.
std::string ofxNDIshaders::GetFileString(const char* filepath)
{
	if (!filepath || !*filepath) return "";

	std::string path = filepath;
	std::string logstr = "";

	if (!path.empty()) {
		// Does the file exist
		if (_access(path.c_str(), 0) != -1) {
			// Open the file
			std::ifstream logstream(path);
			// File loaded OK ?
			if (logstream.is_open()) {
				// Get the file text as a single string
				logstr.assign((std::istreambuf_iterator< char >(logstream)), std::istreambuf_iterator< char >());
				logstr += ""; // ensure a NULL terminator
				logstream.close();
			}
		}
		else {
			SpoutLogError("GetFileString [%s] not found", path.c_str());
		}
	}
	return logstr;
}

//---------------------------------------------------------
// Function: LoadResourceFile
// Load shader source from resources
//		copy
//		flip
//		mirror
//		swap
//		temp
//		brcosa
//		hblur
//		vblur
//      bloom
//		sharpen
//		cas
//		kuwahara
//
std::string ofxNDIshaders::LoadResourceFile(int index)
{
	HMODULE hModule = NULL;
	DWORD size = 0;
	HRSRC hResInfo = NULL;
	HGLOBAL rcImageData = NULL;
	const char* imagedata = NULL;
	std::string shaderstring="";

	hModule = GetCurrentModule();
	if (hModule) {
		hResInfo = FindResource(hModule, MAKEINTRESOURCE(IDC_SHADERFILE0+index), RT_RCDATA);
		if (hResInfo) {
			size = SizeofResource(hModule, hResInfo);
			if (size > 0) {
				rcImageData = LoadResource(hModule, hResInfo);
				if (rcImageData) {
					imagedata = static_cast<const char*>(LockResource(rcImageData));
					shaderstring = imagedata;
					UnlockResource(rcImageData);
					FreeResource(rcImageData);
					UnlockResource(rcImageData);
				}
				FreeResource(rcImageData);
			}
		}
	}
	return shaderstring;
}


// =====================================================================================
