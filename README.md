﻿## ofxNDI
An Openframeworks addon to allow sending and receiving images over a network using the NewTek Network Device Protocol.

## testing branch

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
