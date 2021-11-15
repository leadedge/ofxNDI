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
	15.11.21 - Revise ReadTexturePixels
	
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
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
	PboIndex = NextPboIndex = 0;
	m_fbo = 0;

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
	// pbos for asynchronous readback
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
	PboIndex = NextPboIndex = 0;
	// Allocate utility fbo
	if(m_fbo == 0)
		glGenFramebuffers(1, &m_fbo);

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

	// Delete pbos for re-allocation
	if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
	PboIndex = NextPboIndex = 0; // reset index used for asynchronous fbo readback

	return NDIsender.UpdateSender(width, height);
}

// Close sender and release resources
void ofxNDIsender::ReleaseSender()
{
	// Delete async sending buffers
	if (ndiBuffer[0].isAllocated())	ndiBuffer[0].clear();
	if (ndiBuffer[1].isAllocated())	ndiBuffer[1].clear();

	// Delete readback pbos
	if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
	PboIndex = NextPboIndex = 0;

	// Release utility fbo
	if (m_fbo > 0) glDeleteFramebuffers(1, &m_fbo);
	m_fbo = 0;

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
	// ofxNDIutils::StartTiming();

	if (m_bReadback)
		ReadTexturePixels(tex, width, height, buffer.getData());
	else
		tex.readToPixels(buffer);

}

// Read pixels from fbo to buffer
void ofxNDIsender::ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (m_bReadback) // Asynchronous readback using two pbos TODO - testing
		ReadTexturePixels(fbo.getTexture(), width, height, buffer.getData(), fbo.getId());
	else // Read fbo directly
		fbo.readToPixels(buffer);

}

//
// Asynchronous texture pixel Read-back via pbo
//
// adapted from : http://www.songho.ca/opengl/gl_pbo.html
//
bool ofxNDIsender::ReadTexturePixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data, GLuint HostFBO)
{
	if (!data) {
		return false;
	}

	void *pboMemory = nullptr;
	unsigned int pitch = width * 4; // RGBA pitch

	if (m_fbo == 0) {
		glGenFramebuffers(1, &m_fbo);
	}

	// Create pbos if not already
	if (m_pbo[0] == 0) {
		glGenBuffers(3, m_pbo);
		PboIndex = 0;
		NextPboIndex = 0;
	}

	PboIndex = (PboIndex + 1) % 3;
	NextPboIndex = (PboIndex + 1) % 3;

	// If Texture ID is zero, the texture is already attached to the Host Fbo
	// and we do nothing. If not we need to create an fbo and attach the user texture.
	// If Texture ID is zero, the texture is already attached to the Host Fbo
	// and we do nothing. If not we need to create an fbo and attach the user texture
	if (tex.texData.textureID > 0) {
		// Attach the texture to point 0
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			tex.texData.textureTarget, tex.texData.textureID, 0);
		// Set the target framebuffer to read
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	}
	else if (HostFBO == 0) {
		// If no texture ID, a Host FBO must be provided
		return false;
	}

	// Bind the PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

	// Check it's size
	GLint buffersize = 0;
	glGetBufferParameteriv(GL_PIXEL_PACK_BUFFER, GL_BUFFER_SIZE, &buffersize);
	if (buffersize > 0 && buffersize != (int)(pitch * height)) {
		// For a sender size change, all PBOs must be re-created.
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER, HostFBO);
		glDeleteBuffers(3, m_pbo);
		m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;
		return false;
	}

	// Allocate pbo data buffer with glBufferStorage.
	// The buffer is immutable and size is set for the lifetime of the object.
	if (buffersize == 0) {
		glBufferStorage(GL_PIXEL_PACK_BUFFER, pitch*height, 0, GL_MAP_READ_BIT);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, HostFBO);
		return false; // No more for this round
	}

	// Read pixels from framebuffer to PBO - glReadPixels() should return immediately.
	glPixelStorei(GL_PACK_ROW_LENGTH, pitch / 4); // row length in pixels
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	// If there is data in the next pbo from the previous call, read it back.
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[NextPboIndex]);

	// Map the PBO to process its data by CPU.
	// Map the entire data store into the client's address space.
	// glMapBufferRange may give improved performance over glMapBuffer.
	// GL_MAP_READ_BIT indicates that the returned pointer may be used to read buffer object data.
	pboMemory = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, buffersize, GL_MAP_READ_BIT);

	// glMapBuffer can return NULL when called the first time
	// when the next pbo has not been filled with data yet.
	// Remove the last error
	glGetError();

	// Update data directly from the mapped buffer.
	// If no pbo data, skip the copy rather than return false.
	if (pboMemory) {
		ofxNDIutils::CopyImage((unsigned char *)pboMemory, data, width, height, width * 4);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	// Back to conventional pixel operation
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Restore the previous fbo binding
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, HostFBO);

	return true;

}
