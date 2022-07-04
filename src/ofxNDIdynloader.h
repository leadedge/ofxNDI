#ifndef ofxNDIdynloader_H
#define ofxNDIdynloader_H

//
// Option : disable warning C26812 (unscoped enums) for Visual Studio for using NDI enums.
//
// Use of C++11 scoped (class) enums are not compatible with early compilers (< VS2012 and others).
// However, for Visual Studio this warning is designated "Prefer" and "C" standard unscoped enums
// are acceptable and compatible. The warning can be enabled or disabled here.
//
// #pragma warning(disable:26812)
//

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

private :

#if defined(TARGET_WIN32)
	HMODULE m_hNDILib;
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
	const std::string FindRuntime();
#endif
	const NDIlib_v4* p_NDILib;

};

#endif // ofxNDIdynloader_H
