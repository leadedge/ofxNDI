/*

	ofxNDIdownloader
	
	Dynamic loading of the NDI dlls to avoid needing to use the NDI SDK lib files

	http://NDI.NewTek.com

	Copyright (C) 2019-2024

	Luis Rodil-Fernandez
	https://github.com/IDArnhem/ofxNDI
	https://github.com/dropmeaword

	Lynn Jarvis
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

	03-12-19	- Create files
	04-12-19	- Added licencing
	06-12.19	- Revise for cross-platform without OF dependence
	07-12-19	- Use Openframeworks platform definitions in ofxNDIplatforms.h
				- Repeat library load guard if not nullptr
				- FindRuntime() function used for Linux/OSX
				- "using namespace std" not used due to previous advice
	03.12.20	- Change NULL to nullptr for all pointers
				- #include <cstddef> in header to avoid NULL definition problem
	15.12.20	- dlclose(m_hNDILib) in destructor if not WIN32
	27.04.23	- Change unable to find "Version 4" to "Version of"
	27.04.23	- Change headers to NDI version 5.6.0.0
	05.12.23	- Remove path functions and shlwapi.h
				- Use getenv instead of _dupenv_s if not _MSC_VER compiler
				- Revise Load messagebox warnings
	14.12.23	- Conditional include of io.h for Windows target

*/

#include "ofxNDIdynloader.h"

ofxNDIdynloader::ofxNDIdynloader()
{
	p_NDILib = nullptr;
#if defined(TARGET_WIN32)
	m_hNDILib = NULL;
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
	m_hNDILib = nullptr;
#endif
}

ofxNDIdynloader::~ofxNDIdynloader()
{
	if (p_NDILib) p_NDILib->destroy();
#if defined(TARGET_WIN32)
	if (m_hNDILib)
		FreeLibrary(m_hNDILib);
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
	if (m_hNDILib) {
		dlclose(m_hNDILib);
#endif
}


#if defined(TARGET_WIN32)
const NDIlib_v4* ofxNDIdynloader::Load()
{
	// Guard against reloading
	if (p_NDILib)
		return p_NDILib;

	// First look in the executable folder for the dlls
	// in case they are distributed with the application.
	std::string ndi_path;
	std::string str_path;
	char path[MAX_PATH]{};
	DWORD dwSize = GetModuleFileNameA(NULL, path, sizeof(path));
	if (dwSize == 0) {
		MessageBoxA(NULL, "ofxNDIdynloader::Load - could not get executable path", "Warning", MB_OK);
		return p_NDILib;
	}

	// Replace the executable name with NDI library name
	str_path = path;
	size_t pos = str_path.rfind("\\");
	str_path = str_path.substr(0, pos + 1);
	str_path += NDILIB_LIBRARY_NAME;
	// Does the file exist in the executable folder ?
	if (_access(str_path.c_str(), 0) != -1) {
		// Use the exe path
		ndi_path = str_path;
	}
	else {
		// The dll does not exist in exe folder
		// Check whether the NDI run-time is installed
		char* p_ndi_runtime_v4 = nullptr;
#if defined(_MSC_VER)
		_dupenv_s((char**)&p_ndi_runtime_v4, NULL, NDILIB_REDIST_FOLDER);
#else
		p_ndi_runtime_v4 = getenv("NDILIB_REDIST_FOLDER");
#endif
		if (!p_ndi_runtime_v4) {
			// The NDI run-time is not installed.
			// Let the user know and take them to the download URL.
			str_path = "The NDI run-time is not installed\n";
			str_path += "Install from : ";
			str_path += NDILIB_REDIST_URL;
			str_path += "\nDo you want to install it now?";
			if (MessageBoxA(NULL, str_path.c_str(), "Warning", MB_YESNO) == IDYES) {
				ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
			}
			return nullptr;
		}

		// Get the full path of the installed dll
		ndi_path = p_ndi_runtime_v4;
		ndi_path += "\\";
		ndi_path += NDILIB_LIBRARY_NAME;
		free(p_ndi_runtime_v4); // free the buffer allocated by _dupenv_s
	}

	// Now we have the DLL path, try to load the library
	m_hNDILib = LoadLibraryA(ndi_path.c_str());
	if (!m_hNDILib) {
		MessageBoxA(NULL, "Could not load NDI library", "Warning", MB_OK);
		return nullptr;
	}

	// The main NDI entry point for dynamic loading if we got the library
	const NDIlib_v4* (*NDIlib_v4_load)(void) = nullptr;
	*((FARPROC*)&NDIlib_v4_load) = GetProcAddress(m_hNDILib, "NDIlib_v4_load");
	if (!NDIlib_v4_load) {	
		MessageBoxA(NULL, "Could not find NDI library address", "Warning", MB_OK);
		if (m_hNDILib) FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		return nullptr;
	}

	// Get all of the DLL entry points
	p_NDILib = NDIlib_v4_load();
	if (!p_NDILib) {
		MessageBoxA(NULL, "Could not get NDI library functions", "Warning", MB_OK);
		if (m_hNDILib) FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		return nullptr;
	}

	// Check cpu compatibility
	if (!p_NDILib->is_supported_CPU()) {
		MessageBoxA(NULL, "CPU does not support NDI\nNDILib requires SSE4.1", "Warning", MB_OK);
		if (m_hNDILib) FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		return nullptr;
	}
	else {
		if (!p_NDILib->initialize()) {
			MessageBoxA(NULL, "Could not run NDI - NDILib initialization failed", "Warning", MB_OK);
			if (m_hNDILib) FreeLibrary(m_hNDILib);
			m_hNDILib = NULL;
			return nullptr;
		}
	}

	return p_NDILib;

}
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
// OSX and LINUX
const NDIlib_v4* ofxNDIdynloader::Load()
{
 
	// Guard against reloading
	if (p_NDILib)
		return p_NDILib;

    std::string ndi_path = FindRuntime();
    OUTS << "NDI runtime location " << ndi_path << std::endl;

    // attempt to load the library and get a handle to it
    void *m_hNDILib = dlopen(ndi_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
    if (!m_hNDILib) {
        ERRS << "Couldn't load dynamic library at: " << ndi_path << std::endl;
        return nullptr;
    }

    // binding dynamic library
    // see this for reference: https://raw.githubusercontent.com/Palakis/obs-ndi/master/src/obs-ndi.cpp
    NDIlib_v4_load_ lib_load = reinterpret_cast<NDIlib_v4_load_>(dlsym(m_hNDILib, "NDIlib_v4_load"));

    // if lib is loaded but couldn't bind, user probably needs to reinstall
    if (!lib_load)
    {
        // unload the library
        if (m_hNDILib) {
            dlclose(m_hNDILib);
			m_hNDILib = NULL;
        }
        WARNS << "Please re-install the NewTek NDI Runtimes from " << NDILIB_REDIST_URL << " to use this application" << std::endl;
        return nullptr;
    }

    p_NDILib = lib_load(); // this loads the library and returns a pointer to the dynamic binding

	return p_NDILib;
}

const std::string ofxNDIdynloader::FindRuntime() {
	std::string rt;

	const char* p_NDI_runtime_folder = getenv(NDILIB_REDIST_FOLDER);
	if (p_NDI_runtime_folder)
	{
		rt = p_NDI_runtime_folder;
		rt += NDILIB_LIBRARY_NAME;
	}
	else rt = NDILIB_LIBRARY_NAME;

	return rt;
}

#else
const NDIlib_v4* ofxNDIdynloader::Load()
{
    return p_NDILib;
}
#endif


