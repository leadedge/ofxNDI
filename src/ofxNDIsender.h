/*
	NDI sender

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016 Lynn Jarvis.

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

*/
#pragma once
#ifndef __ofxNDIsender__
#define __ofxNDIsender__

#include <windows.h>
#include <stdio.h>
#include <intrin.h> // for _movsd
#include <emmintrin.h> // for SSE2
#include "Processing.NDI.Lib.h" // NDI SDK
#include "ofxNDIutils.h" // buffer copy utilities

class ofxNDIsender {

public:

	ofxNDIsender();
    ~ofxNDIsender();

	bool CreateSender(const char *sendername, unsigned int width, unsigned int height);
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

	void SetAsync(bool bActive = true);
	bool GetAsync();


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
	BOOL m_bProgressive; // Progressive output flag
	bool bAsync; // NDI asynchronous sender
	bool bNDIinitialized; // NDI initialized

};


#endif
