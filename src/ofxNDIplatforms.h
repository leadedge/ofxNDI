#pragma once

// 28.12.21  Change TARGET_OS_IPHONE_SIMULATOR to TARGET_IPHONE_SIMULATOR
//           PR #29 by dimitre

#ifndef __ofxNDIplatforms_
#define __ofxNDIplatforms_

#if !defined(OF_VERSION_MAJOR) // guard aginst overriding OF constants if we are in a OF app

// error, warning and out streams for non-OF apps all map to `cout`
#define ERRS std::cout
#define WARNS std::cout
#define OUTS std::cout

//
//	Platform definitions
//
// Copied from Openframeworks platform defines in "ofConstants.h"
//
//-------------------------------
//  find the system type --------
//-------------------------------
// 		helpful:
// 		http://www.ogre3d.org/docs/api/html/OgrePlatform_8h-source.html
//

#if defined( __WIN32__ ) || defined( _WIN32 )
#define TARGET_WIN32
#define TARGET_WINDOWS
#elif defined( __APPLE_CC__)
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <TargetConditionals.h>
#if (TARGET_OS_IPHONE || TARGET_OS_IOS || TARGET_OS_SIMULATOR || TARGET_IPHONE_SIMULATOR) && !TARGET_OS_TV && !TARGET_OS_WATCH
#define TARGET_OF_IPHONE
#define TARGET_OF_IOS
#define TARGET_OPENGLES
#include <unistd.h>
#elif TARGET_OS_TV
#define TARGET_OF_IOS
#define TARGET_OF_TVOS
#define TARGET_OPENGLES
#include <unistd.h>
#elif TARGET_OS_WATCH
#define TARGET_OF_IOS
#define TARGET_OF_WATCHOS
#define TARGET_OPENGLES
#include <unistd.h>
#else
#define TARGET_OSX
#endif
#elif defined (__ANDROID__)
#define TARGET_ANDROID
#define TARGET_OPENGLES
#elif defined(__ARMEL__)
#define TARGET_LINUX
#define TARGET_OPENGLES
#define TARGET_LINUX_ARM
#elif defined(__EMSCRIPTEN__)
#define TARGET_EMSCRIPTEN
#define TARGET_OPENGLES
#define TARGET_NO_THREADS
#define TARGET_PROGRAMMABLE_GL
#define TARGET_IMPLEMENTS_URL_LOADER
#else
#define TARGET_LINUX
#endif
//-------------------------------

// Could be extended - see ofConstants.h

#elif defined(OF_VERSION_MAJOR) // we are being compiled from an OF applications

// error, warning and out streams for non-OF apps all map to `cout`
#define ERRS ofLogError()
#define WARNS ofLogWarning()
#define OUTS ofLogNotice()

#endif // !defined(OF_VERSION_MAJOR)

#endif

