/*
	NDI sender

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2017 Lynn Jarvis.

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

*/
#pragma once
#ifndef __ofxNDIsender__
#define __ofxNDIsender__

#include <windows.h>
#include <stdio.h>
#include <string>
#include <intrin.h> // for _movsd
#include <emmintrin.h> // for SSE2
#include "Processing.NDI.Lib.h" // NDI SDK
#include "ofxNDIutils.h" // buffer copy utilities

// Version 2 NDI
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <atomic>

class ofxNDIsender {

public:

	ofxNDIsender();
    ~ofxNDIsender();

	bool CreateSender(const char *sendername, unsigned int width, unsigned int height);
	bool CreateSender(const char *sendername, unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat);
	bool UpdateSender(unsigned int width, unsigned int height);
	void ReleaseSender();

	bool SendImage(unsigned char *image, unsigned int width, unsigned int height,
		           bool bSwapRB = false, bool bInvert=false);
	
	void SetFrameRate(DWORD framerate_N = 60000, DWORD framerate_D = 1000);
	void GetFrameRate(DWORD &framerate_N, DWORD &framerate_D);

	void SetAspectRatio(DWORD horizontal = 16, DWORD vertical = 9);
	void GetAspectRatio(float &aspect);

	void SetProgressive(bool bProgressive = true);
	bool GetProgressive();

	void SetClockVideo(bool bClocked = true);
	bool GetClockVideo();

	void SetAsync(bool bActive = true);
	bool GetAsync();

	// Audio
	void SetAudio(bool bAudio = true);
	void SetAudioSampleRate(DWORD sampleRate = 48000); // 48000 = 48kHz
	void SetAudioChannels(DWORD nChannels = 1); // 2 for stereo
	void SetAudioSamples(DWORD nSamples = 1602); // There can be up to 1602 samples at 29.97 fps
	void SetAudioTimecode(LONGLONG timecode = NDIlib_send_timecode_synthesize); // The timecode of this frame in 100ns intervals or synthesised
	void SetAudioData(FLOAT *data = NULL); // Audio data

	// Metadata
	void SetMetadata(bool bMetadata = true);
	void SetMetadataString(std::string datastring);

	// Utility
	std::string GetNDIversion();

private :

	NDIlib_send_create_t NDI_send_create_desc;
	NDIlib_send_instance_t pNDI_send;
	NDIlib_video_frame_t video_frame;
	BYTE* p_frame;
	DWORD m_frame_rate_N; // Frame rate numerator
	DWORD m_frame_rate_D; // Frame rate denominator
	DWORD m_horizontal_aspect; // Aspect horizontal ratio
	DWORD m_vertical_aspect; // Aspect vertical ratio
	float m_picture_aspect_ratio; // Aspect ratio
	bool m_bProgressive; // Progressive output flag
	bool m_bClockVideo; // Clock video flag
	bool bAsync; // NDI asynchronous sender
	bool bNDIinitialized; // NDI initialized

	// Audio
	bool bNDIaudio;
	NDIlib_audio_frame_t audio_frame;
	DWORD m_AudioSampleRate;
	DWORD m_AudioChannels;
	DWORD m_AudioSamples;
	LONGLONG m_AudioTimecode;
	FLOAT *m_AudioData;

	// Metadata
	bool m_bMetadata;
	NDIlib_metadata_frame_t metadata_frame; // The frame that will be sent
	std::string m_metadataString; // XML message format string NULL terminated - application provided


};


#endif
