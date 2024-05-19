# ofxNDI
An Openframeworks addon to allow sending and receiving images over a network using the NewTek Network Device Protocol.

## Openframeworks

ofxNDIsender and ofxNDIreceiver depend on Openframeworks.

There are options to send and receive using ofFbo, ofTexture, ofPixels as well as an unsigned char pixel buffer. If receiving to ofFbo, ofTexture of ofPixels, it is not necessary to manage sender size change from the application. Nor is it necessary to handle receiver creation. Simply use the receive functions alone. For best efficiency, the sender class includes pbo pixel buffer readback, activated by SetReadback(). Refer to the header files for details. Examples have been updated to include all the options available. For a simple, practical example, refer to the webcam sender. The examples assume Openframeworks 10 and are not compatible with previous versions.

### Setup

### Project Generator

The OF Project Generator will create your project with correct paths. Make sure "ofxNDI" is selected in the addons section and all headers, and DLLs will be imported in the Visual Studio project.

The Project Generator copies the "rgba2yuv" shaders folder into the application. However if the Project Generator is not used, this folder must be copied as below.

	bin
     data
        rgba2yuv

Note that for previous applications, the location of the "rgba2yuv" folder should be changed from within  "bin/data/shaders" to directly within "bin/data".

When the project generation is complete, copy main.cpp, ofApp.cpp and ofApp.h from the required example "src" folder to the project "src" folder. Also copy any files in the example "data" folder to the project. Refer to the example code for options available.

### Manual Setup

1. Add files from "ofxNDI" to your Visual Studio project. A new project filter such as "ofxNDI" recommended.
2. Copy .dll's from "ofxNDI/libs/NDI/bin/vs" to the application "bin" folder
3. In your Visual Studio project properties
- Add "../../../addons/ofxNDI/src" to additional "C/C++/General/Additional Include Directories"
- Add "../../../addons/ofxNDI/libs/NDI/include" to "C/C++/General/Additional Include Directories"
- Add "#include ofxNDI.h" to your source header file

### NDI library files

For Windows, the NDI library files are distributed with this addon in "libs/NDI/bin/vs". They are copied to the application "bin" folder by the Openframeworks Project Generator or by manually as outlined above. The library file version matches that of the NDI header files in "libs/NDI/include" and they have been tested. However, you can use the most recent library files. To do so, remove the NDI dll files from the application executable folder. When you open the example application again, you will be prompted to download the runtime installer. You can also download the installer directly from "http://ndi.link/NDIRedistV6". After installation, the example programs will load the installed file version.

For Mac OSX, Linux and other platforms, the NDI library files are not distributed with this addon and should be installed. After installation, the examples will find and use the installed library file.

## Sub-classes
ofxNDIsend and ofxNDIreceive classes can be used independently of Openframeworks for pixel buffer send and receive.

ofxNDIReceive manages receiver creation and sender name and size change. The receiving buffer size has to be manged from the application. Examples for Windows including Visual Studio project files are contained in the "example-windows" folder.

The Visual Studio solutions "WinSenderNDI.sln" and "WinReceiverNDI.sln" can be opened and built using the addon folder structure.\
After build, copy "Processing.NDI.Lib.x64.dll" from "ofxNDI/libs/NDI/export/vs/x64" to the x64\Release or x64\debug folder.\
Pre-built binaries are included in "example-binaries".

### Setup

To move the project to another folder :
1. Move the entire contents of either the examples-windows\Sender or examples-windows\Receiver folder to the destination.
2. Create a new folder "ofxNDI" within the Sender or Receiver folder.
3. Copy all the files from ofxND\src to the new folder except for :\
    ofxNDIreceiver.cpp, ofxNDIreceiver.h, ofxNDIsender.cpp, ofxNDIsender.h
4. Copy the "ofxNDI\lib" folder and contents to the new folder.
5. In your Visual Studio project properties :\
    [C/C++/General/Additional Include Directories]\
    Change "..\..\libs\NDI\Include" and "..\..\src" to "ofxNDI\libs\NDI\Include" and and "ofxNDI\src".
7. Remove all the files from the project "ofxNDI" filter.
8. Replace them with all the files in the new "ofxNDI" folder.

## Credits
ofxNDI with help from [Harvey Buchan](https://github.com/Harvey3141).

## Copyrights
ofxNDI - Copyright (C) 2016-2024 Lynn Jarvis [http://spout.zeal.co/](http://spout.zeal.co/)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------
NDI SDK - Copyright NewTek Inc. [https://www.ndi.tv/](https://www.ndi.tv/).

You can include the NDI dlls as part of your application as long as your EULA terms cover the specific requirements of the NDI SDK EULA, the terms of the LICENSE section of the SDK documentation and the terms outlined in “3rd party rights” towards the end of the manual. Go to the [NDI web page](https://www.ndi.tv/) and download the NDI SDK. After installation, a license document is available for review from the root of the SDK folder.

If the dlls are not included with the application, the user should install the NDI runtime. Re-start may be required after installation. Download from [http://ndi.link/NDIRedistV6](http://ndi.link/NDIRedistV6).
