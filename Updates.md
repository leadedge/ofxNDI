## Updates
--------------------------
01.01.26 - Updated for YUV to RGBA shader (NDI to Spout)\
ofxNDI version 2.001.000\
NDI SDK Vers 6.2.1.0\
ofxNDIreceive.cpp - default prefer UYVY or BGRA data - NDIlib_recv_color_format_UYVY_BGRA\
ofxNDIreceiver.h - Add variables for UYVY > RGBA conversion shaders
ofxNDIreceiver.cpp - Openframeworks shader for UYVY > RGBA conversion\
  GetPixelData - BT601 if width is less than 720, otherwise BT709 for UYVY\
  GetPixelData - test for "shaders" folder\
  Default format - NDIlib_recv_color_format_UYVY_BGRA\
  Add GetVideoType()\
  Increase pbo number from 2 to 3
  Receive to ofPixels - use revised YUV422_to_RGBA for UYVY data  
ofxNDIsender.h - Add variables for RGBA > UYVY RGBA conversion shaders \
ofxNDIsender.cpp - ReadYUVpixels\
  Use Openframeworks shader for RGBA to UYVY conversion\
  BT601 if width is less than 720, otherwise BT709 for UYVY\
  Restore additional test for "shaders" folder\
ofxNDIutils.cpp - Revise YUV422_to_RGBA with lookup tables\
  Gain 7.9 > 5.5 msec at 1920x1080
--------------------------
21.07.25 - Updated for NDI SDK Vers 6.2.0.3\
ofxNDdynloader\
18.03.25 - FindRuntime add #ifdef for Linux runtime - ofxNDI Issue #61\
ofxNDIreceive\
09.05.25 - FindSenders - Remove check for a name change at the same index
due to problems with command line selecting the wrong sender\
ofxNDIsender\
17.03.25 - ReadPixels - remove glGetTeximage method for RGB.
 (RGB pixels are converted to RGBA in SendImage)\
20.03.25 - ReadTexturePixels, CreateSender, UpdateSender, ReleaseSender\
ofxNDIutils\
12-04-25 - Add rgb2rgba\
19.01.25 - Updated for NDI SDK Vers 6.1.1.0\
Dated changes in individual source files.
08.01.25 - Backup Master to Backup branch.\
Update Master with the Testing branch with current revisions.\
Update to NDI version 6.0.1.1.\
Details of changes are documented in each source file.\
10.12.22 - Updated both Master and Testing branches.\
ofxNDIsender update\
rgbg2Yuv shaders located in a "bin\data\rgbg2Yuv" folder instead of\
"bin\data\shaders\rgbg2Yuv" to avoid conflicts with over-write by Project Generator\
SetFormat - test existence of required rgba2yuv shader in "data/rgba2yuv" or "data/shaders/rgba2yuv" for existing code\
UpdateSender - test for sender creation.\
04.07.22 - Updated Master from testing branch.\
	  Details of changes are documented in each source file.\
18.12.20 - Updated master from testing branch NDI SDK Vers 4.5 dynamic load.\
31.03.18 - Updated for NDI SDK Vers 3. Search source for "Vers 3"\
(Note changes to function argument variable types to match with Version 3.)\
06.08.18 - Updated for NDI SDK Vers 3.5 Visual Studio 2017 and Openframeworks 10.\
10.03.19 - Updated for NDI SDK Vers 3.8\
10.11.19 - Updated for NDI SDK Vers 4.0\
15.11.19 - Change to dynamic load of NDI dlls\
16.11.19 - Reconfigure folders and include make files for the project generator\
(as per [pull request](https://github.com/leadedge/ofxNDI/pull/11) by prisonerjohn).\
04.05.20 - Updated for NDI SDK Vers 4.5\
20.08.21 - Merged testing branch - Updated for NDI SDK Vers 5\
30.03.24 - Updated for NDI SDK Vers 5.6.1\
10.04.24 - Updated for NDI SDK Vers 6.0.0.0\
13.05.24 - Add Windows examples using ofxNDIsend/ofxNDIrecieve independent of Openframeworks.
Add "ofxNDIreceive::OpenReceiver()" to simplify application receiver code.
16.05.24 - Add Windows "FindWinRuntime" function with registry read as well as getenv\
Add ReadPathFromRegistry\
Change from deprecated NDIlib_v4* to NDIlib_v5* p_NDILib\
17.05.24 - ofxNDIdownloader\
OSX FindRuntime - use a fixed installation path due to missing environment variable\
FindWinRuntime - return to _dupenv_s due to compiler warning, with conditional free of data\
Search for dll in executable path within FindWinRuntime\
Change addon library path from "libs/NDI/export/vs" to "libs/NDI/bin/vs"\
Change headers and dll files to NDI version 6.0.1.0\
Update addon_config.mk - comment out addons to copy\
Add osx ADDON_LDFLAGS as per Dimitre fork. Test with Project Generator\
Update example sender/receiver comments - http://NDI.NewTek.com -> https://ndi.video/\
Update sender/receiver binaries\
Create this file - Updates.md
 

