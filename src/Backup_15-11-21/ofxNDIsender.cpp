/*
	NDI sender

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2021 Lynn Jarvis.

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
	printf("gles3 version\n);
#endif

#ifdef GL_ES_VERSION_2_0
	printf("gles2 version\n);
#endif


	m_SenderName = "";
	m_bReadback = false; // Asynchronous fbo pixel data readback option
	ndiPbo[0] = 0;
	ndiPbo[1] = 0;

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
	ndiBuffer[0].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
	ndiBuffer[1].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
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
	// ===============================================================

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
	
	ReadPixels(fbo, width, height, ndiBuffer[m_idx]);

	return NDIsender.SendImage((const unsigned char *)ndiBuffer[m_idx].getData(), width, height, false, bInvert);

}

// Send ofTexture
bool ofxNDIsender::SendImage(ofTexture tex, bool bInvert)
{
	if (!tex.isAllocated()) {
		return false;
	}

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated()) {
		return false;
	}

	// Quit if the texture is not RGBA
	if (tex.getTextureData().glInternalFormat != GL_RGBA) {
		return false;
	}

	unsigned int width = (unsigned int)tex.getWidth();
	unsigned int height = (unsigned int)tex.getHeight();

	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);

	if (GetAsync())
		m_idx = (m_idx + 1) % 2;
	ReadPixels(tex, width, height, ndiBuffer[m_idx]);

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

	unsigned int width = (unsigned int)img.getWidth();
	unsigned int height = (unsigned int)img.getHeight();

	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);

	return SendImage(img.getPixels().getData(),	width, height, false, bInvert);

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

	unsigned int width = (unsigned int)pix.getWidth();
	unsigned int height = (unsigned int)pix.getHeight();

	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);

	return NDIsender.SendImage(pix.getData(), width, height, false, bInvert);

}

// Send RGBA image pixels
bool ofxNDIsender::SendImage(const unsigned char * pixels,
	unsigned int width, unsigned int height,
	bool bSwapRB, bool bInvert)
{
	if (!pixels)
		return false;

	// Update sender to match dimensions
	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight())
		NDIsender.UpdateSender(width, height);
	
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
	NDIsender.SetAsync(false);
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


// Read pixels from texture to buffer
void ofxNDIsender::ReadPixels(ofTexture tex, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (m_bReadback)
		ReadTexturePixels(tex, width, height, buffer.getData());
	else
		tex.readToPixels(buffer);
}

// Read pixels from fbo to buffer
void ofxNDIsender::ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (m_bReadback) // Asynchronous readback using two pbos
		ReadFboPixels(fbo, width, height, buffer.getData());
	else // Read fbo directly
		fbo.readToPixels(buffer);
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
