## ofxNDI
An Openframeworks addon to allow sending and receiving images over a network using the NewTek Network Device Protocol.

## Updates
31.03.18 - Updated for NDI SDK Vers 3 - Search source for "Vers 3".\
Note changes to function argument variable types to match with Version 3.\
06.08.18 - Updated for NDI SDK Vers 3.5 Visual Studio 2017 and Openframeworks 10.\
02.02.19 - Corrections and audio receive testing.\
06.08.18 - Updated NDI dlls and readme files for NDI Vers 3.8.

Significant changes have been made for this version. The original classes have been renamed :

    ofxNDIsender > ofxNDIsend
    ofxNDIreceiver > ofxNDIreceive

The same functions remain, but with additions to support the new classes. For existing applications, you can still use them directly by simply renaming the class names, and including the header files specifically. The significance is that they are not dependent on Openframeworks, so can be used in other applications. 

The new class files depend on Openframeworks and are much easier to use. There are now options to send and receive using ofFbo, ofTexture, ofPixels as well as an unsigned char pixel buffer. 

If receiving to Openframeworks fbo, texture of pixels, it is no longer necessary to manage sender size change from the application. Also it is no longer necessary to handle receiver creation. Simply use the receive functions alone.

The sender includes pbo pixel buffer readback within the class itself, activated by SetReadback.

NDIlib_FourCC_type_UYVY sending format is supported by way of a shader for increased efficiency. Default format is NDIlib_FourCC_type_BGRA.

New functions for the receiver include :

    bool ReceiverCreated();
    bool ReceiveImage(ofFbo &fbo);
    bool ReceiveImage(ofTexture &texture);
    bool ReceiveImage(ofImage &image);
    bool ReceiveImage(ofPixels &pixels);
    NDIlib_frame_type_e GetFrameType();
    bool GetSenderName(char *sendername, int maxsize, int index = -1);
    std::string GetSenderName(int index = -1);
    unsigned int GetSenderWidth();
    unsigned int GetSenderHeight();
	NDIlib_frame_type_e GetFrameType();
    double GetFps();
	
and for the sender :

    bool UpdateSender(unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat);
	bool SenderCreated();
	unsigned int GetWidth();
	unsigned int GetHeight();
    std::string GetSenderName();
	bool SendImage(ofFbo fbo, bool bInvert = false);
	bool SendImage(ofTexture tex, bool bInvert = false);
	bool SendImage(ofImage img, bool bInvert = false);
	bool SendImage(ofPixels pix, bool bInvert = false);
	void SetFrameRate(int framerate);
	void SetFrameRate(double framerate);
	double GetFps();
	void SetReadback(bool bReadback = true);
	bool GetReadback();
	
Refer to the header files for details.

Examples have been updated to include all options available. For a simple, practical example, refer to the webcam sender. The examples assume Openframeworks 10 and are not compatible with previous versions.

## Setup

For Windows

1. Add files from "ofxNDI" to your Visual Studio project.
2. Copy .dll's from ofxNDI/libs to the application "bin" folder
3. [Register with Newtek](http://pages.newtek.com/NDI-Developers.html) for the NDI SDK. Install the NewTek SDK and copy files as follows.
	- Copy the files in  "../NewTek NDI 3.8 SDK/Include" to "ofxNDI/include"
	- Copy the files in "../NewTek NDI 3.8 SDK/Lib" to "ofxNDI/libs/NDI/Libs"

In your Visual Studio project properties :

- Add "ofxNDI/src" to additional "C/C++/General/Additional Include Directories"
- Add "ofxNDI/include" to additional "C/C++/General/Additional Include Directories"
For a 32bit project
- Add "ofxNDI/libs/NDI/Libs/x86" to "Linker > General > Additional Library Directories"
- Add "Processing.NDI.Lib.x86.lib" to "Linker > Input > Additional Dependencies"
For a 64bit project
- Add "ofxNDI/libs/NDI/Libs/x64" to "Linker > General > Additional Library Directories"
- Add "Processing.NDI.Lib.x64.lib" to "Linker > Input > Additional Dependencies"
- Add "#include ofxNDI.h" to your source header file


## Example sender
Copy the images from "ofxNDI/bin/data" to the application "bin/data" folder.

## Example receiver
Press 's' for a listing NDI senders. Press '0' to 'x' to select a sender. 

Refer to the example code for options available.

## Credits
ofxNDI with help from [Harvey Buchan](https://github.com/Harvey3141).

## Copyrights
ofxNDI - Copyright (C) 2016-2019 Lynn Jarvis [http://spout.zeal.co/](http://spout.zeal.co/)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. 
You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------
NDI SDK - Copyright NewTek Inc. [http://NDI.NewTek.com](http://NDI.NewTek.com).

A license agreement is included with the Newtek SDK when you receive it after registration with NewTek.
The SDK is used by you in accordance with the license you accepted by clicking “I accept” during installation. This license is available for review from the root of the SDK folder.
Read the conditions carefully. You can include the NDI dlls as part of your own application, but the Newtek SDK and specfic SDK's which may be contained within it may not be re-distributed.
Your own EULA must cover the specific requirements of the NDI SDK EULA.

