/*
	NDI sender

	using the NDI SDK to send the frames via network

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

	08.07.18 - Uses ofxNDIsend class
	09.07.18 - Send ofFbo, ofTexture, ofImage, ofPixels, char
			   Shaders for fbo/texture colour format conversion
	19.07.18 - ofDisableAlphaBlending before RGBA to YUV conversion
	29.07.18 - Quit SendImage if fbo or texture is not RGBA
	06.08.18 - Add GetSenderName()
	11.08.18 - SendImage - add checks for allocation
			 - Release sender and resources in destructor
			 - Return false for CreateSender if zero width or height

*/
#include "ofxNDIsender.h"


ofxNDIsender::ofxNDIsender()
{
	m_ColorFormat = NDIlib_FourCC_type_RGBA; // default rgba output format
	m_bReadback = false; // Asynchronous fbo pixel data readback option
	ndiPbo[0] = 0;
	ndiPbo[1] = 0;
	m_SenderName = "";

}


ofxNDIsender::~ofxNDIsender()
{
	// Release sender and resources
	ReleaseSender();
}

// Create an RGBA sender
bool ofxNDIsender::CreateSender(const char *sendername, unsigned int width, unsigned int height)
{
	return CreateSender(sendername, width, height, NDIlib_FourCC_type_RGBA);
}

// Create a sender with of specified colour format
bool ofxNDIsender::CreateSender(const char *sendername, unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat)
{
	// printf("ofxNDIsender::CreateSender %s, %dx%d\n", sendername, width, height);

	if (width == 0 || height == 0)
		return false;

	// Initialize pixel buffers for sending
	ndiBuffer[0].allocate(width, height, 4);
	ndiBuffer[1].allocate(width, height, 4);
	m_idx = 0;

	// Initialize OpenGL pbos for asynchronous readback of fbo data
	glGenBuffers(2, ndiPbo);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, ndiPbo[0]);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height * 4, 0, GL_STREAM_READ);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, ndiPbo[1]);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height * 4, 0, GL_STREAM_READ);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	PboIndex = NextPboIndex = 0; // index used for asynchronous fbo readback

	// Allocate utility fbo
	ndiFbo.allocate(width, height, GL_RGBA);

	// Set user specified colour format
	m_ColorFormat = colorFormat;

	if (NDIsender.CreateSender(sendername, width, height, colorFormat)) {
		m_SenderName = sendername;
		return true;
	}
	else {
		m_SenderName = "";
		return false;
	}
}

// Update sender dimensions with existing colour format
bool ofxNDIsender::UpdateSender(unsigned int width, unsigned int height)
{
	return UpdateSender(width, height, m_ColorFormat);
}

// Update sender dimensions and colour format
bool ofxNDIsender::UpdateSender(unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat)
{
	// Re-initialize pixel buffers
	ndiBuffer[0].allocate(width, height, 4);
	ndiBuffer[1].allocate(width, height, 4);
	m_idx = 0;

	// Delete and re-initialize OpenGL pbos
	if (ndiPbo[0])	glDeleteBuffers(2, ndiPbo);
	glGenBuffers(2, ndiPbo);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, ndiPbo[0]);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height * 4, 0, GL_STREAM_READ);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, ndiPbo[1]);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width*height * 4, 0, GL_STREAM_READ);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	PboIndex = NextPboIndex = 0; // reset index used for asynchronous fbo readback

	// Re-initialize utility fbo
	ndiFbo.allocate(width, height, GL_RGBA);

	return NDIsender.UpdateSender(width, height);
}

// Close sender and release resources
void ofxNDIsender::ReleaseSender()
{
	// Delete async sending buffers
	if (ndiBuffer[0].isAllocated())	ndiBuffer[0].clear();
	if (ndiBuffer[1].isAllocated())	ndiBuffer[1].clear();

	// Delete fbo readback pbos
	if (ndiPbo[0]) glDeleteBuffers(2, ndiPbo);

	// Release utility fbo
	if (ndiFbo.isAllocated()) ndiFbo.clear();

	// Release sender
	NDIsender.ReleaseSender();

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

// Send ofFbo
bool ofxNDIsender::SendImage(ofFbo fbo, bool bInvert)
{
	if (!fbo.isAllocated())
		return false;

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated())
		return false;

	// Quit if the fbo is not RGBA
	if (fbo.getTexture().getTextureData().glInternalFormat != GL_RGBA)
		return false;

	unsigned int width = (unsigned int)fbo.getWidth();
	unsigned int height = (unsigned int)fbo.getHeight();


	// Update the sender if the dimensions are changed
	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);

	// For asynchronous NDI sending, alternate between buffers because
	// one buffer is being filled in while the second is "in flight"
	// and being processed by the API.
	if (GetAsync())
		m_idx = (m_idx + 1) % 2;

	switch (m_ColorFormat) {
	case NDIlib_FourCC_type_UYVY:
		// case NDIlib_FourCC_type_UYVA: // Alpha out not supported yet
		ofDisableAlphaBlending();
		ColorConvert(fbo); // RGBA to YUV422
		ReadPixels(ndiFbo, width, height, ndiBuffer[m_idx].getData());
		break;
	case NDIlib_FourCC_type_BGRA:
	case NDIlib_FourCC_type_BGRX:
		// RGBA to BGRA into the utilty fbo
		ColorSwap(fbo);
		// Get pixel data from the fbo
		ReadPixels(ndiFbo, width, height, ndiBuffer[m_idx].getData());
		break;
	default:
		// Default RGBA output
		ReadPixels(fbo, width, height, ndiBuffer[m_idx].getData());
		break;
	}

	// return NDIsender.SendImage((const unsigned char *)ndiBuffer[m_idx].getPixels(), width, height, false, bInvert); // for < OF10
	return NDIsender.SendImage((const unsigned char *)ndiBuffer[m_idx].getData(), width, height, false, bInvert);

}

// Send ofTexture
bool ofxNDIsender::SendImage(ofTexture tex, bool bInvert)
{
	if (!tex.isAllocated())
		return false;

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated())
		return false;

	// Quit if the texture is not RGBA
	if (tex.getTextureData().glInternalFormat != GL_RGBA) {
		printf("Texture is not RGBA (%x)\n", tex.getTextureData().glInternalFormat);
		return false;
	}

	unsigned int width = (unsigned int)tex.getWidth();
	unsigned int height = (unsigned int)tex.getHeight();

	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);

	if (GetAsync())
		m_idx = (m_idx + 1) % 2;

	switch (m_ColorFormat) {
	case NDIlib_FourCC_type_UYVY:
		ofDisableAlphaBlending(); // Avoid alpha trails
		ColorConvert(tex);
		ReadPixels(ndiFbo, width, height, ndiBuffer[m_idx].getData());
		break;
	case NDIlib_FourCC_type_BGRA:
	case NDIlib_FourCC_type_BGRX:
		ColorSwap(tex);
		ReadPixels(ndiFbo, width, height, ndiBuffer[m_idx].getData());
		break;
	default:
		ReadPixels(tex, width, height, ndiBuffer[m_idx].getData());
		break;
	}

	return NDIsender.SendImage((const unsigned char *)ndiBuffer[m_idx].getData(), width, height, false, bInvert);

}

// Send ofImage
bool ofxNDIsender::SendImage(ofImage img, bool bInvert)
{
	if (!img.isAllocated())
		return false;

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated())
		return false;

	// RGBA only for images and pixels
	if (img.getImageType() != OF_IMAGE_COLOR_ALPHA)
		img.setImageType(OF_IMAGE_COLOR_ALPHA);

	return SendImage(img.getPixels().getData(),
		(unsigned int)img.getWidth(), (unsigned int)img.getHeight(), false, bInvert);

}

// Send ofPixels
bool ofxNDIsender::SendImage(ofPixels pix, bool bInvert)
{
	if (!pix.isAllocated())
		return false;

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated())
		return false;

	if (pix.getImageType() != OF_IMAGE_COLOR_ALPHA)
		pix.setImageType(OF_IMAGE_COLOR_ALPHA);

	return NDIsender.SendImage(pix.getData(),
		(unsigned int)pix.getWidth(), (unsigned int)pix.getHeight(), false, bInvert);

}

// Send RGBA image pixels
bool ofxNDIsender::SendImage(const unsigned char * pixels,
	unsigned int width, unsigned int height,
	bool bSwapRB, bool bInvert)
{
	if (!pixels)
		return false;

	// Update sender to match dimensions
	// Data must be RGBA and the sender colour format has to match
	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight() || m_ColorFormat != NDIlib_FourCC_type_RGBA) {
		m_ColorFormat = NDIlib_FourCC_type_RGBA;
		NDIsender.UpdateSender(width, height, NDIlib_FourCC_type_RGBA);
	}
	return NDIsender.SendImage(pixels, width, height, bSwapRB, bInvert);
}

// Set frame rate whole number
void ofxNDIsender::SetFrameRate(int framerate)
{
	NDIsender.SetFrameRate(framerate*1000, 1000);
}

// Set frame rate decimal number
void ofxNDIsender::SetFrameRate(double framerate)
{
	NDIsender.SetClockVideo();
	NDIsender.SetFrameRate((int)(framerate*1000.0), 1000);
}

// Set frame rate numerator and denominator
void ofxNDIsender::SetFrameRate(int framerate_N, int framerate_D)
{
	NDIsender.SetClockVideo();
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

// Get current sender name
std::string ofxNDIsender::GetSenderName()
{
	return m_SenderName;
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

// Convert fbo texture from RGBA to UVYV
void ofxNDIsender::ColorConvert(ofFbo fbo) {

	ndiFbo.begin();
	yuvshaders.rgba2yuvShader.begin();
	fbo.getTexture().bind(1); // Source of RGBA pixels
	yuvshaders.rgba2yuvShader.setUniformTexture("rgbatex", fbo.getTexture(), 1);
	ndiFbo.draw(0, 0);
	yuvshaders.rgba2yuvShader.end();
	ndiFbo.end(); // result is in the utility fbo

}

// Convert texture from RGBA to UVYV
void ofxNDIsender::ColorConvert(ofTexture texture) {

	ndiFbo.begin();
	yuvshaders.rgba2yuvShader.begin();
	texture.bind(1);
	yuvshaders.rgba2yuvShader.setUniformTexture("rgbatex", texture, 1);
	ndiFbo.draw(0, 0);
	yuvshaders.rgba2yuvShader.end();
	ndiFbo.end();
}

// Convert fbo texture RGBA <> BGRA
void ofxNDIsender::ColorSwap(ofFbo fbo) {

	ndiFbo.begin();
	fbo.getTexture().bind(0);
	yuvshaders.rgba2bgra.begin();
	yuvshaders.rgba2bgra.setUniformTexture("texInput", fbo.getTexture(), 0);
	fbo.draw(0, 0); // Result goes to the ndiFbo texture
	yuvshaders.rgba2bgra.end();
	fbo.getTexture().unbind();
	ndiFbo.end();

}

// Convert texture RGBA <> BGRA
void ofxNDIsender::ColorSwap(ofTexture texture) {

	ndiFbo.begin();
	texture.bind(0);
	yuvshaders.rgba2bgra.begin();
	yuvshaders.rgba2bgra.setUniformTexture("texInput", texture, 0);
	texture.draw(0, 0); // Result goes to the ndiFbo texture
	yuvshaders.rgba2bgra.end();
	texture.unbind();
	ndiFbo.end();

}

// Read pixels from fbo to buffer
void ofxNDIsender::ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, unsigned char *data)
{
	if (m_bReadback) // Asynchronous readback using two pbos
		ReadFboPixels(fbo, width, height, ndiBuffer[m_idx].getData());
	else // Read fbo directly
		fbo.readToPixels(ndiBuffer[m_idx]);
}

// Read pixels from texture to buffer
void ofxNDIsender::ReadPixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data)
{
	if (m_bReadback)
		ReadTexturePixels(tex, width, height, ndiBuffer[m_idx].getData());
	else
		tex.readToPixels(ndiBuffer[m_idx]);
}

//
// Asynchronous fbo pixel Read-back
//
// adapted from : http://www.songho.ca/opengl/gl_pbo.html
//
bool ofxNDIsender::ReadFboPixels(ofFbo fbo, unsigned int width, unsigned int height, unsigned char *data)
{
	void *pboMemory;

	if (ndiPbo[0] == 0 || ndiPbo[1] == 0)
		return false;

	PboIndex = (PboIndex + 1) % 2;
	NextPboIndex = (PboIndex + 1) % 2;

	// Bind the fbo passed in
	fbo.bind();

	// Set the target framebuffer to read
	glReadBuffer(GL_FRONT);

	// Bind the current PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, ndiPbo[PboIndex]);

	// Read pixels from framebuffer to the current PBO
	// After a buffer is bound, glReadPixels() will pack(write) data into the Pixel Buffer Object.
	// glReadPixels() should return immediately.
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)0);

	// Map the previous PBO to process its data by CPU
	glBindBuffer(GL_PIXEL_PACK_BUFFER, ndiPbo[NextPboIndex]);
	pboMemory = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (pboMemory) {
		// Use SSE2 mempcy
		ofxNDIutils::CopyImage((unsigned char *)pboMemory, data, width, height, width * 4);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	else {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		fbo.unbind();
		return false;
	}

	// Back to conventional pixel operation
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	fbo.unbind();

	return true;

}

// Asynchronous texture pixel Read-back via fbo
bool ofxNDIsender::ReadTexturePixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data)
{
	void *pboMemory;

	if (ndiPbo[0] == 0 || ndiPbo[1] == 0)
		return false;

	PboIndex = (PboIndex + 1) % 2;
	NextPboIndex = (PboIndex + 1) % 2;

	// The local fbo will be the same size as the sender texture
	ndiFbo.bind();

	// Attach the texture passed in
	ndiFbo.attachTexture(tex, GL_RGBA, 0);

	// Set the target framebuffer to read
	glReadBuffer(GL_FRONT);

	// Bind the current PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, ndiPbo[PboIndex]);

	// Read pixels from framebuffer to the current PBO
	// After a buffer is bound, glReadPixels() will pack(write) data into the Pixel Buffer Object.
	// glReadPixels() should return immediately.
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)0);

	// Map the previous PBO to process its data by CPU
	glBindBuffer(GL_PIXEL_PACK_BUFFER, ndiPbo[NextPboIndex]);
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

	ndiFbo.unbind();

	// Back to conventional pixel operation
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	return true;

}
