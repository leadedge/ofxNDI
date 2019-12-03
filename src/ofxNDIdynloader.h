#pragma once ////

#ifndef OFXNDIDYNLOADER_H
#define OFXNDIDYNLOADER_H

//// #if defined(TARGET_WIN32)
#if defined(_WIN32)
#include <windows.h>
#include <shlwapi.h>  // for path functions
#include <Shellapi.h> // for shellexecute
#pragma comment(lib, "shlwapi.lib")  // for path functions
#pragma comment(lib, "Shell32.lib")  // for shellexecute
#elif defined(TARGET_LINUX) || defined(TARGET_OSX)
#include <dlfcn.h> // dynamic library loading in Linux
#endif

#include <string>
#include <vector>
#include <iostream> //// for cout
#include "Processing.NDI.Lib.h" // NDI SDK

typedef NDIlib_v4* (*NDIlib_v4_load_)(void);


class ofxNDIdynloader
{
	//// 
	/*
	const NDIlib_v4* p_NDILib;
    bool             m_bWasLoaded;
    const std::string FindRuntime();
	*/

public:
    ofxNDIdynloader();
	////
	~ofxNDIdynloader();

    /** load library dynamically */
    const NDIlib_v4* Load();

private :

	////
	HMODULE m_hNDILib;
	const NDIlib_v4* p_NDILib;
	bool             m_bWasLoaded;
	const std::string FindRuntime();


};

#endif // OFXNDIDYNLOADER_H
