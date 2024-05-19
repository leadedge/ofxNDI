/*
	NDI utility functions 

	using the NDI SDK to receive frames from the network

	http://NDI.NewTek.com

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

	16.10.16 - Create file
	11.06.18 - Add changes for OSX (https://github.com/ThomasLengeling/ofxNDI)
	06.12.19 - Remove SSE functions for Linux
	07.12.19 - remove includes emmintrin.h, xmmintrin.h, iostream, cstdint

*/
#pragma once
#ifndef __ofxNDI_
#define __ofxNDI_

#include "ofxNDIplatforms.h" // Openframeworks platform definitions
#include <stdint.h> // ints of known sizes, standard library
#include <stdlib.h>
#include <string.h>
#include <iostream> // for cout

// TODO : test includes for OSX
#if defined(TARGET_OSX)
#if defined(__aarch64__)
#include "sse2neon.h"
#else
#include <x86intrin.h> // for _movsd
#endif
#elif defined(TARGET_WIN32)
#include <windows.h>
#include <intrin.h> // for _movsd
#pragma comment (lib, "winmm.lib") // for timeBeginPeriod
// #else // Linux
#endif

#include <cstring>
#include <climits>

//
// C++11 timer is only available for MS Visual Studio 2015 and above.
//
// Note that _MSC_VER may not correspond correctly if an earlier platform toolset
// is selected for a later compiler e.g. Visual Studio 2010 platform toolset for
// a Visual studio 2017 compiler. "#include <chrono>" will then fail.
// If this is a problem, remove _MSC_VER_ and manually enable/disable the USE_CHRONO define.
//
#if _MSC_VER >= 1900
#define USE_CHRONO
#endif

#ifdef USE_CHRONO
#include <chrono> // c++11 timer
#include <thread>
#endif

namespace ofxNDIutils {

	// ofxNDI version number
	std::string GetVersion();

	// Copy rgba source image to dest.
	// Images must be the same size with no line padding.
	// Option flip image vertically (invert).
	void CopyImage(const unsigned char *source, unsigned char *dest,
		unsigned int width, unsigned int height,
		bool bInvert = false);

	// Copy rgba source image to dest.
	// Source line pitch (unused).
	// Option convert bgra<>rgba.
	// Option flip image vertically (invert).
	void CopyImage(const unsigned char *source, unsigned char *dest,
		unsigned int width, unsigned int height, unsigned int stride,
		bool bSwapRB = false, bool bInvert = false);

	// Copy rgba image buffers line by line.
	// Allow for both source and destination line pitch.
	// Option flip image vertically (invert).
	void CopyImage(const void* source, void* dest,
		unsigned int width, unsigned int height,
		unsigned int sourcePitch, unsigned int destPitch,
		bool bInvert = false);

// TODO : check it works for OSX
#if defined(TARGET_WIN32) || defined(TARGET_OSX)
	void memcpy_sse2(void* dst, const void* src, size_t Size);
	void memcpy_movsd(void* dst, const void* src, size_t Size);
	void rgba_bgra_sse2(const void *source, void *dest, unsigned int width, unsigned int height, bool bInvert = false);
#endif

	void rgba_bgra(const void *rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false);
	void FlipBuffer(const unsigned char *src, unsigned char *dst, unsigned int width, unsigned int height);
	void YUV422_to_RGBA(const unsigned char * source, unsigned char * dest, unsigned int width, unsigned int height, unsigned int stride);

#ifdef USE_CHRONO

	// Start timing period
	void StartTiming();
	// Stop timing and return microseconds elapsed.
	// Code console output can be enabled for quick timing tests.
	double EndTiming();
	void HoldFps(int fps);
#if defined(TARGET_WIN32)
	// Windows minimum time period
	void StartTimePeriod();
	void EndTimePeriod();
#endif

#endif

}


#endif

