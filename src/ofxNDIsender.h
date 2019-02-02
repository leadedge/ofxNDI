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

	08.07.18 - Use ofxNDIsend class

*/
#pragma once
#ifndef __ofxNDIsender__
#define __ofxNDIsender__

#include "ofMain.h"

#if defined(_WIN32)
#include <windows.h>
#include <intrin.h> // for _movsd
#endif

#if defined(__APPLE__)
#include <x86intrin.h> // for _movsd
#endif

#include <stdio.h>
#include <string>
#include <emmintrin.h> // for SSE2
#include <iostream> // for cout
#include "Processing.NDI.Lib.h" // NDI SDK
#include "ofxNDIsend.h" // basic sender functions
#include "ofxNDIshaders.h" // Openframeworks shader functions
#include "ofxNDIutils.h" // buffer copy utilities

class ofxNDIsender {

public:

	ofxNDIsender();
	~ofxNDIsender();

	// Create an RGBA sender
	// - sendername | name for the sender
	// - width | sender image width
	// - height | sender image height
	bool CreateSender(const char *sendername, unsigned int width, unsigned int height);

	// Create a sender of specified colour format
	// Formats supported are RGBA, RGBX, BGRA, BGRX and UVYV
	// - sendername | name for the sender
	// - width | sender image width
	// - height | sender image height
	// - colorFormat | pixel format
	bool CreateSender(const char *sendername, unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat);

	// Update sender dimensions
	// - width | sender image width
	// - height | sender image height
	bool UpdateSender(unsigned int width, unsigned int height);

	// Update sender dimensions and colour format
	// - width | sender image width
	// - height | sender image height
	// - colorFormat | pixel format
	bool UpdateSender(unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat);

	// Close sender and release resources
	void ReleaseSender();

	// Return whether the sender has been created
	bool SenderCreated();

	// Return current sender width
	unsigned int GetWidth();

	// Return current sender height
	unsigned int GetHeight();

	// Return the NDI sender name
	std::string GetSenderName();

	// Send ofFbo
	// - fbo | Openframeworks fbo to send
	// - bInvert | flip the image - default false
	bool SendImage(ofFbo fbo, bool bInvert = false);

	// Send ofTexture
	// - tex | Openframeworks texture to send
	// - bInvert | flip the image - default false
	bool SendImage(ofTexture tex, bool bInvert = false);

	// Send ofImage
	// - image | Openframeworks image to send
	// - bInvert | flip the image - default false
	// - image is converted to RGBA if not already
	bool SendImage(ofImage img, bool bInvert = false);

	// Send ofPixels
	// - pix | Openframeworks pixel buffer to send
	// - bInvert | flip the image - default false
	// - buffer is converted to RGBA if not already
	bool SendImage(ofPixels pix, bool bInvert = false);

	// Send RGBA image pixels
	// - image | pixel data
	// - width | image width
	// - height | image height
	// - bSwapRB | swap red and blue components - default false
	// - bInvert | flip the image - default false
	bool SendImage(const unsigned char *image, unsigned int width, unsigned int height,
		bool bSwapRB = false, bool bInvert = false);


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

	// Set aspect ratio
	// - horizontal | horizontal proportion
	// - vertical | vertical proportion
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
	bool m_bReadback; // Asynchronous readback of pixels from FBO using two PBOs
	NDIlib_FourCC_type_e m_ColorFormat; // Color format to send
	ofPixels ndiBuffer[2]; // Two pixel buffers for async sending
	int m_idx; // Index used for async buffer swapping
	GLuint ndiPbo[2]; // PBOs used for asynchronous read-back from fbo
	int PboIndex; // Index used for asynchronous read-back from fbo
	int NextPboIndex;
	std::string m_SenderName; // current sender name

	// RGBA to YUV and RGBA to BGRA conversion
	ofxNDIshaders yuvshaders;
	ofFbo ndiFbo; // Utility Fbo
	ofTexture ndiTexture; // utility texture

	// Convert fbo texture from RGBA to YUV
	void ColorConvert(ofFbo fbo);

	// Convert texture from RGBA to YUV
	void ColorConvert(ofTexture tex);

	// Convert fbo texture RGBA <> BGRA
	void ColorSwap(ofFbo fbo);

	// Convert texture RGBA <> BGRA
	void ColorSwap(ofTexture tex);

	// Read pixels from fbo to buffer
	void ReadPixels(ofFbo fbo, unsigned int width, unsigned int height, unsigned char *data);

	// Read pixels from texture to buffer
	void ReadPixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data);

	// Asynchronous fbo pixel readback
	bool ReadFboPixels(ofFbo fbo, unsigned int width, unsigned int height, unsigned char *data);

	// Asynchronous texture pixel readback
	bool ReadTexturePixels(ofTexture tex, unsigned int width, unsigned int height, unsigned char *data);


};

#endif
