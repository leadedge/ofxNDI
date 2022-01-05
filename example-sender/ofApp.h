/*
	OpenFrameworks NDI sender example

	using the NDI SDK to send the frames via network

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
#include "ofxNDI.h" // ofxNDI classes

class ofApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);

		ofxNDIsender ndiSender;    // NDI sender
		char senderName[256];      // Sender name
		unsigned int senderWidth;  // Width of the sender output
		unsigned int senderHeight; // Height of the sender output
		ofFbo m_fbo;               // Fbo used for graphics and sending
		ofImage textureImage;      // Texture image for the 3D cube graphics
		float rotX, rotY;          // Cube rotation increment
		ofImage ndiImage;          // Test image for sending
		void DrawGraphics();       // Rotating cube draw
		
		bool bReadback = true;
		bool bAsync = true;
		double framerate = 60.0;

};
