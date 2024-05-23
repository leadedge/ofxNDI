/*

	ofxNDIdownloader
	
	Dynamic loading of the NDI dlls to avoid needing to use the NDI SDK lib files

	https://ndi.video

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
	29.12.23	- For OSX
				- fix redundant brace for m_hNDILib in destructor
				- fix double declare of m_hNDILib
	29.03.24	- Change headers and dll files to NDI version 5.6.1.0
	10.04.24	- Change headers and dll files to NDI version 6.0.0.0
	16.05.24	- Add Windows "FindWinRuntime" function with registry read as well as getenv
				- Add ReadPathFromRegistry
				- Change from deprecated NDIlib_v4* to NDIlib_v5* p_NDILib
	17.05.24	- OSX FindRuntime - use a fixed installation path due to missing environment variable
				- FindWinRuntime - return to _dupenv_s due to compiler warning, with conditional free of data
				- Search for dll in executable path within FindWinRuntime
				- Change addon library path from "libs/NDI/export/vs" to "libs/NDI/bin/vs"
				- Change headers and dll files to NDI version 6.0.1.0
	17.05.24	- Return to using GetModuleFileName to get a full path to the dll

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
	if (m_hNDILib)
		dlclose(m_hNDILib);
#endif
}


#if defined(TARGET_WIN32)
const NDIlib_v5* ofxNDIdynloader::Load()
{
	// Guard against reloading
	if (p_NDILib)
		return p_NDILib;

	// Look for the NDI dll
	std::string ndi_path;
	if (!FindWinRuntime(ndi_path)) {
		// Not installed - invite the user to install it
		ndi_path = "The NDI run-time is not installed\n";
		ndi_path += "Download from : ";
		ndi_path += NDILIB_REDIST_URL;
		ndi_path += "\nDo you want to download it now?";
		if (MessageBoxA(NULL, ndi_path.c_str(), "Warning", MB_YESNO) == IDYES) {
			ndi_path = "After download, run the installer,\n";
			ndi_path += "close and restart this program.";
			MessageBoxA(NULL, ndi_path.c_str(), "Information", MB_OK);
			ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
		}
		// Return to try again
		return nullptr;
	}

	// The dll path has now been found either in the
	// executable folder or in the runtime installation folder.
	// Try to load the library from the file path.
	m_hNDILib = LoadLibraryA(ndi_path.c_str());
	if (!m_hNDILib) {
		MessageBoxA(NULL, "Could not load NDI library", "Warning", MB_OK);
		return nullptr;
	}

	// The main NDI entry point for dynamic loading if we got the library
	const NDIlib_v5* (*NDIlib_v5_load)(void) = nullptr;
	*((FARPROC*)&NDIlib_v5_load) = GetProcAddress(m_hNDILib, "NDIlib_v5_load");
	if (!NDIlib_v5_load) {
		MessageBoxA(NULL, "Could not find NDI library address", "Warning", MB_OK);
		if (m_hNDILib) FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		return nullptr;
	}

	// Get all of the DLL entry points
	p_NDILib = NDIlib_v5_load();
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
		// Initialize the library
		if (!p_NDILib->initialize()) {
			MessageBoxA(NULL, "Could not run NDI - NDILib initialization failed", "Warning", MB_OK);
			if (m_hNDILib) FreeLibrary(m_hNDILib);
			m_hNDILib = NULL;
			return nullptr;
		}
	}
	
	std::cout << "\nLoaded NDI library - " << ndi_path.c_str() << std::endl;
	std::cout << p_NDILib->version() << " (https://ndi.video/)" << std::endl;

	return p_NDILib;

}

//
// Library file finder - Windows version
//
// Tests for an environment variable set by the NDI runtime installation.
// Note that if the runtime has been installed, the environment variable
// cannnot be accessed with out a machine re-boot. However, there is also
// a registry key that can be queried to find the file location.
//
bool ofxNDIdynloader::FindWinRuntime(std::string &runtime)
{
	std::string rt;

	// First look in the executable folder for the dll
	// in case it is distributed with the application.
	// A file name without full path can be used if the application 
	// is run directly from the executable folder, but not when run
	// from within Visual Studio, so create a full path here.
	char path[MAX_PATH]{};
	DWORD dwSize = GetModuleFileNameA(NULL, path, sizeof(path));
	if (dwSize > 0) {
		// Replace the executable name with NDI library name
		rt = path;
		rt = rt.substr(0, rt.rfind("\\") + 1);
		rt += NDILIB_LIBRARY_NAME;
		if (_access(rt.c_str(), 0) != -1) {
			runtime = rt;
			return true;
		}
	}

	// Look for an NDI runtime installation with the
	// environment variable NDILIB_REDIST_FOLDER
	// "C:\Program Files\NDI\NDI 6 Runtime\v6"
	char* p_NDI_runtime_folder = nullptr;
#if defined(_MSC_VER)
	_dupenv_s((char**)&p_NDI_runtime_folder, NULL, NDILIB_REDIST_FOLDER);
#else
	p_ndi_runtime = getenv("NDILIB_REDIST_FOLDER");
#endif
	if (p_NDI_runtime_folder) {
		// Full path to the dll
		rt = p_NDI_runtime_folder;
		rt += "\\";
		rt += NDILIB_LIBRARY_NAME;
#if defined(_MSC_VER)
		free(p_NDI_runtime_folder); // free the buffer allocated by _dupenv_s
#endif
		// Does the file exist?
		if (_access(rt.c_str(), 0) != -1) {
			runtime = rt;
			return true;
		}
	}

	// Environment variable Not found.
	// Look for a registry key in case the system has not been re-started yet.
	// HLKM/System/CurrentControlSet/Control/Session Manager/Environment/NDI_RUNTIME_DIR_V6
	if (ReadPathFromRegistry(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", "NDI_RUNTIME_DIR_V6", path)) {
		rt = path;
		rt += "\\";
		rt += NDILIB_LIBRARY_NAME;
		// Does the file exist?
		if (_access(rt.c_str(), 0) != -1) {
			runtime = rt;
			return true;
		}
	}

	// No runtime environment variable
	return false;
}

// Function: ReadPathFromRegistry
// Read subkey character string
bool ofxNDIdynloader::ReadPathFromRegistry(HKEY hKey, const char* subkey, const char* valuename, char* filepath, DWORD dwSize)
{
	if (!subkey || !*subkey || !valuename || !*valuename || !filepath)
		return false;

	HKEY  hRegKey = NULL;
	LONG  regres = 0;
	DWORD dwKey = 0;
	DWORD dwSizePath = dwSize;

	// Does the key exist
	regres = RegOpenKeyExA(hKey, subkey, NULL, KEY_READ, &hRegKey);
	if (regres == ERROR_SUCCESS) {
		// Read the key Filepath value
		regres = RegQueryValueExA(hRegKey, valuename, NULL, &dwKey, (BYTE*)filepath, &dwSizePath);
		RegCloseKey(hRegKey);
		if (regres == ERROR_SUCCESS) {
			return true;
		}
		if (regres == ERROR_MORE_DATA) {
			printf("ReadPathFromRegistry -  buffer size (%d) not large enough (%d)\n", dwSize, dwSizePath);
		}
		else {
			printf("ReadPathFromRegistry - could not read [%s] from registry\n", valuename);
		}
	}

	// Quit if the key does not exist
	return false;

}

#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
// OSX and LINUX
const NDIlib_v5* ofxNDIdynloader::Load()
{
 
	// Guard against reloading
	if (p_NDILib)
		return p_NDILib;

    std::string ndi_path = FindRuntime();
    OUTS << "NDI runtime location " << ndi_path << std::endl;

    // attempt to load the library and get a handle to it
    m_hNDILib = dlopen(ndi_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
    if (!m_hNDILib) {
        ERRS << "Couldn't load dynamic library at: " << ndi_path << std::endl;
        return nullptr;
    }

    // binding dynamic library
    // see this for reference: https://raw.githubusercontent.com/Palakis/obs-ndi/master/src/obs-ndi.cpp
    NDIlib_v5_load_ lib_load = reinterpret_cast<NDIlib_v5_load_>(dlsym(m_hNDILib, "NDIlib_v5_load"));

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

	// Use a fixed path instead of getenv(NDILIB_REDIST_FOLDER)
	// due to missing environment variable after runtime installation.
	// If the runtime folder is not found, return the library file name.
	const char* p_NDI_runtime_folder = "/usr/local/lib/libndi.dylib";
	if (p_NDI_runtime_folder)
		rt = p_NDI_runtime_folder;
	else
		rt = NDILIB_LIBRARY_NAME;

	return rt;
}

#else
const NDIlib_v5* ofxNDIdynloader::Load()
{
    return p_NDILib;
}
#endif


