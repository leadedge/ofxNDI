/*
	NDI sender

	using the NDI SDK to send the frames via network

	https://ndi.video

	Copyright (C) 2016-2024 Lynn Jarvis.

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

	08.07.18 - Uses ofxNDIsend class
	09.07.18 - Send ofFbo, ofTexture, ofImage, ofPixels, char
			   Shaders for fbo/texture colour format conversion
	19.07.18 - ofDisableAlphaBlending before RGBA to YUV conversion
	29.07.18 - Quit SendImage if fbo or texture is not RGBA
	06.08.18 - Add GetSenderName()
	11.08.18 - SendImage - add checks for allocation
			 - Release sender and resources in destructor
			 - Return false for CreateSender if zero width or height
	10.11.19 - Remove shaders and create an RGBA sender
			   to let the NDI system perform data conversion.
			   ofFbo, ofTexture, ofPixels or pixel data mst be RGBA
	04.12.19 - Revise for ARM port (https://github.com/IDArnhem/ofxNDI)
			   Cleanup
	13.12.19 - Temporary changes to disable pbo functions 
			   to enable compile for Raspberry PI
	27.02.20 - Restored PBO functions
			   TODO disable using #ifdef TARGET_RASPBERRY_PI ?
	08.12.20 - Corrected from ReadPixels(fbo.getTexture(), to ReadPixels(fbo) for SendImage fbo
	24.12.20 - Changed ReadPixels from unsigned char to ofPixels
	15.11.21 - Revise ReadTexturePixels
	18.11.21 - Revise ReadTexturePixels further due to missing glBufferStorage extension for MacOS
			   Change ReadPixels from void to bool
	29.12.21 - Add SetFormat
	31/12/21 - Add AllocatePixelBuffers
			   Clean up SetFrameRate
	01/01/22 - Width/Height check in UpdateSender
			   texture/buffer allocated check in SendImage ofTexture
			   RGBA/RGBA8 check in SendImage ofTexture
			   ofDisableDepthTest in SendImage ofTexture
			   Width/Height/UpdateSender check in SendImage ofTexture
			   texture/buffer allocated check in ReadPixels ofTexture
			   data/pbo/fbo allocated check in ReadTexturePixels
			   Add double GetFrameRate()
	03/01/22 - Add YUV shader conversion functions
	06.01.22 - Conditional formats for SetFormat
	12.01.22 - "doesFileExist" instead of "_access" in SetFormat (PR #31)
	27.04.22 - Allow for non-texture image in Send ofImage
			 - Correct use of ndiBuffer[m_idx] directly in ReadYUVPixels
			 - Correct missing return statement for non-texture image.
	28.04.22 - Add GetNDIname()
	10.05.22 - Allow for RGBX in check of format in Sendimage
	22.06.22 - rgbg2Yuv shaders located in a "bin\data\rgba2Yuv" folder
	           instead of "bin\data\shaders\rgba2Yuv" to avoid conflicts
			   with over-write by Project Generator
	10.12.22 - SetFormat - test existence of required rgba2yuv shader
			   in "data/rgba2yuv" or "data/shaders/rgba2yuv" for existing code
			   UpdateSender - test for sender creation.
	11.12.22 - ReadYUVpixels - corrected shader load for optional shaders subfolder.
	28.01.23 - Fix missing comment double quotes for gles version (PR #41)
	18.11.23 - Remove glReadBuffer from ReadTexturePixels
	05.12.23 - ReadYUVpixels use absolute shader path instead of relative
	07.12.23 - SendImage cast to int for dimensions to prevent C4267 warning
	15.12.23 - ReadYUVpixels use ofFilePath::getCurrentExeDir() instead of GetCurrentModule
	16.12.23 - Remove "shaders/rgba2yuv/" folder option
			 - Revise SetFormat to find the shader folder and test existence
	23.05.24 - SendImage ofTexture - RGBA only

*/
#include "ofxNDIsender.h"


ofxNDIsender::ofxNDIsender()
{
	// DEBUG - report ARM target
#ifdef TARGET_LINUX
	printf("TARGET_LINUX\n");
#endif

#ifdef TARGET_LINUX_ARM
	printf("TARGET_LINUX_ARM\n");
#endif

#ifdef TARGET_OPENGLES
	printf("TARGET_OPENGLES\n");
#endif
	
#ifdef TARGET_LINUX_ARM
	printf("TARGET_LINUX_ARM\n");
#endif

#ifdef TARGET_RASPBERRY_PI
	printf("TARGET_RASPBERRY_PI\n");
#endif

#ifdef GL_ES_VERSION_3_0
	printf("gles3 version\n");
#endif

#ifdef GL_ES_VERSION_2_0
	printf("gles2 version\n");
#endif

	m_SenderName = "";
	m_bReadback = false; // Asynchronous fbo pixel data readback option
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;

}

ofxNDIsender::~ofxNDIsender()
{
	// Release sender and resources
	ReleaseSender();
}

// Create an RGBA sender
bool ofxNDIsender::CreateSender(const char *sendername, unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0)
		return false;

	// Initialize pixel buffers for sending
	AllocatePixelBuffers(width, height);
	m_idx = 0;

	// Initialize OpenGL pbos for asynchronous readback of fbo data
	if(!m_pbo[0])
		glGenBuffers(3, m_pbo);

	PboIndex = NextPboIndex = 0; // index used for asynchronous fbo readback

	// Allocate utility fbo to the sender size
	ndiFbo.allocate(width, height, GL_RGBA);

	if (NDIsender.CreateSender(sendername, width, height)) {
		m_SenderName = sendername;
		return true;
	}
	else {
		m_SenderName = "";
		return false;
	}
}

// Update sender dimensions
bool ofxNDIsender::UpdateSender(unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0)
		return false;

	// Return if no sender created
	if (!NDIsender.SenderCreated())
		return false;

	// Re-allocate pixel buffers
	m_idx = 0;
	AllocatePixelBuffers(width, height);

	// Delete and re-initialize OpenGL pbos
	if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
	glGenBuffers(3, m_pbo);
	PboIndex = NextPboIndex = 0; // reset index used for asynchronous fbo readback

	// Re-allocate utility fbo
	ndiFbo.allocate(width, height, GL_RGBA);
	
	// Update NDI video frame
	return NDIsender.UpdateSender(width, height);
}

// Close sender and release resources
void ofxNDIsender::ReleaseSender()
{
	// Release sender first so no more frames are sent
	NDIsender.ReleaseSender();

	// Delete async sending buffers
	if (ndiBuffer[0].isAllocated())	ndiBuffer[0].clear();
	if (ndiBuffer[1].isAllocated())	ndiBuffer[1].clear();

	// Delete readback pbos
	if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;

	// Release utility fbo
	if (ndiFbo.isAllocated()) ndiFbo.clear();

	m_SenderName = "";
}

// Return whether the sender has been created
bool ofxNDIsender::SenderCreated()
{
	return NDIsender.SenderCreated();
}

// Return current sender width
unsigned int ofxNDIsender::GetWidth()
{
	return NDIsender.GetWidth();
}

// Return current sender height
unsigned int ofxNDIsender::GetHeight() 
{
	return NDIsender.GetHeight();
}

// Return the sender name
std::string ofxNDIsender::GetSenderName()
{
	return NDIsender.GetSenderName();
}

// Return the sender NDI name
std::string ofxNDIsender::GetNDIname()
{
	return NDIsender.GetNDIname();
}

// Send ofFbo
bool ofxNDIsender::SendImage(ofFbo fbo, bool bInvert)
{
	return SendImage(fbo.getTexture(), bInvert);
}

// Send ofTexture
bool ofxNDIsender::SendImage(ofTexture tex, bool bInvert)
{
	if (!tex.isAllocated())
		return false;

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated())
		return false;

	// Quit if the texture is not RGBA, RGBA8 or BGRA
	if (!(tex.getTextureData().glInternalFormat != GL_RGBA 
		|| tex.getTextureData().glInternalFormat != GL_RGBA8
		|| tex.getTextureData().glInternalFormat != GL_BGRA))
		return false;

	ofDisableDepthTest(); // In case this was enabled, or textures do not show

	unsigned int width  = (unsigned int)tex.getWidth();
	unsigned int height = (unsigned int)tex.getHeight();

	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);

	if (GetAsync())
		m_idx = (m_idx + 1) % 2;

	// Read texture pixels into a pixel buffer
	bool bResult = false;
	// NDIlib_FourCC_video_type_UYVY can only be enabled by SetFormat
	// Path to required shaders is tested
	if (NDIsender.GetFormat() == NDIlib_FourCC_video_type_UYVY) {
		// Convert to the YUV format at the same time.
		// YUV output width is half that of the RGBA input
		bResult = ReadYUVpixels(tex, width/2, height, ndiBuffer[m_idx]);
	}
	else {
		bResult = ReadPixels(tex, width, height, ndiBuffer[m_idx]);
	}

	// Send pixel data
	// NDI video frame line stride has been set to match the data format.
	// (see ofxNDIsend::SetVideoStride)
	if (bResult)
		return NDIsender.SendImage((const unsigned char *)ndiBuffer[m_idx].getData(), width, height, false, bInvert);


	return false;

}

// Send ofImage
bool ofxNDIsender::SendImage(ofImage img, bool bSwapRB, bool bInvert)
{
	if (!img.isAllocated())
		return false;
	
	// RGBA for images
	if (img.getImageType() != OF_IMAGE_COLOR_ALPHA)
		img.setImageType(OF_IMAGE_COLOR_ALPHA);
	
	if (img.isUsingTexture())
		return SendImage(img.getTexture(), bInvert);
	else
		return SendImage(img.getPixels(), bSwapRB, bInvert);


}

// Send ofPixels
bool ofxNDIsender::SendImage(ofPixels pix, bool bSwapRB, bool bInvert)
{
	if (!pix.isAllocated())
		return false;

	// RGBA for ofPixels
	if (pix.getImageType() != OF_IMAGE_COLOR_ALPHA)
		pix.setImageType(OF_IMAGE_COLOR_ALPHA);

	return SendImage((const unsigned char *)pix.getData(),
		(int)pix.getWidth(), (int)pix.getHeight(), bSwapRB, bInvert);

}

// Send RGBA image pixels
bool ofxNDIsender::SendImage(const unsigned char * pixels,
	unsigned int width, unsigned int height,
	bool bSwapRB, bool bInvert)
{
	if (!pixels)
		return false;

	// NDI format must be set to RGBA to match the pixel data
	if (!(GetFormat() == NDIlib_FourCC_video_type_RGBA || GetFormat() == NDIlib_FourCC_video_type_RGBX)) {
			SetFormat(NDIlib_FourCC_video_type_RGBA);
	}

	// Update sender to match dimensions
	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);
	
	return NDIsender.SendImage(pixels, width, height, bSwapRB, bInvert);

}

// Set output format
void ofxNDIsender::SetFormat(NDIlib_FourCC_video_type_e format)
{
	if (format == NDIlib_FourCC_video_type_UYVY) {
		// For YUV format, test existence of required rgba2yuv shader folder
		std::string shaderpath = ofFilePath::getCurrentExeDir();
		shaderpath += "data\\rgba2yuv\\";
		if (ofDirectory::doesDirectoryExist(shaderpath, false)) {
			NDIsender.SetFormat(format);
			// Buffer size will change between YUV and RGBA
			// Retain sender dimensions, but update the sender
			// to re-create pbos, buffers and NDI video frame
			// Update sender if already created (UpdateSender checks)
			UpdateSender(NDIsender.GetWidth(), NDIsender.GetHeight());
		}
		else {
			printf("rgba2yuv shader not found\n");
		}
	}
	else if (format == NDIlib_FourCC_video_type_BGRA
		  || format == NDIlib_FourCC_video_type_BGRX
		  || format == NDIlib_FourCC_video_type_RGBA
		  || format == NDIlib_FourCC_video_type_RGBX) {
			  // Supported formats
			  NDIsender.SetFormat(format);
			  UpdateSender(NDIsender.GetWidth(), NDIsender.GetHeight());
	}
	else {
		printf("Incompatible format\n");
	}

}

// Get output format
NDIlib_FourCC_video_type_e ofxNDIsender::GetFormat()
{
	return NDIsender.GetFormat();
}

// Set frame rate whole number
void ofxNDIsender::SetFrameRate(int framerate)
{
	NDIsender.SetFrameRate(framerate);
}

// Set frame rate decimal number
void ofxNDIsender::SetFrameRate(double framerate)
{
	NDIsender.SetFrameRate(framerate);
}

// Set frame rate numerator and denominator
void ofxNDIsender::SetFrameRate(int framerate_N, int framerate_D)
{
	NDIsender.SetFrameRate(framerate_N, framerate_D);
}

// Return current fps
double ofxNDIsender::GetFps()
{
	int num = 0;
	int den = 0;
	double fps = 0.0;
	NDIsender.GetFrameRate(num, den);
	if (den > 0)
		fps = (double)num / (double)den;
	else
		fps = 60.0; // default
	return fps;
}

// Get current frame rate numerator and denominator
void ofxNDIsender::GetFrameRate(int &framerate_N, int &framerate_D)
{
	NDIsender.GetFrameRate(framerate_N, framerate_D);
}

// Get current frame rate
double ofxNDIsender::GetFrameRate()
{
	int num = 0;
	int den = 0;
	NDIsender.GetFrameRate(num, den);
	if (den > 0)
		return (double)num / (double(den));
	else
		return 0.0;
}

// Set aspect ratio
void ofxNDIsender::SetAspectRatio(int horizontal, int vertical)
{
	NDIsender.SetAspectRatio(horizontal, vertical);
}

// Get current aspect ratio
void ofxNDIsender::GetAspectRatio(float &aspect)
{
	NDIsender.GetAspectRatio(aspect);
}

// Set progressive mode
void ofxNDIsender::SetProgressive(bool bProgressive)
{
	NDIsender.SetProgressive(bProgressive);
}

// Get whether progressive
bool ofxNDIsender::GetProgressive()
{
	return NDIsender.GetProgressive();
}

// Set clocked 
void ofxNDIsender::SetClockVideo(bool bClocked)
{
	NDIsender.SetClockVideo(bClocked);
}

// Get whether clocked
bool ofxNDIsender::GetClockVideo()
{
	return NDIsender.GetClockVideo();
}

// Set asynchronous sending mode
void ofxNDIsender::SetAsync(bool bActive)
{
	NDIsender.SetAsync(bActive);
}

// Get whether async sending mode
bool ofxNDIsender::GetAsync()
{
	return NDIsender.GetAsync();
}

// Set asynchronous readback of pixels from FBO or texture
void ofxNDIsender::SetReadback(bool bReadback)
{
	m_bReadback = bReadback;
}

// Get current readback mode
bool ofxNDIsender::GetReadback()
{
	return m_bReadback;
}

// Set to send Audio
void ofxNDIsender::SetAudio(bool bAudio)
{
	NDIsender.SetAudio(bAudio);
}

// Set audio sample rate
void ofxNDIsender::SetAudioSampleRate(int sampleRate)
{
	NDIsender.SetAudioSampleRate(sampleRate);
}

// Set number of audio channels
void ofxNDIsender::SetAudioChannels(int nChannels)
{
	NDIsender.SetAudioChannels(nChannels);
}

// Set number of audio samples
void ofxNDIsender::SetAudioSamples(int nSamples)
{
	NDIsender.SetAudioSamples(nSamples);
}

// Set audio timecode
void ofxNDIsender::SetAudioTimecode(int64_t timecode)
{
	NDIsender.SetAudioTimecode(timecode);
}

// Set audio data
void ofxNDIsender::SetAudioData(float *data)
{
	NDIsender.SetAudioData(data);
}

// Set to send metadata
void ofxNDIsender::SetMetadata(bool bMetadata)
{
	NDIsender.SetMetadata(bMetadata);
}

// Set metadata
void ofxNDIsender::SetMetadataString(std::string datastring)
{
	NDIsender.SetMetadataString(datastring);
}

// Get NDI dll version number
std::string ofxNDIsender::GetNDIversion()
{
	return NDIsender.GetNDIversion();
}

//
// =========== Private functions ===========
//

// Read pixels from fbo to buffer
bool ofxNDIsender::ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (m_bReadback) {
		// Asynchronous readback using pbos
		return ReadTexturePixels(fbo.getTexture(), width, height, buffer.getData());
	}
	else {
		// Read fbo directly
		// Specify width and height
		fbo.bind();
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void *)buffer.getData());
		fbo.unbind();

	}
	return true;
}

// Read pixels from texture to buffer
bool ofxNDIsender::ReadPixels(ofTexture tex, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (!tex.isAllocated() || !buffer.isAllocated())
		return false;

	bool bRet = true;
	if (m_bReadback) {
		bRet = ReadTexturePixels(tex, width, height, buffer.getData());
	}
	else {
		tex.readToPixels(buffer); // Uses glGetTexImage
	}

	return bRet;

}

// Asynchronous texture pixel Read-back via fbo
bool ofxNDIsender::ReadTexturePixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data)
{
	if (!data || m_pbo[0] == 0 || !ndiFbo.isAllocated())
		return false;

	void *pboMemory = nullptr;

	PboIndex = (PboIndex + 1) % 3;
	NextPboIndex = (PboIndex + 1) % 3;

	// The local fbo will be the same size as the sender texture
	ndiFbo.bind();

	// Attach the texture passed in
	ndiFbo.attachTexture(tex, GL_RGBA, 0);

	// Bind the current PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

	// Null existing PBO data to avoid a stall
	// This allocates memory for the PBO
	glBufferDataARB(GL_PIXEL_PACK_BUFFER, (size_t)width * (size_t)height * 4, 0, GL_STREAM_READ);

	// Read pixels from framebuffer to the current PBO
	// After a buffer is bound, glReadPixels() will pack(write) data into the Pixel Buffer Object.
	// glReadPixels() should return immediately.
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)0);

	// Map the previous PBO to process its data by CPU
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[NextPboIndex]);
	pboMemory = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (pboMemory) {
		// Use SSE2 mempcy
		ofxNDIutils::CopyImage((unsigned char *)pboMemory, data, width, height, width * 4);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	else {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		ndiFbo.unbind();
		return false;
	}

	// Back to conventional pixel operation
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	ndiFbo.unbind();

	return true;

}

// Initialize pixel buffers for sending
void ofxNDIsender::AllocatePixelBuffers(unsigned int width, unsigned int height)
{
	if (width > 0 && height > 0) {
		ndiBuffer[0].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
		ndiBuffer[1].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
	}
}


//
// YUV shader functions
//

//
// Read yuv pixels from rgba fbo to buffer
//
// Shaders should be in a "bin\data\rgbg2Yuv" folder
//   bin
//     data
//        rgba2yuv
//
bool ofxNDIsender::ReadYUVpixels(ofFbo &fbo, unsigned int halfwidth, unsigned int height, ofPixels &buffer)
{
	return ReadYUVpixels(fbo.getTexture(), halfwidth, height, buffer);
}

// Read yuv pixels from an rgba texture to buffer
// The halfwidth argument is half the sender width
bool ofxNDIsender::ReadYUVpixels(ofTexture &tex, unsigned int halfwidth, unsigned int height, ofPixels &buffer)
{
	if (halfwidth == 0 || height == 0)
		return false;

	// Load the shader
	if (!rgba2yuv.isLoaded()) {
		// Get the rgba2yuv shader folder full path
		std::string shaderpath = ofFilePath::getCurrentExeDir();
#ifdef TARGET_OPENGLES
		shaderpath += "data\\rgba2yuv\\ES2\\rgba2yuv";
#else
		if (ofIsGLProgrammableRenderer())
			shaderpath += "data\\rgba2yuv\\GL3\\rgba2yuv";
		else
			shaderpath += "data\\rgba2yuv\\GL2\\rgba2yuv";
#endif
		if (!rgba2yuv.load(shaderpath))
			return false;
	}

	// Convert the rgba texture to YUV via fbo
	ndiFbo.begin();
	ofDisableAlphaBlending();
	ofDisableDepthTest();
	rgba2yuv.begin();
	rgba2yuv.setUniformTexture("rgbatex", tex, 1);
	tex.draw(0, 0);
	rgba2yuv.end();
	ndiFbo.end();

	// The YUV result is in the ndiFbo texture
	// The data is half the width of the rgba texture.
	return ReadPixels(ndiFbo, halfwidth, height, buffer);

}

