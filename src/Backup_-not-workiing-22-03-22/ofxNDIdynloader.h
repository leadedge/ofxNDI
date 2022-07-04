#ifndef ofxNDIdynloader_H
#define ofxNDIdynloader_H

#include <cstddef> // to avoid NULL definition problem
#include "ofxNDIplatforms.h"

// Avoids warning C26451 with VS2022 code review
// https://docs.microsoft.com/en-us/visualstudio/code-quality/c26451
#pragma warning( push )
#pragma warning( disable: 26812 )
#include "Processing.NDI.Lib.h" // NDI SDK
#pragma warning( pop )

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
