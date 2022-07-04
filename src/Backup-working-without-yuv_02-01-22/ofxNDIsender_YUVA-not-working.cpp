/*
	NDI sender

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2022 Lynn Jarvis.

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
	// ndiBuffer[0].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
	// ndiBuffer[1].allocate(width, height, OF_IMAGE_COLOR_ALPHA);

	m_idx = 0;

	// Initialize OpenGL pbos for asynchronous readback of fbo data
	glGenBuffers(3, m_pbo);
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
	AllocatePixelBuffers(width, height);
	// LJ DEBUG
	// ndiBuffer[0].allocate(width, height, 4);
	// ndiBuffer[1].allocate(width, height, 4);
	m_idx = 0;

	// Delete and re-initialize OpenGL pbos
	if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
	glGenBuffers(3, m_pbo);
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

	// Delete readback pbos
	if (m_pbo[0]) glDeleteBuffers(3, m_pbo);
	m_pbo[0] = m_pbo[1] = m_pbo[2] = 0;

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
	return SendImage(fbo.getTexture(), bInvert);
}

// Send ofTexture
bool ofxNDIsender::SendImage(ofTexture tex, bool bInvert)
{
	if (!tex.isAllocated())
		return false;

	if (!ndiBuffer[0].isAllocated() || !ndiBuffer[1].isAllocated())
		return false;

	// Quit if the texture is not RGBA
	if (tex.getTextureData().glInternalFormat != GL_RGBA)
		return false;

	unsigned int width = (unsigned int)tex.getWidth();
	unsigned int height = (unsigned int)tex.getHeight();

	if (width != NDIsender.GetWidth() || height != NDIsender.GetHeight()) {
		NDIsender.UpdateSender(width, height);
	}

	if (GetAsync())
		m_idx = (m_idx + 1) % 2;

	// If output format is set to YUV, convert RGBA to YUV422
	// LJ DEBUG
	bool bResult = false;
	if (NDIsender.GetFormat() == NDIlib_FourCC_video_type_UYVY)
		bResult = ReadYUVpixels(tex, width, height, ndiBuffer[m_idx]);
	else if(NDIsender.GetFormat() == NDIlib_FourCC_type_UYVA)
		bResult = ReadYUVApixels(tex, width, height, ndiBuffer[m_idx]);
	else
		bResult = ReadPixels(tex, width, height, ndiBuffer[m_idx]);

	// Send pixel data
	if (bResult)
		return NDIsender.SendImage((const unsigned char *)ndiBuffer[m_idx].getData(), width, height, false, bInvert);
	else
		return false;

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

// Set output format
void ofxNDIsender::SetFormat(NDIlib_FourCC_video_type_e format)
{
	NDIsender.SetFormat(format);
}

// Get output format
NDIlib_FourCC_video_type_e ofxNDIsender::GetFormat()
{
	return NDIsender.GetFormat();
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

// Read pixels from fbo to buffer
bool ofxNDIsender::ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (m_bReadback) {
		// Asynchronous readback using pbos
		return ReadTexturePixels(fbo.getTexture(), width, height, buffer.getData());
	}
	else {
		// Read fbo directly
		fbo.readToPixels(buffer);
	}
	return true;
}

// Read pixels from texture to buffer
bool ofxNDIsender::ReadPixels(ofTexture tex, unsigned int width, unsigned int height, ofPixels &buffer)
{
	if (m_bReadback)
		return ReadTexturePixels(tex, width, height, buffer.getData());
	else
		tex.readToPixels(buffer); // Uses glGetTexImage

	return true;
}

// Read yuv pixels from rgba fbo to buffer
// Shaders must be copied to the "data/bin" folder
//  data
//    bin
//     shaders
//       rgba2yuv
//       rgba2yuva
//
bool ofxNDIsender::ReadYUVpixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer)
{
	return ReadYUVpixels(fbo.getTexture(), width, height, buffer);
}

// Read yuv pixels from an rgba texture to buffer
bool ofxNDIsender::ReadYUVpixels(ofTexture tex, unsigned int width, unsigned int height, ofPixels &buffer)
{
	// Load the shader for rgba - yuv conversion
	if (!rgba2yuv.isLoaded()) {
		bool bResult = false;
#ifdef TARGET_OPENGLES
		bResult = rgba2yuv.load("/shaders/rgba2yuv/ES2/rgba2yuv");
#else
		if (ofIsGLProgrammableRenderer()) {
			bResult = rgba2yuv.load("/shaders/rgba2yuv/GL3/rgba2yuv");
		}
		else {
			bResult = rgba2yuv.load("/shaders/rgba2yuv/GL2/rgba2yuv");
		}
#endif
		if (!bResult) {
			printf("rgba2yuv shader not loaded\n");
			return false;
		}
	}

	// Allocate utility fbo
	if (!ndiFbo.isAllocated()
		|| width / 2 != (unsigned int)ndiFbo.getWidth()
		|| height    != (unsigned int)ndiFbo.getHeight()) {
		ndiFbo.allocate(width/2, height, GL_RGBA);
	}

	// Convert the rgba texture to YUV via fbo
	ndiFbo.begin();
	ofDisableAlphaBlending();
	rgba2yuv.begin();
	rgba2yuv.setUniformTexture("rgbatex", tex, 1);
	tex.draw(0, 0);
	rgba2yuv.end();
	ndiFbo.end();

	// The YUV result is in the ndiFbo texture
	// YUV output is half the width of RGBA input
	return ReadPixels(ndiFbo.getTexture(), width/2, height, ndiBuffer[m_idx]);

}

bool ofxNDIsender::ReadYUVApixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer)
{
	return ReadYUVApixels(fbo.getTexture(), width, height, buffer);
}

// Read yuv pixels from an rgba texture to buffer
bool ofxNDIsender::ReadYUVApixels(ofTexture tex, unsigned int width, unsigned int height, ofPixels &buffer)
{
	printf("ReadYUVApixels\n");

	// Load the shader for rgba - yuv conversion
	if (!rgba2yuva.isLoaded()) {
		bool bResult = false;
#ifdef TARGET_OPENGLES
		bResult = rgba2yuva.load("/shaders/rgba2yuva/ES2/rgba2yuva");
#else
		if (ofIsGLProgrammableRenderer()) {
			bResult = rgba2yuva.load("/shaders/rgba2yuva/GL3/rgba2yuva");
		}
		else {
			bResult = rgba2yuva.load("/shaders/rgba2yuva/GL2/rgba2yuva");
		}
#endif
		if (!bResult) {
			printf("rgba2yuva shader not loaded\n");
			return false;
		}
	}

	// Allocate utility fbo
	if (!ndiFbo.isAllocated()
		|| width / 2 != (unsigned int)ndiFbo.getWidth()
		|| height  != (unsigned int)ndiFbo.getHeight()) {
		ndiFbo.allocate(width/2, height, GL_RGBA);
	}

	// Convert the rgba texture to YUV via fbo
	ndiFbo.begin();
	ofDisableAlphaBlending();
	rgba2yuva.begin();
	rgba2yuva.setUniformTexture("rgbatex", tex, 1);

	tex.draw(0, 0);
	rgba2yuva.end();
	ndiFbo.end();

	// The YUV result is in the ndiFbo texture
	// YUV output is half the width of RGBA input
	return ReadPixels(ndiFbo.getTexture(), width / 2, height, ndiBuffer[m_idx]);

	// Too slow
	// if (ReadPixels(ndiFbo.getTexture(), width/2, height, ndiBuffer[m_idx])) {
		
		// LJ DEBUG
		// Append alpha data at the end of the pbo
		// Create an rgba texture for extracting alpha
		// ofPixels buffer;
		// buffer.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
		// ReadPixels(tex, width, height, buffer);
		

		/*
		// copy the alpha bytes
		unsigned char * src = buffer.getData();
		unsigned char * dst = ndiBuffer[m_idx].getData();
		dst += width*4*height;
		for (int i = 0; i < (int)height; i++) {
			for (int j = 0; j < (int)width; j++) {
				src += 3; // alpha
				// *dst = *src;
			}
		}
		*/
		// buffer.clear();
		// return true;

	// }
	// return false;
}


// Asynchronous texture pixel Read-back via fbo
bool ofxNDIsender::ReadTexturePixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data)
{
	if (!data || m_pbo[0] == 0)
		return false;

	void *pboMemory = nullptr;

	PboIndex = (PboIndex + 1) % 3;
	NextPboIndex = (PboIndex + 1) % 3;

	// The local fbo will be the same size as the sender texture
	ndiFbo.bind();

	// Attach the texture passed in
	ndiFbo.attachTexture(tex, GL_RGBA, 0);

	// Set the target framebuffer to read
	glReadBuffer(GL_FRONT);

	// Bind the current PBO
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[PboIndex]);

	// Null existing PBO data to avoid a stall
	// This allocates memory for the PBO
	glBufferDataARB(GL_PIXEL_PACK_BUFFER, width*height*4, 0, GL_STREAM_READ);

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
	// LJ DEBUG
	if (NDIsender.GetFormat() == NDIlib_FourCC_type_UYVA) {
		// Add space for an alpha channel width x height bytes
		// That is allocate and alpha buffer - height + height*width/4
		ndiBuffer[0].allocate(width, height + height*width/4, OF_IMAGE_COLOR_ALPHA);
		ndiBuffer[1].allocate(width, height + height*width/4, OF_IMAGE_COLOR_ALPHA);
	}
	else {
		ndiBuffer[0].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
		ndiBuffer[1].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
	}
}


