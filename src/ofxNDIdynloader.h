#ifndef ofxNDIdynloader_H
#define ofxNDIdynloader_H

#if defined(_WIN32)
#include <windows.h>
#include <shlwapi.h>  // for path functions and HMODULE definition
#include <Shellapi.h> // for shellexecute
#pragma comment(lib, "shlwapi.lib")  // for path functions
#pragma comment(lib, "Shell32.lib")  // for shellexecute
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h> // dynamic library loading in Linux
#endif

#include <string>
#include <vector>
#include <iostream> // for cout
#include "Processing.NDI.Lib.h" // NDI SDK

typedef NDIlib_v4* (NDIlib_v4_load_)(void);

class ofxNDIdynloader
{
	
public:
    ofxNDIdynloader();
	~ofxNDIdynloader();

    // load library dynamically
    const NDIlib_v4* Load();

private :

#if defined(_WIN32)
	HMODULE m_hNDILib;
#endif
	const NDIlib_v4* p_NDILib;

};

#endif // ofxNDIdynloader_H
