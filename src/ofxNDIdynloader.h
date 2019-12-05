#ifndef OFXNDIDYNLOADER_H
#define OFXNDIDYNLOADER_H

#include <string>
#include <vector>

#include <boost/dll/import.hpp>         // for dll::import
#include <boost/dll/shared_library.hpp> // for dll::shared_library
#include <boost/function.hpp>

namespace dll = boost::dll;

#include "Processing.NDI.Lib.h" // NDI SDK

typedef NDIlib_v4* (NDIlib_v4_load_)(void);


class ofxNDIdynloader
{
    const NDIlib_v4* p_NDILib;

    dll::shared_library dynlib;
    const std::string FindRuntime();

public:
    ofxNDIdynloader();
    ~ofxNDIdynloader();

    /** load library dynamically */
    const NDIlib_v4* Load();
    const NDIlib_v4* Ref() { return p_NDILib; }
};

#endif // OFXNDIDYNLOADER_H
