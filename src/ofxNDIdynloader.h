#ifndef ofxNDIdynloader_H
#define ofxNDIdynloader_H

#include "ofxNDIplatforms.h"

#if defined(TARGET_WIN32)
#include <windows.h>
#include <shlwapi.h>  // for path functions and HMODULE definition
#include <Shellapi.h> // for shellexecute
#pragma comment(lib, "shlwapi.lib")  // for path functions
#pragma comment(lib, "Shell32.lib")  // for shellexecute
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
#include <dlfcn.h> // dynamic library loading in Linux
#endif

#include <string>
#include <vector>
#include "Processing.NDI.Lib.h" // NDI SDK

typedef NDIlib_v4* (*NDIlib_v4_load_)(void);

class ofxNDIdynloader
{
	
public:
    ofxNDIdynloader();
	~ofxNDIdynloader();

    const std::string FindRuntime();

    // load library dynamically
    const NDIlib_v4* Load();
    bool IsLoaded();

private :

#if defined(TARGET_WIN32)
	HMODULE m_hNDILib;
#endif

    bool m_bIsLoaded;
	const NDIlib_v4* p_NDILib;

};

#endif // ofxNDIdynloader_H
