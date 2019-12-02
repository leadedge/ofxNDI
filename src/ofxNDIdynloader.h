#ifndef OFXNDIDYNLOADER_H
#define OFXNDIDYNLOADER_H

#include <string>
#include <vector>

#include "Processing.NDI.Lib.h" // NDI SDK

typedef NDIlib_v4* (*NDIlib_v4_load_)(void);


class ofxNDIdynloader
{
	const NDIlib_v4* p_NDILib;
    
    const std::string FindRuntime();

public:
    ofxNDIdynloader();

    /** load library dynamically */
    const NDIlib_v4* Load();
};

#endif // OFXNDIDYNLOADER_H
