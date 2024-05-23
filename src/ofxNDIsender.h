/*
	NDI sender

	using the NDI SDK to send the frames via network

	https://ndi.video/

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

	08.07.18 - Use ofxNDIsend class
	07.12.19 - remove iostream
	26.12.21 - Correct m_pbo dimension from 2 to 3. PR #27 by Dimitre

*/
#pragma once
#ifndef __ofxNDIsender__
#define __ofxNDIsender__

#include "ofMain.h"

#if defined(TARGET_WIN32)
#include <windows.h>
#include <intrin.h> // for _movsd
#elif defined(TARGET_OSX)
#if not defined(__aarch64__)
#include <x86intrin.h> // for _movsd
#endif
#elif defined(TARGET_LINUX)
//// TODO - what? - Check
#endif

#include <stdio.h>
#include <string>
#include "ofxNDIsend.h" // basic sender functions
#include "ofxNDIutils.h" // buffer copy utilities

class ofxNDIsender {

public:

	ofxNDIsender();
	~ofxNDIsender();

	// Create an RGBA sender
	// - sendername | name for the sender
	// - width      | sender image width
	// - height     | sender image height
	bool CreateSender(const char *sendername, unsigned int width, unsigned int height);

	// Update sender dimensions
	// - width  | sender image width
	// - height | sender image height
	bool UpdateSender(unsigned int width, unsigned int height);

	// Close sender and release resources
	void ReleaseSender();

	// Return whether the sender has been created
	bool SenderCreated();

	// Return current sender width
	unsigned int GetWidth();

	// Return current sender height
	unsigned int GetHeight();

	// Return the sender name
	std::string GetSenderName();

	// Return the sender NDI name
	std::string GetNDIname();

	// Send ofFbo
	// - fbo     | Openframeworks fbo to send
	// - bInvert | flip the image - default false
	bool SendImage(ofFbo fbo, bool bInvert = false);

	// Send ofTexture
	// - tex     | Openframeworks texture to send
	// - bInvert | flip the image - default false
	bool SendImage(ofTexture tex, bool bInvert = false);
	
	// Send ofImage
	// - image   | Openframeworks image to send
	// - bInvert | flip the image - default false
	// - image is converted to RGBA if not already
	bool SendImage(ofImage img, bool bSwapRB = false, bool bInvert = false);

	// Send ofPixels
	// - pix     | Openframeworks pixel buffer to send
	// - bInvert | flip the image - default false
	// - buffer is converted to RGBA if not already
	bool SendImage(ofPixels pix, bool bSwapRB = false, bool bInvert = false);

	// Send RGBA image pixels
	// - image   | pixel data
	// - width   | image width
	// - height  | image height
	// - bSwapRB | swap red and blue components - default false
	// - bInvert | flip the image - default false
	bool SendImage(const unsigned char *image, unsigned int width, unsigned int height,
		bool bSwapRB = false, bool bInvert = false);

	// Set output format
	void SetFormat(NDIlib_FourCC_video_type_e format);

	// Get output format
	NDIlib_FourCC_video_type_e GetFormat();

	// Set frame rate whole number
	// - framerate - frames per second
	// Initialized 60fps
	void SetFrameRate(int framerate);

	// Set frame rate decimal number
	// - framerate - frames per second
	// Initialized 60fps
	void SetFrameRate(double framerate);

	// Set frame rate numerator and denominator
	// - framerate_N | numerator
	// - framerate_D | denominator
	// Initialized 60fps
	void SetFrameRate(int framerate_N, int framerate_D);

	// Return current fps
	double GetFps();

	// Get current frame rate numerator and denominator
	// - framerate_N | numerator
	// - framerate_D | denominator
	void GetFrameRate(int &framerate_N, int &framerate_D);

	// Get current frame rate as a value
	double GetFrameRate();

	// Set aspect ratio
	// - horizontal | horizontal proportion
	// - vertical   | vertical proportion
	// Initialized 1:1 to use the image source aspect ratio
	void SetAspectRatio(int horizontal = 16, int vertical = 9);

	// Get current aspect ratio
	// - aspect | ratio of horizontal and vertical proportions
	void GetAspectRatio(float &aspect);

	// Set progressive mode
	// Refer to NDI documentation
	// Initialized true
	void SetProgressive(bool bProgressive = true);

	// Get whether progressive
	bool GetProgressive();

	// Set clocked 
	// Refer to NDI documentation
	// (do not clock the video for async sending)
	// Initialized true
	void SetClockVideo(bool bClocked = true);

	// Get whether clocked
	bool GetClockVideo();

	// Set asynchronous sending mode
	// (disables clocked video)
	// Initialized false
	void SetAsync(bool bActive = true);

	// Get whether async sending mode
	bool GetAsync();

	// Set asynchronous readback of pixels from FBO or texture
	void SetReadback(bool bReadback = true);

	// Get current readback mode
	bool GetReadback();

	// Set to send Audio
	// Initialized false
	void SetAudio(bool bAudio = true);

	// Set audio sample rate
	// - sampleRate | rate in hz
	// Initialized 48000 (48khz)
	void SetAudioSampleRate(int sampleRate = 48000); // 48000 = 48kHz

	// Set number of audio channels
	// - nChannels | 1 for mono, 2 for stereo
	// Initialized mono
	void SetAudioChannels(int nChannels = 1);

	// Set number of audio samples
	// There can be up to 1602 samples at 29.97 fps
	// Initialized 1602
	void SetAudioSamples(int nSamples = 1602);

	// Set audio timecode
	// - timecode | the timecode of this frame in 100ns intervals or synthesised
	// Initialized synthesised
	void SetAudioTimecode(int64_t timecode = NDIlib_send_timecode_synthesize);

	// Set audio data
	// - data | data to send (float)
	void SetAudioData(float *data = NULL); // Audio data

	// Set to send metadata
	// Initialized false
	void SetMetadata(bool bMetadata = true);

	// Set metadata
	// - datastring | XML message format string NULL terminated
	void SetMetadataString(std::string datastring);

	// Get the current NDI SDK version
	std::string GetNDIversion();

private:

	ofxNDIsend NDIsender; // Basic sender functions
	std::string m_SenderName; // current sender name

	ofPixels ndiBuffer[2]; // Two pixel buffers for async sending
	int m_idx; // Index used for async buffer swapping

	bool m_bReadback; // Asynchronous readback of pixels from FBO using two PBOs
	GLuint m_pbo[3]; // PBOs used for asynchronous read-back from fbo
	int PboIndex; // Index used for asynchronous read-back from fbo
	int NextPboIndex;
	ofFbo ndiFbo; // Utility Fbo

	// Read pixels from fbo to pixel buffer
	bool ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, ofPixels &buffer);

	// Read pixels from texture to pixel buffer
	bool ReadPixels(ofTexture tex, unsigned int width, unsigned int height, ofPixels &buffer);

	// Asynchronous texture pixel data readback
	bool ReadTexturePixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data);

	// Initialize pixel buffers for sending
	void AllocatePixelBuffers(unsigned int width, unsigned int height);

	//
	// YUV format conversion functions
	//

	ofShader rgba2yuv;  // RGBA to YUV shader

	// Read YUV pixels from RGBA fbo to pixel buffer
	bool ReadYUVpixels(ofFbo &fbo, unsigned int halfwidth, unsigned int height, ofPixels &buffer);

	// Read YUV pixels from RGBA texture to pixel buffer
	bool ReadYUVpixels(ofTexture &tex, unsigned int halfwidth, unsigned int height, ofPixels &buffer);


};

#endif
