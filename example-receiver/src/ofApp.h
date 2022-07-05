/*

	OpenFrameworks NDI receiver example

	using the NDI SDK to receive frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2022 Lynn Jarvis.

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

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void keyPressed(int key);

		ofxNDIreceiver ndiReceiver; // NDI receiver
		ofFbo ndiFbo; // Fbo to receive
		ofTexture ndiTexture; // Texture to receive
		ofImage ndiImage; // Image to receive
		ofPixels ndiPixels; // Pixels to receive
		unsigned char *ndiChars; // unsigned char image array to receive
		unsigned int senderWidth = 0; // sender width and height needed to receive char pixels
		unsigned int senderHeight = 0;
		void ShowInfo();

};
