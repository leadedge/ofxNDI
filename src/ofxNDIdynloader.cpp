#include "ofxNDIdynloader.h"
//// #include "ofMain.h"

/*
// Move to header
#if defined(TARGET_WIN32)
    #include <windows.h>
    #include <shlwapi.h>  // for path functions
    #include <Shellapi.h> // for shellexecute
    #pragma comment(lib, "shlwapi.lib")  // for path functions
    #pragma comment(lib, "Shell32.lib")  // for shellexecute
#elif defined(TARGET_LINUX) || defined(TARGET_OSX)
    #include <dlfcn.h> // dynamic library loading in Linux
#endif
*/

ofxNDIdynloader::ofxNDIdynloader()
{
    m_bWasLoaded = false;
	m_hNDILib = NULL; ////
}

////
ofxNDIdynloader::~ofxNDIdynloader()
{
	if (p_NDILib) p_NDILib->destroy();
#if defined(WIN32)
	if (m_hNDILib) FreeLibrary(m_hNDILib);
#endif
}


const std::string ofxNDIdynloader::FindRuntime() {
    std::string rt;

    //// const char* p_NDI_runtime_folder = getenv(NDILIB_REDIST_FOLDER);
	char* p_NDI_runtime_folder = nullptr;
	size_t nchars;
	_dupenv_s((char **)&p_NDI_runtime_folder, &nchars, NDILIB_REDIST_FOLDER);
    ////
	// ofLogNotice() << "NDI runtime location " << p_NDI_runtime_folder;
	std::cout << "NDI runtime location " << p_NDI_runtime_folder << std::endl;
    if (p_NDI_runtime_folder)
    {
        rt = p_NDI_runtime_folder;
        rt += NDILIB_LIBRARY_NAME;
    }
    else rt = NDILIB_LIBRARY_NAME;

	////
	free(p_NDI_runtime_folder); // free the buffer allocated by _dupenv_s

    return rt;
}

#if defined(TARGET_OSX) || defined(TARGET_LINUX)
// Dynamic loading of the NDI dlls to avoid needing to use the NDI SDK lib files
const NDIlib_v4* ofxNDIdynloader::Load()
{
    // protect against loading the NDI dll again
    if (m_bWasLoaded) {
        return p_NDILib;
    }

    std::string ndi_path = FindRuntime();

    // attempt to load the library and get a handle to it
    void *m_hNDILib = dlopen(ndi_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
    if (!m_hNDILib) {
        ////
		// ofLogWarning() << "Couldn't load dynamic library at: " << ndi_path;
		std::cout << "Couldn't load dynamic library at: " << ndi_path << std::endl;
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
			////
			m_hNDILib = NULL;
        }
        ////
		// ofLogError() << "Please re-install the NewTek NDI Runtimes from " << NDILIB_REDIST_URL << " to use this application";
		std::cout << "Please re-install the NewTek NDI Runtimes from " << NDILIB_REDIST_URL << " to use this application" << std::endl;
        return nullptr;
    }

    p_NDILib = lib_load(); // this loads the library and returns a pointer to the dynamic binding
    m_bWasLoaded = true;

    return p_NDILib;
}
#elif defined(TARGET_WIN32)
const NDIlib_v4* ofxNDIdynloader::Load()
{
	////
	// protect against loading the NDI dll again
	if (m_bWasLoaded) {
		return p_NDILib;
	}

	std::string ndi_path; ////

    // First look in the executable folder for the dlls
    // in case they are distributed with the application.
    char path[MAX_PATH];
    DWORD dwSize = GetModuleFileNameA(NULL, path, sizeof(path));
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
            char *p_ndi_runtime_v4 = NULL;
            size_t nchars;
            _dupenv_s((char **)&p_ndi_runtime_v4, &nchars, NDILIB_REDIST_FOLDER);
            if (!p_ndi_runtime_v4) {
                // The NDI run-time is not yet installed. Let the user know and take them to the download URL.
                MessageBoxA(NULL, "The NewTek NDI run-time is not yet installed\nPlease install it to use this application", "Warning.", MB_OK);
                ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
                return false;
            }
            // Get the full path of the installed dll
            ndi_path = p_ndi_runtime_v4;
            ndi_path += "\\" NDILIB_LIBRARY_NAME;
            free(p_ndi_runtime_v4); // free the buffer allocated by _dupenv_s
        }
        // Now we have the DLL path
        // printf("Path [%s]\n", ndi_path.c_str());
    }

    // Try to load the library
	m_hNDILib = LoadLibraryA(ndi_path.c_str()); ////
    if (!m_hNDILib) {
        MessageBoxA(NULL, "NDI library failed to load\nPlease re-install the NewTek NDI Runtimes\nfrom " NDILIB_REDIST_URL " to use this application", "Warning", MB_OK);
        ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
		return NULL; //// false;
    }

    // The main NDI entry point for dynamic loading if we got the library
    const NDIlib_v4* (*NDIlib_v4_load)(void) = NULL;
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
		return NULL; // false;
    }

    // Get all of the DLL entry points
    p_NDILib = NDIlib_v4_load();
    if (!p_NDILib) {
        MessageBoxA(NULL, "Failed to load Version 4 NDI library\nPlease use the correct dll files\nor re-install the NewTek NDI Runtimes to use this application", "Warning", MB_OK);
        ShellExecuteA(NULL, "open", NDILIB_REDIST_URL, 0, 0, SW_SHOWNORMAL);
		return NULL; /// false;
    }

    // Check cpu compatibility
    if (!p_NDILib->is_supported_CPU()) {
        MessageBoxA(NULL, "CPU does not support NDI NDILib requires SSE4.1 NDIreceiver", "Warning", MB_OK);
        if (m_hNDILib)
            FreeLibrary(m_hNDILib);
		m_hNDILib = NULL;
		return NULL; //// false;
    }
    else {
        if (!p_NDILib->initialize()) {
            MessageBoxA(NULL, "Cannot run NDI - NDILib initialization failed", "Warning", MB_OK);
            if (m_hNDILib)
                FreeLibrary(m_hNDILib);
			m_hNDILib = NULL;
			return NULL; //// false;
        }
        //// m_bNDIinitialized = true;
		m_bWasLoaded = true;
    }

    /// return true;
	return p_NDILib; ////
}
#endif

