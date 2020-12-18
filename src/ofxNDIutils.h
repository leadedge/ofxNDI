/*
	NDI utility functions 

	using the NDI SDK to receive frames from the network

	http://NDI.NewTek.com

	Copyright (C) 2016-2020 Lynn Jarvis.

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
#include <x86intrin.h> // for _movsd
#elif defined(TARGET_WIN32)
#include <windows.h>
#include <intrin.h> // for _movsd
// #else // Linux
#endif

#include <cstring>
#include <climits>

namespace ofxNDIutils {
	void CopyImage(const unsigned char *source, unsigned char *dest, 
				   unsigned int width, unsigned int height, unsigned int stride,
				   bool bSwapRB = false, bool bInvert = false);
// TODO : check it works for OSX
#if defined(TARGET_WIN32) || defined(TARGET_OSX)
	void memcpy_sse2(void* dst, const void* src, size_t Size);
	void memcpy_movsd(void* dst, const void* src, size_t Size);
	void rgba_bgra_sse2(const void *source, void *dest, unsigned int width, unsigned int height, bool bInvert = false);
#endif
	void rgba_bgra(const void *rgba_source, void *bgra_dest, unsigned int width, unsigned int height, bool bInvert = false);
	void FlipBuffer(const unsigned char *src, unsigned char *dst, unsigned int width, unsigned int height);
	void YUV422_to_RGBA(const unsigned char * source, unsigned char * dest, unsigned int width, unsigned int height, unsigned int stride);
}


#endif

