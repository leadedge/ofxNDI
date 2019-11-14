## ofxNDI
An Openframeworks addon to allow sending and receiving images over a network using the NewTek Network Device Protocol.

## Updates
31.03.18 - Updated for NDI SDK Vers 3. Search source for "Vers 3"\
(Note changes to function argument variable types to match with Version 3.)\
06.08.18 - Updated for NDI SDK Vers 3.5 Visual Studio 2017 and Openframeworks 10.\
10.03.19 - Updated for NDI SDK Vers 3.8\
10.11.19 - Updated for NDI SDK Vers 4.0

With update to NDI 4.0, shaders have been removed. All buffers need to be RGBA or BGRA and, for a sender, conversion to other formats are handled by the NDI API. Default receiving format is BGRA, and conversion to RGBA is made within the ofxNDIreceiver class. Experimental audio functions are included but not tested.

ofxNDIsender and ofxNDIreceiver depend on Openframeworks. There are options to send and receive using ofFbo, ofTexture, ofPixels as well as an unsigned char pixel buffer. If receiving to ofFbo, ofTexture of ofPixels, it is not necessary to manage sender size change from the application. Nor is it necessary to handle receiver creation. Simply use the receive functions alone. For best efficiency, the sender class includes pbo pixel buffer readback, activated by SetReadback(). Refer to the header files for details. Examples have been updated to include all the options available. For a simple, practical example, refer to the webcam sender. The examples assume Openframeworks 10 and are not compatible with previous versions.

ofxNDIsend and ofxNDIreceive classes can be used independently for applications other than Openframeworks. Sender size update and receiving buffer size change have to be manged from the application.

## Manual Setup

For Windows

1. Add files from "ofxNDI" to your Visual Studio project.
2. Copy .dll's from ofxNDI/libs to the application "bin" folder
3. Go to the [NDI web page](https://www.ndi.tv/) and download the NDI SDK. Install the NewTek SDK and copy files as follows.
	- Copy the files in  "../NDI 4 SDK/Include" to "ofxNDI/include"
	- Copy the files in "../NDI 4 SDK/Lib" to "ofxNDI/libs/NDI/Lib"

In your Visual Studio project properties :

- Add "ofxNDI/src" to additional "C/C++/General/Additional Include Directories"
- Add "ofxNDI/include" to additional "C/C++/General/Additional Include Directories"\
For a 32bit project
- Add "ofxNDI/libs/NDI/Lib/x86" to "Linker > General > Additional Library Directories"
- Add "Processing.NDI.Lib.x86.lib" to "Linker > Input > Additional Dependencies"\
For a 64bit project
- Add "ofxNDI/libs/NDI/Lib/x64" to "Linker > General > Additional Library Directories"
- Add "Processing.NDI.Lib.x64.lib" to "Linker > Input > Additional Dependencies"
- Add "#include ofxNDI.h" to your source header file

NOTE: for existing projects, you will need to change "ofxNDI/libs/NDI/Libs" to ofxNDI/libs/NDI/Lib". This is in keeping with the NDI folder naming.

## Example sender
Copy the images from "ofxNDI/bin/data" to the application "bin/data" folder.

## Example receiver
Press 's' for a listing NDI senders. Press '0' to 'x' to select a sender. 

Refer to the example code for options available.

## Credits
ofxNDI with help from [Harvey Buchan](https://github.com/Harvey3141).

## Copyrights
ofxNDI - Copyright (C) 2016-2019 Lynn Jarvis [http://spout.zeal.co/](http://spout.zeal.co/)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------
NDI SDK - Copyright NewTek Inc. [https://www.ndi.tv/](https://www.ndi.tv/).

You can include the NDI dlls as part of your own application, but you need to ensure that your application complies with the NDI SDK license. If you have not already installed the NDI SDK, you should download from the [NDI web page](https://www.ndi.tv/) and install it to review the license agreements for it's use. The SDK is used by you in accordance with the license that you accept before installation. In addition, a license document is available for review from the root of the SDK folder. Read the conditions carefully. Your own EULA terms must cover the specific requirements of the NDI SDK EULA, the terms of the LICENSE section and the terms outlined in “3rd party rights” towards the end of the manual.

