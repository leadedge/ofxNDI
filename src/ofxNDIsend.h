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

	16.10.16 - common buffer copy utilities
	09.02.17 - Changes for NDI SDK Version 2
			 - include changes by Harvey Buchan
			 - include metadata
	17.02.17 - GetNDIversion - NDIlib_version
	31.03.18 - Update to NDI SDK Version 3

	Changes with 3.5 update
	11.06.18 - Change class name to ofxNDIsend
			   Class can be used independently of Openframeworks
			 - Remove Version 2 includes
	         - Header function comments expanded so that they are visible to the user
			 - Add changes for OSX (https://github.com/ThomasLengeling/ofxNDI)
			 - add "m_" prefix to all class variables
	15.11.19 - Change to dynamic load of Newtek NDI dlls

*/
#pragma once
#ifndef __ofxNDIsend__
#define __ofxNDIsend__

#include <stdio.h>
#include <string>

#include "ofxNDIdynloader.h" // NDI library loader
#include "ofxNDIutils.h" // buffer copy utilities

// Definition is in WinBase.h
// define for compilers that don't include this
#ifndef MAX_COMPUTERNAME_LENGTH
#ifndef _MAC
#define MAX_COMPUTERNAME_LENGTH 15
#else
#define MAX_COMPUTERNAME_LENGTH 31
#endif
#endif


class ofxNDIsend {

public:

	ofxNDIsend();
	~ofxNDIsend();

	// Create an RGBA sender
	// - sendername | name for the sender
	// - width | sender image width
	// - height | sender image height
	bool CreateSender(const char *sendername, unsigned int width, unsigned int height);

	// Update sender dimensions
	// - width | sender image width
	// - height | sender image height
	bool UpdateSender(unsigned int width, unsigned int height);

	// Send image pixels
	// - image | pixel data BGRA or RGBA
	// - width | image width
	// - height | image height
	// - bSwapRB | swap red and blue components - default false
	// - bInvert | flip the image - default false
	bool SendImage(const unsigned char *image, unsigned int width, unsigned int height,
		bool bSwapRB, bool bInvert = false);

	// Send image pixels
	// - image | pixel data BGRA or RGBA
	// - width | image width
	// - height | image height
	// - bInvert | flip the image - default false
	bool SendImage(const unsigned char *image, unsigned int width, unsigned int height,	bool bInvert = false);

	// Send image pixels allowing for source buffer pitch
	// - image | pixel data BGRA or RGBA
	// - width | image width
	// - height | image height
	// - pitch | source buffer pitch
	// - bInvert | flip the image - default false
	bool SendImage(const unsigned char *image,
		unsigned int width, unsigned int height, 
		unsigned int sourcePitch, bool bInvert = false);

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

	// Set output format
	void SetFormat(NDIlib_FourCC_video_type_e format);

	// Get output format
	NDIlib_FourCC_video_type_e GetFormat();

	// Set frame rate
	// - framerate - frames per second
	// Initialized 60fps
	void SetFrameRate(int framerate);
	void SetFrameRate(double framerate);

	// Set frame rate
	// - framerate_N | numerator
	// - framerate_D | denominator
	// Initialized 60fps
	void SetFrameRate(int framerate_N, int framerate_D);

	// Get current frame rate
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

	// Set to send Audio
	// Initialized false
	void SetAudio(bool bAudio = true);

	// Get whether audio sending is set
	bool GetAudio();

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
	void SetAudioData(const float *data = NULL); // Audio data

	// Set to send metadata
	// Initialized false
	void SetMetadata(bool bMetadata = true);

	// Set metadata
	// - datastring | XML message format string NULL terminated
	void SetMetadataString(std::string datastring);

	// Get the current NDI SDK version
	std::string GetNDIversion();

private:

	const NDIlib_v4* p_NDILib;
	bool m_bNDIinitialized;

	ofxNDIdynloader libloader;

	NDIlib_send_create_t NDI_send_create_desc;
	NDIlib_send_instance_t pNDI_send;
	NDIlib_video_frame_v2_t video_frame;
	uint8_t* p_frame;

	// Sender dimensions
	unsigned int m_Width, m_Height;
	bool bSenderInitialized;

	// Sender options
	int m_frame_rate_N; // Frame rate numerator
	int m_frame_rate_D; // Frame rate denominator
	int m_horizontal_aspect; // Aspect horizontal ratio
	int m_vertical_aspect; // Aspect vertical ratio
	float m_picture_aspect_ratio; // Aspect ratio
	bool m_bProgressive; // Progressive output flag
	bool m_bClockVideo; // Clock video flag
	bool m_bAsync; // NDI asynchronous sender
	NDIlib_FourCC_video_type_e m_Format; // Output format. Default RGBA. May also be BGRA or YUV.
	void SetVideoStride(NDIlib_FourCC_video_type_e format); // Set line stride for YUV or RGBA

	// Audio
	bool m_bAudio;
	NDIlib_audio_frame_v2_t m_audio_frame;
	int m_AudioSampleRate;
	int m_AudioChannels;
	int m_AudioSamples;
	int64_t m_AudioTimecode;
	float *m_AudioData;

	// Metadata
	bool m_bMetadata;
	NDIlib_metadata_frame_t metadata_frame; // The frame that will be sent
	std::string m_metadataString; // XML message format string NULL terminated - application provided

};


#endif
