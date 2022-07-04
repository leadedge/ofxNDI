/*

	Dynamic loading of the NDI dlls to avoid needing to use the NDI SDK lib files

	http://NDI.NewTek.com

	Copyright (C) 2019-2020

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

*/

#include "ofxNDIdynloader.h"

ofxNDIdynloader::ofxNDIdynloader()
{
	p_NDILib = nullptr;
#if defined(TARGET_WIN32)
	m_hNDILib = NULL;
#endif
}

ofxNDIdynloader::~ofxNDIdynloader()
{
	if (p_NDILib) p_NDILib->destroy();
#if defined(TARGET_WIN32)
	if (m_hNDILib) FreeLibrary(m_hNDILib);
#endif
}


#if defined(TARGET_WIN32)
const NDIlib_v4* ofxNDIdynloader::Load()
{
	// Guard against reloading
	if(p_NDILib)
		return p_NDILib;

	std::string ndi_path;

	// First look in the executable folder for the dlls
	// in case they are distributed with the application.
	char path[MAX_PATH];
	uint32_t dwSize = (uint32_t)GetModuleFileNameA(NULL, path, sizeof(path));
	if (dwSize > 0) {
		PathRemoveFileSpecA(path); // Remove executable name
		strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, MAX_PATH, NDILIB_LIBRARY_NAME);
		// Does the dll file exist in the executable folder ?
		if (PathFileExistsA(path)) {
			// Use the exe path
			ndi_path = path;
		}
		else {
			// The dll does not exist in exe folder
			// Check whether the NDI run-time is installed
			char *p_ndi_runtime_v4 = nullptr;
			size_t nchars;
			_dupenv_s((char **)&p_ndi_runtime_v4, &nchars, NDILIB_REDIST_FOLDER);
			if (!p_ndi_runtime_v4) {
				// The NDI run-time is not yet installed. Let the user know and take them to the download URL.
				MessageBoxA(NULL, "The NewTek NDI run-time is not yet installed\nPlease install it to use this application", "Warning.", MB_OK);
				ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
				return nullptr;
			}
			// Get the full path of the installed dll
			ndi_path = p_ndi_runtime_v4;
			ndi_path += "\\" NDILIB_LIBRARY_NAME;
			free(p_ndi_runtime_v4); // free the buffer allocated by _dupenv_s
		}
		// Now we have the DLL path
	}

	// Try to load the library
	m_hNDILib = LoadLibraryA(ndi_path.c_str());
	if (!m_hNDILib) {
		MessageBoxA(NULL, "NDI library failed to load\nPlease re-install the NewTek NDI Runtimes\nfrom " NDILIB_REDIST_URL " to use this application", "Warning", MB_OK);
		ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
		return nullptr;
	}

	// The main NDI entry point for dynamic loading if we got the library
	const NDIlib_v4* (*NDIlib_v4_load)(void) = nullptr;
	*((FARPROC*)&NDIlib_v4_load) = GetProcAddress(m_hNDILib, "NDIlib_v4_load");

	// If we failed to load the library then we tell people to re-install it
	if (!NDIlib_v4_load)
	{	// Unload the DLL if we loaded it
		if (m_hNDILib)
			FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		// The NDI run-time is not installed correctly. Let the user know and take them to the download URL.
		MessageBoxA(NULL, "Failed to find Version 4 NDI library\nPlease use the correct dll files\nor re-install the NewTek NDI Runtimes to use this application", "Warning", MB_OK);
		ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
		return nullptr;
	}

	// Get all of the DLL entry points
	p_NDILib = NDIlib_v4_load();
	if (!p_NDILib) {
		MessageBoxA(NULL, "Failed to load Version 4 NDI library\nPlease use the correct dll files\nor re-install the NewTek NDI Runtimes to use this application", "Warning", MB_OK);
		ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
		return nullptr;
	}

	// Check cpu compatibility
	if (!p_NDILib->is_supported_CPU()) {
		MessageBoxA(NULL, "CPU does not support NDI NDILib requires SSE4.1 NDIreceiver", "Warning", MB_OK);
		if (m_hNDILib)
			FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		return nullptr;
	}
	else {
		if (!p_NDILib->initialize()) {
			MessageBoxA(NULL, "Cannot run NDI - NDILib initialization failed", "Warning", MB_OK);
			if (m_hNDILib)
				FreeLibrary(m_hNDILib);
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


