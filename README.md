## ofxNDI
An Openframeworks addon to allow sending and receiving images over a network using the NewTek Network Device Protocol.

## Updates
31.03.18 - Updated for NDI SDK Vers 3
Search source for "Vers 3"
Note changes to function argument variable types to match with Version 3.

## Setup

For Windows

1. Include the "ofxNDI" addon in your Visual Studio project.
2. Copy .dll's from ofxNDI/libs to the application "bin" folder
3. [Register with Newtek](http://pages.newtek.com/NDI-Developers.html) for the NDI SDK. Install the NewTek SDK and copy files as follows.
	- Copy the files in  "../NewTek NDI SDK/Include" to "ofxNDI/include"
	- Copy the files in "../NewTek NDI SDK/Libs" to "ofxNDI/libs/NDI/Libs"

In your Visual Studio project properties :

- Add "ofxNDI/src" to additional "C/C++ > General > Additional Include Directories"
- Add "ofxNDI/include" to additional "C/C++ > General > Additional Include Directories"
- Add "ofxNDI/libs/NDI/Libs/x86" to "Linker > General > Additional Library Directories"
- Add "Processing.NDI.Lib.x86.lib" to "Linker > Input > Additional Dependencies"
- Add "#include ofxNDI.h" to your source header file

## Example sender
Copy the image from "ofxNDI/example-sender/bin/data" to the application "bin/data" folder.
To set NDI asynchronous sending instead of clocked at 60fps, change ndiSender->SetAsync to true.

## Example receiver
Press 's' for a listing NDI senders. Press '0' to 'x' to select a sender. 
RH click to activate a sender selection dialog.
(The dialog class and resources are Windows only and can be omitted if necessary).

## Credits
ofxNDI with help from [Harvey Buchan](https://github.com/Harvey3141).

## Copyrights
<<<<<<< HEAD
ofxNDI - Copyright (C) 2016-2018 Lynn Jarvis [http://spout.zeal.co/](http://spout.zeal.co/)
=======
ofxNDI - Copyright (C) 2016-2017 Lynn Jarvis [http://spout.zeal.co/](http://spout.zeal.co/)
>>>>>>> origin/master

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser  General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details. 
You should have received a copy of the GNU Lesser General Public License along with this program.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses).

----------------------
NDI SDK - Copyright NewTek Inc. [http://NDI.NewTek.com](http://NDI.NewTek.com).

A license agreement is included with the Newtek SDK when you receive it after registration with NewTek.
The SDK is used by you in accordance with the license you accepted by clicking “I accept” during installation. This license is available for review from the root of the SDK folder.
Read the conditions carefully. You can include the NDI dlls as part of your own application, but the Newtek SDK and specfic SDK's which may be contained within it may not be re-distributed.
Your own EULA must cover the specific requirements of the NDI SDK EULA.

