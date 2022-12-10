## ofxNDI
An Openframeworks addon to allow sending and receiving images over a network using the NewTek Network Device Protocol.

## Updates

The Project Generator copies the "rgba2yuv" shaders folder into the application.
However if the Project Generator is not used, this folder must be copied as below.

	bin
     data
        rgba2yuv

Note that for previous applications, the location of the "rgba2yuv" folder should be changed
from within a containing "shaders" folder to directly in "bin/data". Example binaries are included for testing.

--------------------------
10.12.22 - Updated both Master and Testing branches.\
ofxNDIsender update\
rgbg2Yuv shaders located in a "bin\data\rgbg2Yuv" folder
instead of "bin\data\shaders\rgbg2Yuv" to avoid conflicts with over-write by Project Generator\
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
20.08.21 - Merged testing branch - Updated for NDI SDK Vers 5

With update to NDI 4.0, shaders have been removed. All buffers need to be RGBA or BGRA and, for a sender, conversion to other formats are handled by the NDI API. Default receiving format is BGRA, and conversion to RGBA is made within the ofxNDIreceiver class. Experimental audio functions are included but not tested.

ofxNDIsender and ofxNDIreceiver depend on Openframeworks. There are options to send and receive using ofFbo, ofTexture, ofPixels as well as an unsigned char pixel buffer. If receiving to ofFbo, ofTexture of ofPixels, it is not necessary to manage sender size change from the application. Nor is it necessary to handle receiver creation. Simply use the receive functions alone. For best efficiency, the sender class includes pbo pixel buffer readback, activated by SetReadback(). Refer to the header files for details. Examples have been updated to include all the options available. For a simple, practical example, refer to the webcam sender. The examples assume Openframeworks 10 and are not compatible with previous versions.

ofxNDIsend and ofxNDIreceive classes can be used independently for applications other than Openframeworks. Sender size update and receiving buffer size change have to be manged from the application.

## Setup

For Windows

### Project Generator

The OF Project Generator will create your project with correct paths. Make sure "ofxNDI" is selected in the addons section and all headers, and DLLs will be imported in the Visual Studio project.

### Manual Setup

1. Add files from "ofxNDI" to your Visual Studio project.
2. Copy .dll's from "ofxNDI/libs/NDI/export/vs/Win32" and "ofxNDI/libs/NDI/export/vs/x64" to the application "bin" folder
3. In your Visual Studio project properties :
- Add "ofxNDI/src" to additional "C/C++/General/Additional Include Directories"
- Add "ofxNDI/libs/NDI/include" to  "C/C++/General/Additional Include Directories"
- Add "#include ofxNDI.h" to your source header file

## Example sender
Copy the images from "ofxNDI/example-sender/bin/data" to the application "bin/data" folder.

## Example receiver
Press 's' for a listing NDI senders. Press '0' to 'x' to select a sender. 

Refer to the example code for options available.

## Credits
ofxNDI with help from [Harvey Buchan](https://github.com/Harvey3141).

## Copyrights
ofxNDI - Copyright (C) 2016-2022 Lynn Jarvis [http://spout.zeal.co/](http://spout.zeal.co/)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------
NDI SDK - Copyright NewTek Inc. [https://www.ndi.tv/](https://www.ndi.tv/).

You can include the NDI dlls as part of your application as long as your EULA terms cover the specific requirements of the NDI SDK EULA, the terms of the LICENSE section of the SDK documentation and the terms outlined in “3rd party rights” towards the end of the manual. Go to the [NDI web page](https://www.ndi.tv/) and download the NDI SDK. After installation, a license document is available for review from the root of the SDK folder.

If the dlls are not included with the application, the user should install the NDI runtime. Re-start may be required after installation. Download from [http://new.tk/NDIRedistV4](http://new.tk/NDIRedistV4).
