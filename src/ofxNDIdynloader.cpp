#include "ofxNDIdynloader.h"
#include "ofMain.h"

ofxNDIdynloader::ofxNDIdynloader()
{
}

ofxNDIdynloader::~ofxNDIdynloader()
{
    if( dynlib.is_loaded() ) {
        dynlib.unload();
    }
}

const std::string ofxNDIdynloader::FindRuntime() {
    std::string rt;

    const char* p_NDI_runtime_folder = getenv(NDILIB_REDIST_FOLDER);
    ofLogNotice() << "NDI runtime location " << p_NDI_runtime_folder;
    if (p_NDI_runtime_folder)
    {
        rt = p_NDI_runtime_folder;
        rt += NDILIB_LIBRARY_NAME;
    }
    else rt = NDILIB_LIBRARY_NAME;

    return rt;
}

// Dynamic loading of the NDI dlls to avoid needing to use the NDI SDK lib files
const NDIlib_v4* ofxNDIdynloader::Load()
{
    std::string path = FindRuntime();
    //path.append(SharedLibrary::suffix());   // adds ".dll" or ".so"

    dynlib.load(path, dll::load_mode::search_system_folders);
    if( dynlib.has("NDIlib_v4_load") ) {
        ofLogNotice() << "library has NDIlib_v4_load method";
    }

    NDIlib_v4_load_& lib_load = dynlib.get<typeof(NDIlib_v4_load_)>("NDIlib_v4_load");
    p_NDILib = lib_load();
    return p_NDILib;
}
