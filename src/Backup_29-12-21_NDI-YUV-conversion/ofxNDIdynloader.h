#ifndef ofxNDIdynloader_H
#define ofxNDIdynloader_H

#include <cstddef> // to avoid NULL definition problem
#include "ofxNDIplatforms.h"
#include "Processing.NDI.Lib.h" // NDI SDK

#if defined(TARGET_WIN32)
#include <windows.h>
#include <shlwapi.h>  // for path functions and HMODULE definition
#include <Shellapi.h> // for shellexecute
#pragma comment(lib, "shlwapi.lib")  // for path functions
#pragma comment(lib, "Shell32.lib")  // for shellexecute
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
#include <dlfcn.h> // dynamic library loading in Linux
// typedef used for Linux
typedef NDIlib_v4* (*NDIlib_v4_load_)(void);
#endif

#include <string>
#include <vector>
#include <iostream> // for cout

class ofxNDIdynloader
{
	
public:

    ofxNDIdynloader();
    ~ofxNDIdynloader();

    // load library dynamically
    const NDIlib_v4* Load();
	
	// LJ DEBUG
	// An undocumented NDI lib function
	const bool(*bgrx_to_uyvy_cond)(
		/* src */	const BYTE* __restrict p_src_bgrx, const int src_bgrx_stride,
		/* src */	const BYTE* __restrict p_src_bgrx_old, const int src_bgrx_old_stride,
		/* dst */	      BYTE*	__restrict p_dst_uyvy, const int dst_uyvy_stride,
		/* N   */	const int xres, const int yres);


private :

#if defined(TARGET_WIN32)
	HMODULE m_hNDILib;
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
	const std::string FindRuntime();
#endif
	const NDIlib_v4* p_NDILib;

};

#endif // ofxNDIdynloader_H
