/*
	OpenFrameworks NDI webcam sender example

	using the NDI SDK to send the frames via network

	https://ndi.video/

	Copyright (C) 2016-2026 Lynn Jarvis.

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
*/
#pragma once

#include "ofMain.h"
#include "ofxNDI.h" // NDI classes

class ofApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);

		ofVideoGrabber vidGrabber; // Webcam
		std::vector <ofVideoDevice> camdevices; // Webcams available
		ofxNDIsender camsender; // NDI sender object
		std::string camsendername; // Sender name
		int camindex = 0; // Selected webcam in the device list
		bool bSendCam = true; // Clear to send the webcam texture
		unsigned char* campixels = nullptr; // rgba pixel buffer

};
